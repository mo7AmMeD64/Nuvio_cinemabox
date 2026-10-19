// Worker da TV Hisense VIDAA: o site estatico (/tv/*) e o proxy que a pagina
// usa (/v1/proxy). Sem D1, sem sessao, sem segredo.
//
// POR QUE E UM WORKER SEPARADO: isto morava em nuvio-recomendacoes, e o
// [assets] ficava no wrangler.toml de la. O wrangler troca os assets inteiros a
// cada deploy, entao qualquer deploy da API feito de uma arvore sem
// build/vidaa-site (todas menos a feat/vidaa) tirava a pagina da TV do ar: caiu
// em 24/09 e de novo de 29/09 a 05/10 (issue #135). Separado, so
// tools/vidaa-publicar.sh mexe aqui.
import { rotaProxy, rotaProxyCors } from "../../recomendacoes/src/proxy.js";

const erro = (msg, status) =>
  new Response(JSON.stringify({ erro: msg }), {
    status,
    headers: { "content-type": "application/json; charset=utf-8", ...rotaProxyCors },
  });

// --- Hospedagem estatica da TV VIDAA -----------------------------------------
//
// build/vidaa-site/ e um site gerado por outro script (tools/vidaa-site.sh),
// nao por este worker: tv/versao.txt com a versao corrente e
// tv/<versao>/{mt,st}/{index.html,index.js,index.wasm,index.data,
// decodificador.js,hls.min.js}, mais tv/icone-*.png. O worker so serve isto
// (wrangler.toml: [assets] + run_worker_first) porque tres coisas
// exigem codigo no meio do caminho: resolver /tv para a versao atual sem o
// cliente saber qual e, ligar COOP/COEP so no mt/ (pthreads exige isolamento
// cross-origin; SharedArrayBuffer nao existe sem isso) e por cache longo nos
// arquivos versionados. O binding falta em ambiente de teste sem `[assets]`
// configurado (ex.: teste-xtream.mjs chamando rotaXtream direto) — por isso
// toda funcao aqui comeca conferindo `env.ASSETS`.
let versaoCache = null, versaoCacheAte = 0;
async function versaoAtual(env) {
  const t = Date.now();
  if (versaoCache && t < versaoCacheAte) return versaoCache;
  const r = await env.ASSETS.fetch(new Request("https://tv.interna/tv/versao.txt"));
  if (!r.ok) return null;
  const v = (await r.text()).trim();
  if (!v) return null;
  versaoCache = v;
  versaoCacheAte = t + 60000;   // 60s: o mesmo isolate nao bate no ASSETS a cada pedido de /tv
  return v;
}

// Extensao -> content-type. SO as que build/vidaa-site produz; o resto sai
// como o Asset Worker ja serviu (ele acerta html/js/png sozinho pela mesma
// tabela de mimes do navegador — o que ele NAO acerta e o que este mapa
// cobre).
const TIPOS_TV = {
  ".wasm": "application/wasm",
  ".data": "application/octet-stream",
};
function tipoTvDe(caminho) {
  const i = caminho.lastIndexOf(".");
  return i < 0 ? null : TIPOS_TV[caminho.slice(i)] || null;
}

async function rotaTv(req, url, env) {
  if (!env.ASSETS) return erro("hospedagem da tv indisponivel", 404);
  const rota = url.pathname;

  // /tv e /tv/ -> a versao atual, sempre mt/ primeiro (a shell la dentro pula
  // sozinha para ../st/ quando falta SharedArrayBuffer — ver tools/tizen.sh).
  if (rota === "/tv" || rota === "/tv/") {
    const v = await versaoAtual(env);
    if (!v) return erro("versao indisponivel", 404);
    return Response.redirect(new URL(`/tv/${v}/mt/`, url).toString(), 302);
  }

  if (/^\/tv\/icone-\d+\.png$/.test(rota)) {
    const r = await env.ASSETS.fetch(req);
    if (!r.ok) return r;
    const h = new Headers(r.headers);
    h.set("cache-control", "public, max-age=31536000, immutable");
    return new Response(r.body, { status: r.status, headers: h });
  }

  // /tv/versao.txt DIRETO (nao so a leitura interna de versaoAtual): quem
  // testar uma TV manda esta URL para conferir qual versao esta no ar sem
  // seguir o redirect inteiro. Achado testando o deploy real (24/09): a
  // regex de baixo exige /mt//st, entao este caminho caia em 404 mesmo
  // com o arquivo publicado e o redirect de /tv/ funcionando (ele le por
  // fetch interno, que nao passa por aqui).
  if (rota === "/tv/versao.txt") {
    const r = await env.ASSETS.fetch(req);
    if (!r.ok) return r;
    const h = new Headers(r.headers);
    h.set("cache-control", "no-cache");
    return new Response(r.body, { status: r.status, headers: h });
  }

  const m = /^\/tv\/([^/]+)\/(mt|st)\/(.*)$/.exec(rota);
  if (!m) return erro("rota da tv desconhecida", 404);
  const modo = m[2], resto = m[3];

  const r = await env.ASSETS.fetch(req);
  if (!r.ok) return r;
  const h = new Headers(r.headers);

  const tipo = tipoTvDe(resto);
  if (tipo) h.set("content-type", tipo);

  // versao.txt (fora de /tv/<v>/) ja e tratado acima; aqui dentro so o
  // index.html do bundle pode mudar sem trocar de caminho (a pessoa fica na
  // MESMA versao enquanto uma TV vieja ainda a usa). Tudo o mais no caminho
  // versionado e imutavel: o nome do arquivo so muda quando o conteudo muda.
  const ehIndex = resto === "" || resto === "index.html";
  h.set("cache-control", ehIndex ? "no-cache" : "public, max-age=31536000, immutable");

  // COOP/COEP SO NO mt/: e o modo com pthreads, que so arranca com
  // SharedArrayBuffer, que so existe com a origem isolada. O st/ compila sem
  // -pthread (fio1.c cooperativo) e nao precisa disto — e exigir credentialless
  // ali quebraria, sem motivo, qualquer sub-recurso que o st/ venha a buscar
  // sem CORP/CORS proprio.
  if (modo === "mt") {
    h.set("cross-origin-opener-policy", "same-origin");
    h.set("cross-origin-embedder-policy", "credentialless");
    h.set("cross-origin-resource-policy", "same-origin");
  }

  return new Response(r.body, { status: r.status, headers: h });
}

export default {
  async fetch(req, env) {
    const url = new URL(req.url);
    const rota = url.pathname;

    if (req.method === "OPTIONS") return new Response(null, { status: 204, headers: rotaProxyCors });
    if (rota === "/") return Response.redirect(new URL("/tv/", url).toString(), 302);
    if (rota === "/tv" || rota.startsWith("/tv/")) return rotaTv(req, url, env);
    if (rota === "/v1/proxy" && req.method === "GET") return rotaProxy(req, url, env);
    return erro("rota desconhecida", 404);
  },
};
