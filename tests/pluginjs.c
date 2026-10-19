// Motor dos plugins (src/pluginjs.c) — F09. Sem rede externa: o que usa rede
// fala com tests/plugins_server.py em 127.0.0.1.
//   * ambiente (cheerio, CryptoJS, URL, TextDecoder, module.exports);
//   * promessa esquecida, laco infinito, setInterval eterno, sintaxe;
//   * teto de memoria do runtime e ORCAMENTO GLOBAL entre runtimes;
//   * fetch: ok, prazo por fetch, corpo grande (com e sem Content-Length),
//     cota de fetches, so http(s), HTML hostil no teto do DOM;
//   * cancelamento (flag) e GERACAO (rede_grupo_avancar) no meio de um fetch,
//     com o resultado descartado e o tempo de reacao medido.
#include "pluginjs.h"
#include "streams.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static int falhas;
static char BASE[128];
#define CONFERE(c, m) do { if (!(c)) { printf("FALHOU: %s\n", m); falhas++; } else printf("ok: %s\n", m); } while (0)

typedef struct { PjPedido p; PjResultado r; char *src; } Arg;
static void *fio(void *u) { Arg *a = u; pj_executar(&a->p, &a->r); return NULL; }
static void preparar(Arg *a, const char *src, int prazo) {
  memset(a, 0, sizeof *a);
  a->src = malloc(strlen(src) + 200);
  sprintf(a->src, "var BASE='%s';\n%s", BASE, src);
  a->p.codigo = a->src; a->p.nCodigo = strlen(a->src); a->p.arquivo = "teste.js";
  a->p.idScraper = "teste"; a->p.nomeScraper = "Teste"; a->p.tmdbId = "603"; a->p.tipo = "tv";
  a->p.temporada = 2; a->p.episodio = 5; a->p.prazoMs = prazo; a->p.ajustesJson = "{\"q\":1}";
}
static pthread_t disparar(Arg *a) {
  pthread_t t; pthread_attr_t at;
  pthread_attr_init(&at); pthread_attr_setstacksize(&at, pj_pilha());
  pthread_create(&t, &at, fio, a);
  pthread_attr_destroy(&at);
  return t;
}
static void rodar(Arg *a, const char *src, int prazo) {
  preparar(a, src, prazo);
  pthread_join(disparar(a), NULL);
}
static void soltar(Arg *a) { pj_resultado_soltar(&a->r); free(a->src); a->src = NULL; }
static unsigned long agoraMs(void) {
  struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
  return (unsigned long)ts.tv_sec * 1000UL + (unsigned long)ts.tv_nsec / 1000000UL;
}
static void dormir(int ms) { struct timespec t = { ms / 1000, (ms % 1000) * 1000000L }; nanosleep(&t, NULL); }

static const char *BOM =
  "\"use strict\";\n"
  "const cheerio = require('cheerio-without-node-native');\n"
  "const CryptoJS = require('crypto-js');\n"
  "const HTML = '<ul class=\"q\"><li><a href=\"/v/1?a=1&amp;b=2\">Epi 1 <b>1080p</b></a></li>"
  "<li><a href=\"/v/2\" data-q=\"720\">Epi 2</a><li><a>sem link</a></ul><p id=x>  muitos   espacos </p>';\n"
  "async function getStreams(tmdb, tipo, t, e) {\n"
  "  await new Promise(r => setTimeout(r, 30));\n"
  "  const $ = cheerio.load(HTML);\n"
  "  const out = [];\n"
  "  $('ul.q li a[href]').each((i, el) => {\n"
  "    const u = new URL($(el).attr('href'), 'https://exemplo.org/base/');\n"
  "    out.push({ name: 'Teste', title: $(el).text(), url: u.href, quality: $(el).find('b').text() || ($(el).attr('data-q') || '') + 'p',\n"
  "      size: '1.2 GB', headers: { Referer: 'https://exemplo.org/' } });\n"
  "  });\n"
  "  const md5 = CryptoJS.MD5('abc').toString();\n"
  "  const txt = new TextDecoder().decode(new TextEncoder().encode('ção'));\n"
  "  out.push({ name: 'Info', title: [tmdb, tipo, t, e, md5, txt, $('#x').text(), SCRAPER_SETTINGS.q, $('li').length, $('a').eq(1).parent().is('li')].join('|'), url: 'https://exemplo.org/info.m3u8' });\n"
  "  out.push({ name: 'Ruim', url: { url: '[object Object]' } });\n"
  "  return out;\n"
  "}\n"
  "module.exports = { getStreams };\n";

