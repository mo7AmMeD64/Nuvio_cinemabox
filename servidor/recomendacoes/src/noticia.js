// NOTICIA COMPLETA PARA A SAMSUNG (modal de noticia da Agenda, 29/09/2026).
//
// Na TV da LG o app baixa a pagina do veiculo e extrai o texto em C
// (src/noticia.c + src/leitura.c). No Tizen (WASM) os mesmos tres pedidos morrem
// por CORS — o mesmo motivo de /v1/noticias. Estas duas rotas fazem o caminho do
// lado do servidor, com AS MESMAS REGRAS do extrator em C (quem mexer num mexe
// no outro):
//
//   GET /v1/noticia?u=<link do RSS>   (tambem ?url=)
//     1. link do Google News -> URL do veiculo: formato antigo (base64 com a
//        URL dentro) ou a pagina do artigo + POST batchexecute com a
//        assinatura (data-n-a-sg / data-n-a-ts); link do Bing -> o `url=`;
//     2. a pagina do veiculo, cortada em 512 KB, 10 s de prazo;
//     3. og:title/og:description/og:image/og:site_name, os <p> do no com mais
//        texto (pai + metade do avo, regra do Readability), o articleBody do
//        JSON-LD e o JSON do Arc XP quando o HTML nao da texto.
//     Responde {url, titulo, resumo, imagem, site, paragrafos[]} — `imagem` ja
//     apontando para /v1/noticia/img. Pagina que nao deixou ler responde 200
//     com a `url` e o resto vazio: e o que basta para o QR "Abrir no celular".
//   GET /v1/noticia/img?u=<og:image>
//     Repassa SO image/*, ate 3 MB, cache de 1 dia.
//
// LIMITES, porque o worker vira um buscador de URL arbitraria:
//   - so http/https, sem usuario/senha na URL, sem IP privado/loopback/
//     metadados/IPv6 literal/nome sem ponto (destinoPublico, de xtream.js);
//   - redirecionamento MANUAL, cada salto passa de novo pelo mesmo crivo, no
//     maximo 4;
//   - teto de bytes lido em fluxo (nao guarda mais que o teto na memoria);
//   - 10 s por pedido; cache de 1 h por link na borda;
//   - log so com o HOST, nunca a URL inteira (a query de agregador traz
//     rastreio, e o link do Google e longo e inutil no log).
import { destinoPublico } from "./xtream.js";

const PAGINA_MAX = 512 * 1024;
const GN_MAX = 768 * 1024;          // a pagina do artigo no Google tem ~570 KB
const IMG_MAX = 3 * 1024 * 1024;
const PRAZO_MS = 10000;
const SALTOS = 4;
const PAR_MAX = 6, PAR_TAM = 900, TOTAL = 4200;
const UA = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/128.0 Safari/537.36";
const CORS = { "access-control-allow-origin": "*", "cross-origin-resource-policy": "cross-origin" };

const json = (d, st = 200, extra = {}) => new Response(JSON.stringify(d), {
  status: st, headers: { "content-type": "application/json; charset=utf-8", ...CORS, ...extra } });

const host = (u) => { try { return new URL(u).hostname.replace(/^www\./, ""); } catch { return "?"; } };

// Le o corpo ate `max` bytes; passou disso, CORTA (nao falha): para HTML o
// comeco da pagina e o que importa, e o C faz o mesmo (rede_baixar_* com teto).
async function lerAte(resp, max) {
  const r = resp.body.getReader();
  const partes = [];
  let n = 0;
  for (;;) {
    const { done, value } = await r.read();
    if (done) break;
    const falta = max - n;
    if (value.byteLength >= falta) { partes.push(value.subarray(0, falta)); n = max; r.cancel().catch(() => {}); break; }
    partes.push(value); n += value.byteLength;
  }
  const b = new Uint8Array(n);
  let o = 0;
  for (const p of partes) { b.set(p, o); o += p.byteLength; }
  return b;
}

// GET/POST com o crivo em TODO salto. Devolve a Response final ou null.
export async function buscarSeguro(alvo, buscar, op = {}) {
  let u = destinoPublico(alvo);
  for (let salto = 0; u && salto <= SALTOS; salto++) {
    const ctl = new AbortController();
    const t = setTimeout(() => ctl.abort(), PRAZO_MS);
    let r;
    try {
      r = await buscar(u.toString(), { ...op, redirect: "manual", signal: ctl.signal,
        headers: { "user-agent": UA, "accept-language": "pt-BR,pt;q=0.9,en;q=0.8", ...(op.headers || {}) } });
    } catch { clearTimeout(t); return null; }
    clearTimeout(t);
    if (r.status >= 300 && r.status < 400) {
      const loc = r.headers.get("location");
      if (!loc) return null;
      u = destinoPublico(new URL(loc, u).toString());
      op = { ...op, method: "GET", body: undefined };
      continue;
    }
    return r;
  }
  return null;
}

