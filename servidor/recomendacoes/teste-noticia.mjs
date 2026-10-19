// Teste das rotas /v1/noticia e /v1/noticia/img (src/noticia.js) em Node puro,
// sem wrangler e sem rede: o fetch e um duble que serve as MESMAS paginas que
// o extrator em C le em tests/leitura.c (tests/fixtures/noticia-*.html).
//
//   node servidor/recomendacoes/teste-noticia.mjs
import assert from "node:assert/strict";
import { readFileSync } from "node:fs";
import { fileURLToPath } from "node:url";
import { rotaNoticia, rotaNoticiaImg, extrair, gnId, gnAntigo, bingDireto } from "./src/noticia.js";

const ok = (nome, cond) => { assert.ok(cond, nome); console.log("ok", nome); };
const U = (s) => new URL("https://rec.exemplo.dev" + s);
const fixture = (n) => readFileSync(fileURLToPath(new URL("../../tests/fixtures/" + n, import.meta.url)), "utf8");

// O worker roda sem nodejs_compat: `Buffer` nao existe la. O extrator roda
// aqui sem ele, para o teste pegar quem voltar a usa-lo (o Response do Node
// precisa dele, por isso so em volta do extrair).
const bufferGuardado = globalThis.Buffer;
const semBuffer = (f) => { delete globalThis.Buffer; try { return f(); } finally { globalThis.Buffer = bufferGuardado; } };

// --- extrator: as mesmas armadilhas do C
{
  const L = semBuffer(() => extrair(fixture("noticia-wordpress.html"), "https://updateordie.com/2025/07/02/x/"));
  ok("extrair: og:title decodificado", L.titulo.startsWith("Foundation temporada 3: primeiras críticas"));
  ok("extrair: og:image absoluta", L.imagem.startsWith("https://updateordie.com/"));
  ok("extrair: 2..6 paragrafos", L.paragrafos.length >= 2 && L.paragrafos.length <= 6);
  ok("extrair: nenhum paragrafo curto nem estourado",
    L.paragrafos.every((p) => new TextEncoder().encode(p).length >= 60 && new TextEncoder().encode(p).length <= 900));
}
{
  const L = semBuffer(() => extrair(fixture("noticia-arc.html"), "https://www.estadao.com.br/x/"));
  ok("extrair: Arc XP tira o texto do JSON do Fusion", L.paragrafos.length >= 2);
}
{
  const L = semBuffer(() => extrair(fixture("noticia-duplicada.html"), "https://seriesemcena.com.br/x/"));
  ok("extrair: texto duplicado (celular + mesa) sai uma vez", new Set(L.paragrafos).size === L.paragrafos.length);
}