static void semRede(void) {
  Arg a;
  Stream *l = NULL;
  int n;
  rodar(&a, BOM, 5000);
  CONFERE(!a.r.erro[0], "ambiente: sem erro");
  CONFERE(a.r.n == 3, "ambiente: 3 fontes (a de url invalida sai)");
  CONFERE(a.r.criptoUsado, "CryptoJS carregado sob demanda");
  n = a.r.json ? stream_extrair(a.r.json, "Teste", &l) : 0;
  CONFERE(n == 3, "stream_extrair le o JSON do mapeador");
  if (n == 3) {
    CONFERE(!strcmp(l[0].url, "https://exemplo.org/v/1?a=1&b=2"), "URL relativa + entidade do atributo");
    CONFERE(strstr(l[2].descricao, "603|tv|2|5|900150983cd24fb0d6963f7d28e17f72|\xc3\xa7\xc3\xa3o|muitos espacos|1|3|true") != NULL,
            "argumentos, MD5, TextDecoder, texto Jsoup, SCRAPER_SETTINGS, eq/parent/is");
  }
  free(l); soltar(&a);

  rodar(&a, "module.exports.getStreams = () => new Promise(() => {});", 3000);
  CONFERE(a.r.n == 0 && strstr(a.r.erro, "nunca resolveu"), "promessa esquecida volta na hora");
  soltar(&a);
  rodar(&a, "module.exports.getStreams = async () => { for(;;){} };", 400);
  CONFERE(a.r.estourou && a.r.ms < 1500, "laco infinito cortado pelo prazo");
  soltar(&a);
  rodar(&a, "module.exports.getStreams = async () => { setInterval(() => {}, 50); return new Promise(() => {}); };", 400);
  CONFERE(a.r.estourou && a.r.ms < 1500, "setInterval eterno cortado pelo prazo");
  soltar(&a);
  rodar(&a, "module.exports.getStreams = async () => { for (let i = 0; i < 1000; i++) setTimeout(() => {}, 100000); return []; };", 2000);
  CONFERE(a.r.n == 0 && strstr(a.r.erro, "timer quota"), "cota de timers");
  soltar(&a);
  rodar(&a, "function getStreams( {", 2000);
  CONFERE(a.r.n == 0 && strstr(a.r.erro, "codigo do scraper"), "erro de sintaxe vira erro, nao queda");
  soltar(&a);
  preparar(&a, "module.exports.getStreams = async () => { const a = []; for(;;) a.push('x'.repeat(1e6)); };", 8000);
  a.p.memoriaMax = 24L * 1024 * 1024;
  pthread_join(disparar(&a), NULL);
  CONFERE(a.r.n == 0 && a.r.semMemoria && a.r.memPico <= 24u * 1024 * 1024, "teto de memoria do runtime");
  if (!a.r.semMemoria || a.r.memPico > 24u * 1024 * 1024) printf("  semMemoria=%d pico=%zu erro=%s\n", a.r.semMemoria, a.r.memPico, a.r.erro);
  CONFERE(pj_orcamento_heap_uso() == 0, "orcamento global devolvido ao fim");
  soltar(&a);
}