// --- extrator (a mesma regra de src/leitura.c) -------------------------------

const ENT = { nbsp: 32, amp: 38, lt: 60, gt: 62, quot: 34, apos: 39, hellip: 8230, mdash: 8212,
  ndash: 8211, lsquo: 8216, rsquo: 8217, sbquo: 8218, ldquo: 8220, rdquo: 8221, bdquo: 8222,
  laquo: 171, raquo: 187, copy: 169, reg: 174, trade: 8482, deg: 176, middot: 183, bull: 8226,
  euro: 8364, ordf: 170, ordm: 186, iexcl: 161, iquest: 191, szlig: 223, shy: 173 };
const LAT1 = "Agrave Aacute Acirc Atilde Auml Aring AElig Ccedil Egrave Eacute Ecirc Euml Igrave Iacute Icirc Iuml ETH Ntilde Ograve Oacute Ocirc Otilde Ouml times Oslash Ugrave Uacute Ucirc Uuml Yacute THORN szlig agrave aacute acirc atilde auml aring aelig ccedil egrave eacute ecirc euml igrave iacute icirc iuml eth ntilde ograve oacute ocirc otilde ouml divide oslash ugrave uacute ucirc uuml yacute thorn yuml".split(" ");
LAT1.forEach((n, i) => { if (!(n in ENT)) ENT[n] = 192 + i; });

export function entidades(s) {
  return String(s || "").replace(/&(#x[0-9a-f]+|#[0-9]+|[a-z]+);/gi, (m, e) => {
    let c;
    if (e[0] === "#") c = e[1] === "x" || e[1] === "X" ? parseInt(e.slice(2), 16) : parseInt(e.slice(1), 10);
    else c = ENT[e];
    return c > 0 && c < 0x110000 ? String.fromCodePoint(c === 160 ? 32 : c) : m;
  });
}
const compacta = (s) => s.replace(/\s+/g, " ").trim();
function corta(s) {
  const b = new TextEncoder().encode(s);
  if (b.length < PAR_TAM) return s;
  let t = new TextDecoder().decode(b.subarray(0, PAR_TAM - 4)).replace(/\uFFFD+$/, "");
  const k = t.lastIndexOf(" ");
  if (k > t.length / 2) t = t.slice(0, k);
  return t + "\u2026";
}

const DESCARTA = new Set(["script", "style", "noscript", "nav", "header", "footer", "aside", "form",
  "figure", "figcaption", "button", "svg", "iframe", "template", "select", "textarea", "video",
  "audio", "object", "canvas"]);
const CRUA = new Set(["script", "style", "textarea", "template", "noscript"]);
const VAZIA = new Set(["br", "img", "meta", "link", "input", "hr", "source", "wbr", "area", "base",
  "col", "embed", "param", "track"]);
const BLOCO = new Set(["p", "div", "section", "article", "ul", "ol", "li", "table", "h1", "h2", "h3",
  "h4", "h5", "h6", "blockquote", "main"]);
const CONTEINER = new Set(["div", "section", "ul", "ol", "span", "p", "aside", "li"]);
const FORA = /comment|share|social|related|newsletter|promo|advert|sponsor|banner|cookie|popup|breadcrumb|subscribe|sidebar|recommend|outbrain|taboola|disqus|read-more|leia-tambem|saiba-mais|veja-tambem|author-bio|byline|caption|credit|footer|menu/i;
const SALVA = /article|content|main|body|post|entry|story|texto|materia|column/i;
const CORPO = /articlebody|article-body|article__body|article-content|entry-content|post-content|content-text|story-body|materia-conteudo|news-body|texto-materia/i;
const classeFora = (c) => !!c && FORA.test(c) && !SALVA.test(c);

function atributo(tag, nome) {
  const re = new RegExp("(?:^|\\s)" + nome + "\\s*=\\s*(?:\"([^\"]*)\"|'([^']*)'|([^\\s>]+))", "i");
  const m = tag.match(re);
  return m ? (m[1] ?? m[2] ?? m[3] ?? "") : null;
}

function absoluta(u, base) {
  if (!u) return "";
  try { return new URL(u, base).toString(); } catch { return u; }
}