// --- Google News / Bing
ok("gnId: id do link do RSS", gnId("https://news.google.com/rss/articles/CBMiAbc_-1?oc=5") === "CBMiAbc_-1");
ok("gnId: outro host nao e Google News", gnId("https://example.com/rss/articles/abc") === null);
{
  // formato antigo: 08 13 22 <len> <url> em base64 url-safe
  const url = "https://www.exemplo.com.br/noticia/1";
  const bin = String.fromCharCode(0x08, 0x13, 0x22, url.length) + url;
  const id = btoa(bin).replace(/\+/g, "-").replace(/\//g, "_").replace(/=+$/, "");
  ok("gnAntigo: URL dentro do base64", gnAntigo(id) === url);
}
ok("bingDireto: apiclick com url=", bingDireto("https://www.bing.com/news/apiclick.aspx?ref=FexRss&url=https%3a%2f%2fx.com.br%2fa&c=1") === "https://x.com.br/a");

// --- rota: pagina do veiculo servida pelo duble
let pedidos = [];
const HTML = fixture("noticia-wordpress.html");
const rede = (mapa) => async (alvo, op) => {
  pedidos.push({ alvo, op });
  const f = mapa[alvo];
  if (!f) return new Response("nao", { status: 404 });
  return typeof f === "function" ? f(op) : f.clone();
};
const PAG = "https://updateordie.com/2025/07/02/foundation/";
const html200 = new Response(HTML, { status: 200, headers: { "content-type": "text/html; charset=utf-8" } });

let r = await rotaNoticia(U("/v1/noticia?u=" + encodeURIComponent(PAG)), rede({ [PAG]: html200 }));
let j = await r.json();
ok("rota: 200 com CORS e cache de 1 h", r.status === 200 && r.headers.get("access-control-allow-origin") === "*" &&
  /max-age=3600/.test(r.headers.get("cache-control")));
ok("rota: url, titulo e paragrafos", j.url === PAG && j.titulo.startsWith("Foundation") && j.paragrafos.length >= 2);
ok("rota: imagem reescrita para /v1/noticia/img", j.imagem.startsWith("https://rec.exemplo.dev/v1/noticia/img?u=https%3A%2F%2Fupdateordie.com"));
ok("rota: pedido com UA de navegador e redirect manual",
  /Mozilla/.test(pedidos[0].op.headers["user-agent"]) && pedidos[0].op.redirect === "manual");

r = await rotaNoticia(U("/v1/noticia?url=" + encodeURIComponent(PAG)), rede({ [PAG]: html200 }));
ok("rota: ?url= e alias de ?u=", (await r.json()).url === PAG);

// --- limites (SSRF): recusa sem buscar nada
pedidos = [];
for (const ruim of ["", "file:///etc/passwd", "javascript:alert(1)", "ftp://x.com/a", "http://127.0.0.1/",
  "http://10.0.0.5/a", "http://192.168.1.20:8080/", "http://169.254.169.254/latest/meta-data/",
  "http://[::1]/", "http://localhost/", "http://intranet/", "http://user:pw@x.com/", "http://tv.local/"]) {
  r = await rotaNoticia(U("/v1/noticia?u=" + encodeURIComponent(ruim)), rede({}));
  assert.equal(r.status, 400, "link " + ruim);
}
ok("limites: esquema/IP privado/loopback/metadados/IPv6/nome sem ponto recusados sem pedir nada", pedidos.length === 0);

// redirecionamento para IP privado: o salto passa pelo mesmo crivo
pedidos = [];
r = await rotaNoticia(U("/v1/noticia?u=" + encodeURIComponent(PAG)), rede({
  [PAG]: new Response("", { status: 302, headers: { location: "http://10.1.2.3/admin" } }) }));
j = await r.json();
ok("limites: redirect para rede privada nao e seguido", pedidos.length === 1 && j.paragrafos.length === 0);

// pagina gigante: le no maximo 512 KB e ainda extrai
{
  const enorme = HTML + "<p>" + "x".repeat(2 * 1024 * 1024) + "</p>";
  let lidos = 0;
  const corpo = new ReadableStream({
    start(c) { const b = new TextEncoder().encode(enorme);
      for (let i = 0; i < b.length; i += 65536) c.enqueue(b.subarray(i, i + 65536)); c.close(); },
  });
  const contador = new TransformStream({ transform(ch, c) { lidos += ch.byteLength; c.enqueue(ch); } });
  r = await rotaNoticia(U("/v1/noticia?u=" + encodeURIComponent(PAG)), async () =>
    new Response(corpo.pipeThrough(contador), { status: 200, headers: { "content-type": "text/html" } }));
  j = await r.json();
  ok("limites: pagina de 2 MB cortada (le <= 512 KB + um bloco)", lidos <= 512 * 1024 + 65536 && j.paragrafos.length >= 2);
}

// prazo: rede que nunca responde aborta pelo sinal
{
  const t0 = Date.now();
  const pendurada = (alvo, op) => new Promise((_, rej) => op.signal.addEventListener("abort", () => rej(new Error("abort"))));
  const orig = globalThis.setTimeout;
  globalThis.setTimeout = (f) => orig(f, 20);   // 10 s viram 20 ms no teste
  r = await rotaNoticia(U("/v1/noticia?u=" + encodeURIComponent(PAG)), pendurada);
  globalThis.setTimeout = orig;
  j = await r.json();
  ok("limites: prazo estourado responde 200 vazio (a TV cai no QR)", r.status === 200 && j.paragrafos.length === 0 && Date.now() - t0 < 2000);
}

// pagina que bloqueia (403): 200 com a url, sem texto — o que basta ao QR
r = await rotaNoticia(U("/v1/noticia?u=" + encodeURIComponent(PAG)), rede({ [PAG]: new Response("robo", { status: 403 }) }));
j = await r.json();
ok("fallback: 403 do veiculo = url sem texto", r.status === 200 && j.url === PAG && j.paragrafos.length === 0);

// Google News formato novo: pagina com assinatura + POST batchexecute
{
  const LINK = "https://news.google.com/rss/articles/AU_yqLtokenOpaco?oc=5";
  const pg = '<div data-n-a-sg="SIG_abc-1" data-n-a-ts="1727600000"></div>';
  const resp = ')]}\'\n\n[["wrb.fr","Fbv4je","[\\"garturlres\\",\\"' + PAG + '\\",1]",null]]';
  pedidos = [];
  r = await rotaNoticia(U("/v1/noticia?u=" + encodeURIComponent(LINK)), rede({
    [LINK]: new Response(pg, { status: 200 }),
    "https://news.google.com/_/DotsSplashUi/data/batchexecute": (op) => {
      ok("gn: POST leva id, ts e assinatura", op.method === "POST" &&
        decodeURIComponent(op.body).includes("AU_yqLtokenOpaco") && decodeURIComponent(op.body).includes("SIG_abc-1"));
      return new Response(resp, { status: 200 });
    },
    [PAG]: html200,
  }));
  j = await r.json();
  ok("gn: link opaco resolvido ate o veiculo", j.url === PAG && j.paragrafos.length >= 2);
}

// cache: segundo pedido nao sai
{
  const mapa = new Map();
  const cache = { match: async (k) => mapa.get(k.url)?.clone(), put: async (k, v) => { mapa.set(k.url, v); } };
  pedidos = [];
  await rotaNoticia(U("/v1/noticia?u=" + encodeURIComponent(PAG)), rede({ [PAG]: html200 }), cache);
  r = await rotaNoticia(U("/v1/noticia?u=" + encodeURIComponent(PAG)), rede({ [PAG]: html200 }), cache);
  ok("cache: resposta guardada nao repete a busca", pedidos.length === 1 && (await r.json()).url === PAG);
}

// --- /v1/noticia/img
const IMG = "https://updateordie.com/capa.jpg";
r = await rotaNoticiaImg(U("/v1/noticia/img?u=" + encodeURIComponent(IMG)),
  rede({ [IMG]: new Response(new Uint8Array([0xff, 0xd8, 0xff]), { status: 200, headers: { "content-type": "image/jpeg" } }) }));
ok("img: image/* repassada com CORS e cache de 1 dia", r.status === 200 && r.headers.get("content-type") === "image/jpeg" &&
  r.headers.get("access-control-allow-origin") === "*" && /max-age=86400/.test(r.headers.get("cache-control")));
r = await rotaNoticiaImg(U("/v1/noticia/img?u=" + encodeURIComponent(IMG)),
  rede({ [IMG]: new Response("<html>", { status: 200, headers: { "content-type": "text/html" } }) }));
ok("img: o que nao e imagem vira 502", r.status === 502);
r = await rotaNoticiaImg(U("/v1/noticia/img?u=" + encodeURIComponent(IMG)),
  rede({ [IMG]: new Response("<svg/>", { status: 200, headers: { "content-type": "image/svg+xml" } }) }));
ok("img: svg recusado (script dentro)", r.status === 502);
r = await rotaNoticiaImg(U("/v1/noticia/img?u=" + encodeURIComponent(IMG)),
  rede({ [IMG]: new Response("x", { status: 200, headers: { "content-type": "image/png", "content-length": String(4 * 1024 * 1024) } }) }));
ok("img: maior que 3 MB recusada", r.status === 502);
r = await rotaNoticiaImg(U("/v1/noticia/img?u=" + encodeURIComponent("http://192.168.1.1/a.png")), rede({}));
ok("img: IP privado recusado", r.status === 400);

console.log("PASS: noticia (extrator, Google News, limites, cache, imagem)");