// Dois runtimes com teto proprio de 48 MB cada, orcamento global de 40 MB:
// a soma nunca passa do global, e os dois voltam.
static void orcamentoGlobal(void) {
  Arg a, b;
  pthread_t ta, tb;
  const char *GULOSO = "module.exports.getStreams = async () => { const a = []; for(;;) { a.push('y'.repeat(2e5)); await null; } };";
  pj_orcamento_definir(40u * 1024 * 1024, 0);
  pj_orcamento_zerar_pico();
  preparar(&a, GULOSO, 6000); preparar(&b, GULOSO, 6000);
  ta = disparar(&a); tb = disparar(&b);
  pthread_join(ta, NULL); pthread_join(tb, NULL);
  CONFERE(a.r.semMemoria || b.r.semMemoria, "orcamento global recusou alocacao");
  CONFERE(pj_orcamento_heap_pico() <= 40u * 1024 * 1024, "pico global <= 40 MB");
  CONFERE(pj_orcamento_heap_uso() == 0, "orcamento global zerado depois dos dois");
  soltar(&a); soltar(&b);
  pj_orcamento_definir(128u * 1024 * 1024, 0);
}

static void comRede(void) {
  Arg a;
  unsigned long t0;
  RedeGrupo *g;
  rodar(&a, "module.exports.getStreams = async () => { const r = await fetch(BASE + '/ok', {headers: {'Host': 'mal', 'X-Y': '1'}});"
            " const j = await r.json(); const h = await fetch(BASE + '/redir');"
            " return [{name:'x', url:'https://e.org/' + j.v + '/' + r.status + '/' + (h.redirected ? 'r' : 'n')}]; };", 5000);
  CONFERE(a.r.n == 1 && a.r.json && strstr(a.r.json, "https://e.org/42/200/r"), "fetch ok, Host descartado, redirect seguido");
  soltar(&a);

  preparar(&a, "module.exports.getStreams = async () => { try { await fetch(BASE + '/lento?ms=3000'); return [{name:'x',url:'https://e.org/a'}]; }"
               " catch (e) { return [{name:'erro', url:'https://e.org/' + encodeURIComponent(String(e.message))}]; } };", 8000);
  a.p.redeSegundos = 1;
  t0 = agoraMs();
  pthread_join(disparar(&a), NULL);
  CONFERE(a.r.json && strstr(a.r.json, "prazo") && agoraMs() - t0 < 2500, "prazo por fetch (1 s) corta servidor lento");
  soltar(&a);

  preparar(&a, "module.exports.getStreams = async () => { try { await fetch(BASE + '/grande?mb=8'); return []; }"
               " catch (e) { return [{name:'e', url:'https://e.org/' + encodeURIComponent(String(e.message))}]; } };", 8000);
  a.p.redeMaxBytes = 1024 * 1024;
  pthread_join(disparar(&a), NULL);
  CONFERE(a.r.json && strstr(a.r.json, "teto"), "corpo com Content-Length acima do teto recusado");
  CONFERE(pj_orcamento_rede_uso() == 0, "reserva de rede devolvida");
  soltar(&a);

  preparar(&a, "module.exports.getStreams = async () => { try { await fetch(BASE + '/grande-chunk?mb=6'); return []; }"
               " catch (e) { return [{name:'e', url:'https://e.org/' + encodeURIComponent(String(e.message))}]; } };", 8000);
  a.p.redeMaxBytes = 1024 * 1024;
  pthread_join(disparar(&a), NULL);
  CONFERE(a.r.json && strstr(a.r.json, "teto"), "corpo chunked acima do teto recusado");
  soltar(&a);

  rodar(&a, "module.exports.getStreams = async () => { const ps = []; for (let i = 0; i < 400; i++) ps.push(fetch(BASE + '/ok').catch(() => 0));"
            " await Promise.all(ps); return []; };", 15000);
  CONFERE(a.r.fetchesRecusados > 0 && a.r.fetches <= 64 + 6, "cota de fila de fetch por runtime");
  CONFERE(pj_orcamento_rede_uso() == 0, "reserva de rede devolvida depois da rajada");
  soltar(&a);

  rodar(&a, "module.exports.getStreams = async () => { try { await fetch('file:///etc/passwd'); } catch (e) {} return []; };", 2000);
  CONFERE(a.r.n == 0 && a.r.fetchesFalhos == 1, "so http(s) sai");
  soltar(&a);

  preparar(&a, "const cheerio = require('cheerio-without-node-native');"
               "module.exports.getStreams = async () => { const r = await fetch(BASE + '/html?n=60000'); const t = await r.text();"
               " try { const $ = cheerio.load(t); return [{name:'x', url:'https://e.org/' + $('a').length}]; }"
               " catch (e) { return [{name:'e', url:'https://e.org/' + encodeURIComponent(String(e.message))}]; } };", 15000);
  a.p.domMax = 4L * 1024 * 1024;
  pthread_join(disparar(&a), NULL);
  CONFERE(a.r.json && strstr(a.r.json, "DOM"), "HTML grande para no teto do DOM (antes de montar)");
  soltar(&a);
  preparar(&a, "const cheerio = require('cheerio-without-node-native');"
               "module.exports.getStreams = async () => { const r = await fetch(BASE + '/html?n=2000'); const $ = cheerio.load(await r.text());"
               " return [{name:'x', url:'https://e.org/' + $('div.c > a[href^=\"/v/\"]').length}]; };", 15000);
  pthread_join(disparar(&a), NULL);
  CONFERE(a.r.json && strstr(a.r.json, "https://e.org/2000"), "HTML dentro do teto: seletor sobre resposta real");
  soltar(&a);

  // Cancelamento pela flag no meio de um fetch lento.
  { int cancel = 0;
    pthread_t t;
    preparar(&a, "module.exports.getStreams = async () => { await fetch(BASE + '/lento?ms=5000'); return [{name:'x',url:'https://e.org/a'}]; };", 15000);
    a.p.cancelado = &cancel;
    t0 = agoraMs();
    t = disparar(&a);
    dormir(300); __atomic_store_n(&cancel, 1, __ATOMIC_RELEASE);
    pthread_join(t, NULL);
    CONFERE(a.r.cancelado && a.r.n == 0 && !a.r.json, "cancelamento descarta o resultado");
    CONFERE(agoraMs() - t0 < 1500, "cancelamento reage em menos de 1,2 s");
    soltar(&a); }

  // Geracao: troca de perfil (grupo avancado) no meio do fetch.
  g = rede_grupo_criar();
  { pthread_t t;
    preparar(&a, "module.exports.getStreams = async () => { await fetch(BASE + '/pinga'); return [{name:'x',url:'https://e.org/a'}]; };", 15000);
    a.p.grupo = g; a.p.job = rede_job_criar(g);
    t0 = agoraMs();
    t = disparar(&a);
    dormir(400); rede_grupo_avancar(g);
    pthread_join(t, NULL);
    CONFERE(a.r.cancelado && !a.r.json, "geracao nova descarta execucao antiga");
    CONFERE(agoraMs() - t0 < 1500, "geracao nova interrompe fetch em voo");
    rede_job_soltar(a.p.job);
    soltar(&a); }
  // Execucao que COMECA com geracao velha nem roda.
  { RedeJob *j = rede_job_criar(g);
    rede_grupo_avancar(g);
    rodar(&a, BOM, 5000);
    soltar(&a);
    preparar(&a, BOM, 5000); a.p.job = j;
    pthread_join(disparar(&a), NULL);
    CONFERE(a.r.cancelado && a.r.n == 0 && a.r.ms == 0, "job de geracao velha nao executa");
    rede_job_soltar(j); soltar(&a); }
  rede_grupo_soltar(g);
  // Os fios de rede que ficaram com pedidos abandonados terminam e devolvem.
  for (int k = 0; k < 60 && pj_orcamento_rede_uso(); k++) dormir(100);
  CONFERE(pj_orcamento_rede_uso() == 0 && pj_orcamento_heap_uso() == 0, "nada vaza no orcamento global");
}

int main(int argc, char **argv) {
  if (argc > 1) snprintf(BASE, sizeof BASE, "%s", argv[1]);
  semRede();
  orcamentoGlobal();
  if (BASE[0]) comRede();
  printf(falhas ? "pluginjs: %d falha(s)\n" : "pluginjs: ok\n", falhas);
  return falhas != 0;
}