function paragrafoDeJson(s) {
  const t = compacta(entidades(String(s).replace(/<[^>]*>/g, " ")));
  return t.length >= 60 ? corta(t) : "";
}

export function extrair(html, base) {
  const L = { titulo: "", resumo: "", imagem: "", site: "", paragrafos: [] };
  let tituloTag = "", twTit = "", twDesc = "", desc = "", img = "", twImg = "", ld = null;
  const pilha = [];
  const cand = [];
  let emP = false, par = "", nos = 0;
  const fechaP = () => {
    if (!emP) return;
    emP = false;
    const t = compacta(entidades(par));
    par = "";
    // BYTES, como o C (leitura.c conta strlen). Sem Buffer: o worker roda sem
    // nodejs_compat (wrangler.toml), e la `Buffer` nem existe.
    if (new TextEncoder().encode(t).length < 60) return;
    let ip = pilha.length - 1;
    while (ip >= 0 && pilha[ip].nome !== "p") ip--;
    cand.push({ pai: ip >= 1 ? pilha[ip - 1].id : -1, avo: ip >= 2 ? pilha[ip - 2].id : (ip === 1 ? -1 : 0),
      paiCorpo: ip >= 1 && pilha[ip - 1].corpo, avoCorpo: ip >= 2 && pilha[ip - 2].corpo, txt: corta(t) });
  };
  const re = /<!--[\s\S]*?-->|<![^>]*>|<\?[^>]*>|<(\/?)([a-zA-Z][a-zA-Z0-9-]*)((?:[^>"']|"[^"]*"|'[^']*')*)>|[^<]+|</g;
  let m;
  while ((m = re.exec(html))) {
    const [tudo, fecha, nomeCru, attrs] = m;
    if (nomeCru === undefined) {
      if (tudo[0] !== "<" && emP && !(pilha.length && pilha[pilha.length - 1].descarta)) par += tudo;
      continue;
    }
    const nome = nomeCru.toLowerCase();
    if (fecha) {
      if (emP && BLOCO.has(nome)) fechaP();
      for (let k = pilha.length - 1; k >= 0; k--) if (pilha[k].nome === nome) { pilha.length = k; break; }
      continue;
    }
    if (nome === "meta") {
      const chave = (atributo(attrs, "property") || atributo(attrs, "name") || atributo(attrs, "itemprop") || "").toLowerCase();
      const val = atributo(attrs, "content");
      if (chave && val) {
        if (chave === "og:title" && !L.titulo) L.titulo = val;
        else if (chave === "twitter:title" && !twTit) twTit = val;
        else if (chave === "og:description" && !L.resumo) L.resumo = val;
        else if (chave === "twitter:description" && !twDesc) twDesc = val;
        else if (chave === "description" && !desc) desc = val;
        else if (/^og:image(:url|:secure_url)?$/.test(chave) && !img) img = val;
        else if (/^twitter:image(:src)?$/.test(chave) && !twImg) twImg = val;
        else if (chave === "og:site_name" && !L.site) L.site = val;
      }
      continue;
    }
    if (nome === "title" && !tituloTag) {
      const f = html.toLowerCase().indexOf("</title", re.lastIndex);
      if (f >= 0) { tituloTag = html.slice(re.lastIndex, f); re.lastIndex = f; continue; }
    }
    if (CRUA.has(nome)) {
      const f = html.toLowerCase().indexOf("</" + nome, re.lastIndex);
      if (nome === "script" && !ld && /application\/ld\+json/i.test(atributo(attrs, "type") || "")) {
        const corpo = html.slice(re.lastIndex, f < 0 ? html.length : f);
        if (corpo.includes("\"articleBody\"")) ld = corpo;
      }
      re.lastIndex = f < 0 ? html.length : f;
      continue;
    }
    if (BLOCO.has(nome) && emP) fechaP();
    if (nome === "br" && emP) par += " ";
    if (!VAZIA.has(nome) && !attrs.trim().endsWith("/")) {
      const cls = atributo(attrs, "class") || "", id = atributo(attrs, "id") || "", ip = atributo(attrs, "itemprop") || "";
      const pai = pilha[pilha.length - 1];
      const nv = { nome, id: ++nos,
        descarta: (pai && pai.descarta) || DESCARTA.has(nome) || (CONTEINER.has(nome) && (classeFora(cls) || classeFora(id))),
        corpo: nome === "article" || CORPO.test(cls) || CORPO.test(id) || ip === "articleBody" };
      if (pilha.length < 256) pilha.push(nv);
      if (nome === "p" && !nv.descarta) { emP = true; par = ""; }
    }
  }
  fechaP();

  // O no vencedor.
  const nota = new Map();
  for (const c of cand) {
    if (c.pai) nota.set(c.pai, (nota.get(c.pai) || 0) + (c.paiCorpo ? 1.5 : 1) * c.txt.length);
    if (c.avo) nota.set(c.avo, (nota.get(c.avo) || 0) + (c.avoCorpo ? 0.75 : 0.5) * c.txt.length);
  }
  let no = 0, melhor = -1;
  for (const [k, v] of nota) if (v > melhor) { melhor = v; no = k; }
  let total = 0;
  const poe = (t) => {
    if (L.paragrafos.length >= PAR_MAX || total >= TOTAL || !t || L.paragrafos.includes(t)) return;
    L.paragrafos.push(t); total += t.length;
  };
  for (const c of cand) if (no && (c.pai === no || c.avo === no)) poe(c.txt);

  if (L.paragrafos.length < 2 && ld) {
    const k = ld.match(/"articleBody"\s*:\s*("(?:[^"\\]|\\.)*")/);
    if (k) {
      let corpo = "";
      try { corpo = JSON.parse(k[1]); } catch {}
      L.paragrafos = []; total = 0;
      for (const linha of corpo.split("\n")) poe(paragrafoDeJson(linha));
    }
  }
  if (L.paragrafos.length < 2) {
    const arc = [];
    for (const a of html.matchAll(/"content":("(?:[^"\\]|\\.)*"),"type":"text"/g)) {
      let t = "";
      try { t = JSON.parse(a[1]); } catch {}
      const p = paragrafoDeJson(t);
      if (p && arc.length < PAR_MAX && !arc.includes(p)) arc.push(p);
    }
    if (arc.length > L.paragrafos.length) L.paragrafos = arc;
  }
  if (!L.titulo) L.titulo = twTit || tituloTag;
  if (!L.resumo) L.resumo = twDesc || desc;
  L.titulo = compacta(entidades(L.titulo));
  L.resumo = compacta(entidades(L.resumo));
  L.site = compacta(entidades(L.site));
  const cru = compacta(entidades(img || twImg));
  L.imagem = cru.startsWith("data:") ? "" : absoluta(cru, base);
  if (L.paragrafos.length > 1 && L.resumo && L.paragrafos[0].startsWith(L.resumo.slice(0, 80)))
    L.paragrafos.shift();
  return L;
}

// --- Google News / Bing ----------------------------------------------------------

export function gnId(link) {
  const m = /^https:\/\/news\.google\.com\/(?:rss\/)?articles\/([A-Za-z0-9_-]+)/.exec(link);
  return m ? m[1] : null;
}
export function gnAntigo(id) {
  let bin;
  try { bin = atob(id.replace(/-/g, "+").replace(/_/g, "/")); } catch { return null; }
  const m = /https?:\/\/[\x21-\x7e]+/.exec(bin);
  return m && m[0].includes(".") ? m[0] : null;
}
export function bingDireto(link) {
  try {
    const u = new URL(link);
    if (!/(^|\.)bing\.com$/.test(u.hostname) || !u.pathname.startsWith("/news/apiclick")) return null;
    const d = u.searchParams.get("url");
    return d && /^https?:\/\//.test(d) ? d : null;
  } catch { return null; }
}

async function resolver(link, buscar) {
  const b = bingDireto(link);
  if (b) return b;
  const id = gnId(link);
  if (!id) return link;
  const antigo = gnAntigo(id);
  if (antigo) return antigo;
  const r = await buscarSeguro(link, buscar);
  if (!r || !r.ok) return null;
  const pg = new TextDecoder().decode(await lerAte(r, GN_MAX));
  const sg = /data-n-a-sg="([A-Za-z0-9_-]+)"/.exec(pg), ts = /data-n-a-ts="(\d+)"/.exec(pg);
  if (!sg || !ts) return null;
  const req = JSON.stringify([[["Fbv4je", JSON.stringify(["garturlreq",
    [["X", "X", ["X", "X"], null, null, 1, 1, "US:en", null, 1, null, null, null, null, null, 0, 1], "X", "X", 1, [1, 1, 1], 1, 1, null, 0, 0, null, 0],
    id, Number(ts[1]), sg[1]]), null, "generic"]]]);
  const r2 = await buscarSeguro("https://news.google.com/_/DotsSplashUi/data/batchexecute", buscar, {
    method: "POST", body: "f.req=" + encodeURIComponent(req),
    headers: { "content-type": "application/x-www-form-urlencoded;charset=UTF-8" } });
  if (!r2 || !r2.ok) return null;
  const txt = await r2.text();
  const u = /garturlres\\",\\"(https?:\/\/[^"\\]+(?:\\\\u[0-9a-f]{4}[^"\\]*)*)\\"/i.exec(txt);
  return u ? u[1].replace(/\\\\u([0-9a-f]{4})/gi, (_, h) => String.fromCharCode(parseInt(h, 16))) : null;
}

function charsetDe(resp, bytes) {
  const ct = resp.headers.get("content-type") || "";
  const cab = new TextDecoder("latin1").decode(bytes.subarray(0, 4096));
  return /charset=["']?(iso-8859-1|windows-1252|latin1)/i.test(ct + " " + cab) ? "windows-1252" : "utf-8";
}

export async function rotaNoticia(url, buscar = fetch, cache = null) {
  // `u` e o que o cliente manda; `url` e aceito pelo mesmo caminho (nome
  // mais obvio para quem testa a rota na mao).
  const link = (url.searchParams.get("u") || url.searchParams.get("url") || "").slice(0, 600);
  if (!destinoPublico(link)) return json({ erro: "link recusado" }, 400);
  const origem = url.origin;
  const chave = new Request(origem + "/v1/noticia?u=" + encodeURIComponent(link));
  if (cache) { const c = await cache.match(chave); if (c) return new Response(c.body, c); }
  const vazio = { url: "", titulo: "", resumo: "", imagem: "", site: "", paragrafos: [] };
  let final = null;
  try { final = await resolver(link, buscar); } catch { final = null; }
  let saida = vazio;
  if (final && destinoPublico(final)) {
    saida = { ...vazio, url: final };
    const r = await buscarSeguro(final, buscar, { headers: { accept: "text/html,application/xhtml+xml;q=0.9,*/*;q=0.8" } });
    if (r && r.ok) {
      const bytes = await lerAte(r, PAGINA_MAX);
      const html = new TextDecoder(charsetDe(r, bytes)).decode(bytes);
      const L = extrair(html, final);
      saida = { url: final, titulo: L.titulo, resumo: L.resumo, site: L.site, paragrafos: L.paragrafos,
        imagem: L.imagem && destinoPublico(L.imagem) ? origem + "/v1/noticia/img?u=" + encodeURIComponent(L.imagem) : "" };
    }
    console.log(`noticia ${host(final)} -> ${r ? r.status : "sem resposta"}, ${saida.paragrafos.length} par`);
  } else console.log(`noticia ${host(link)} -> nao resolvido`);
  // Sem texto (prazo, 403 de anti-robo, link nao resolvido) guarda so 5 min:
  // o veiculo que caiu agora merece outra tentativa antes de uma hora.
  const resp = json(saida, 200, { "cache-control": "public, max-age=" + (saida.paragrafos.length ? 3600 : 300) });
  if (cache) await cache.put(chave, resp.clone());
  return resp;
}

export async function rotaNoticiaImg(url, buscar = fetch, cache = null) {
  const alvo = (url.searchParams.get("u") || "").slice(0, 1000);
  if (!destinoPublico(alvo)) return json({ erro: "imagem recusada" }, 400);
  const chave = new Request(url.origin + "/v1/noticia/img?u=" + encodeURIComponent(alvo));
  if (cache) { const c = await cache.match(chave); if (c) return new Response(c.body, c); }
  const r = await buscarSeguro(alvo, buscar, { headers: { accept: "image/*" } });
  const tipo = r ? (r.headers.get("content-type") || "").split(";")[0].trim().toLowerCase() : "";
  if (!r || !r.ok || !tipo.startsWith("image/") || tipo === "image/svg+xml") {
    console.log(`noticia-img ${host(alvo)} -> recusada (${r ? r.status : 0} ${tipo || "sem tipo"})`);
    return json({ erro: "nao e imagem" }, 502);
  }
  const tam = Number(r.headers.get("content-length") || 0);
  if (tam > IMG_MAX) return json({ erro: "imagem grande demais" }, 502);
  const bytes = await lerAte(r, IMG_MAX + 1);
  if (bytes.byteLength > IMG_MAX) return json({ erro: "imagem grande demais" }, 502);
  const resp = new Response(bytes, { status: 200, headers: { "content-type": tipo,
    "cache-control": "public, max-age=86400", ...CORS } });
  if (cache) await cache.put(chave, resp.clone());
  return resp;
}
