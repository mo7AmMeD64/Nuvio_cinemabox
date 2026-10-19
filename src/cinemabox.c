// Provedor CinemaBox embutido — ver cinemabox.h para o desenho.
//
// Tudo que sai daqui e no formato que descoberta.c/addons.c ja leem: o corpo do
// manifesto, {"metas":[...]} dos catalogos, {"meta":{...}} da ficha e
// {"streams":[...]} das fontes. Nenhum modulo de UI sabe que o CinemaBox existe.
#include "cinemabox.h"
#include "js.h"
#include "rede.h"

#include <ctype.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>

#define CB_API     "https://cinema.albox.co/api/v4"
#define CB_PREFIXO "cinemabox://cb"
#define CB_UA      "Dalvik/2.1.0 (Linux; U; Android 14; 23043RP34G Build/UKQ1.240624.001)"
// Teto de temporadas de uma serie (CAT_TEMP_MAX e 64) e de fios por rodada.
#define CB_EST_MAX 64
#define CB_FIOS    6

// Fileiras da Home: as secoes que o app do CinemaBox mostra, uma por catalogo
// declarado no manifesto. O id e o id da secao no /home.
static const char CB_MANIFESTO[] =
"{\"id\":\"space.nuvio.cinemabox\",\"version\":\"1.0.0\",\"name\":\"CinemaBox\","
"\"description\":\"cinema.albox.co\",\"types\":[\"movie\",\"series\"],"
"\"resources\":["
"{\"name\":\"catalog\",\"types\":[\"movie\",\"series\"]},"
"{\"name\":\"meta\",\"types\":[\"movie\",\"series\"],\"idPrefixes\":[\"cb:\"]},"
"{\"name\":\"stream\",\"types\":[\"movie\",\"series\"],\"idPrefixes\":[\"cb:\"]}],"
"\"catalogs\":["
"{\"type\":\"movie\",\"id\":\"144\",\"name\":\"Featured\"},"
"{\"type\":\"movie\",\"id\":\"238\",\"name\":\"Weekly Trending\"},"
"{\"type\":\"movie\",\"id\":\"112\",\"name\":\"Movies must see\"},"
"{\"type\":\"movie\",\"id\":\"104\",\"name\":\"Movies\"},"
"{\"type\":\"series\",\"id\":\"105\",\"name\":\"Series\"},"
"{\"type\":\"series\",\"id\":\"101\",\"name\":\"Shows You Must Watch\"},"
"{\"type\":\"series\",\"id\":\"106\",\"name\":\"Anime\"},"
"{\"type\":\"series\",\"id\":\"118\",\"name\":\"Top Anime\"},"
"{\"type\":\"series\",\"id\":\"107\",\"name\":\"TV-SHOW\"},"
"{\"type\":\"series\",\"id\":\"108\",\"name\":\"Cartoon\"},"
"{\"type\":\"movie\",\"id\":\"search\",\"name\":\"Search\","
"\"extra\":[{\"name\":\"search\",\"isRequired\":true}]},"
"{\"type\":\"series\",\"id\":\"search\",\"name\":\"Search\","
"\"extra\":[{\"name\":\"search\",\"isRequired\":true}]}]}";

// ---------------------------------------------------------------- HTTP
static char *cb_baixar(const char *caminho, int segundos) {
  char url[512];
  int w = snprintf(url, sizeof url, "%s/%s", CB_API, caminho);
  if (w < 0 || (size_t)w >= sizeof url) return NULL;
  return rede_baixar(url, segundos);
}

// O /home alimenta TODAS as fileiras; uma busca por fileira seriam 10 GETs
// identicos no arranque. Copia propria porque o chamador libera o retorno.
static pthread_mutex_t homeTrava = PTHREAD_MUTEX_INITIALIZER;
static char *homeCorpo;
static time_t homeEm;
static char *cb_home(void) {
  time_t agora = time(NULL);
  char *r = NULL;
  pthread_mutex_lock(&homeTrava);
  if (homeCorpo && agora - homeEm <= 120) r = strdup(homeCorpo);
  pthread_mutex_unlock(&homeTrava);
  if (r) return r;
  r = cb_baixar("home", 20);
  if (!r) return NULL;
  pthread_mutex_lock(&homeTrava);
  free(homeCorpo);
  homeCorpo = strdup(r);
  homeEm = agora;
  pthread_mutex_unlock(&homeTrava);
  return r;
}

// ---------------------------------------------------------------- JSON
typedef struct { char *p; size_t n, cap; } CbBuf;
static int cb_cresce(CbBuf *b, size_t extra) {
  if (b->n + extra + 1 <= b->cap) return 1;
  size_t cap = b->cap ? b->cap : 4096;
  while (cap < b->n + extra + 1) cap += cap / 2 + 1024;
  char *p = realloc(b->p, cap);
  if (!p) return 0;
  b->p = p;
  b->cap = cap;
  return 1;
}
static int cb_bruto(CbBuf *b, const char *s, size_t n) {
  if (!cb_cresce(b, n)) return 0;
  memcpy(b->p + b->n, s, n);
  b->n += n;
  b->p[b->n] = 0;
  return 1;
}
static int cb_cru(CbBuf *b, const char *s) {
  return !s || cb_bruto(b, s, strlen(s));
}
static int cb_escapar(CbBuf *b, const char *s) {
  static const char *HEX = "0123456789abcdef";
  const unsigned char *p;
  if (!s) return 1;
  for (p = (const unsigned char *)s; *p; p++) {
    unsigned char c = *p;
    char e[8];
    size_t n;
    if (c == '"' || c == '\\') { e[0] = '\\'; e[1] = (char)c; n = 2; }
    else if (c == '\n') { e[0] = '\\'; e[1] = 'n'; n = 2; }
    else if (c < 0x20) {
      e[0] = '\\'; e[1] = 'u'; e[2] = '0'; e[3] = '0';
      e[4] = HEX[c >> 4]; e[5] = HEX[c & 15]; n = 6;
    } else { e[0] = (char)c; n = 1; }
    if (!cb_bruto(b, e, n)) return 0;
  }
  return 1;
}
// Campo de texto com virgula no fim; o ultimo e limpo por cb_fecha.
static void cb_campo(CbBuf *b, const char *chave, const char *valor) {
  if (!valor || !valor[0]) return;
  cb_cru(b, "\"");
  cb_cru(b, chave);
  cb_cru(b, "\":\"");
  cb_escapar(b, valor);
  cb_cru(b, "\",");
}
static void cb_numero(CbBuf *b, const char *chave, double v, int casas) {
  char tmp[64];
  if (casas > 0) snprintf(tmp, sizeof tmp, "\"%s\":%.*f,", chave, casas, v);
  else snprintf(tmp, sizeof tmp, "\"%s\":%d,", chave, (int)(v + 0.5));
  cb_cru(b, tmp);
}
static void cb_fecha(CbBuf *b) {
  if (b->n && b->p[b->n - 1] == ',') b->p[--b->n] = 0;
  cb_cru(b, "}");
}
static void cb_segunda_virgula(CbBuf *b, int *contador) {
  if (*contador) cb_cru(b, ",");
  (*contador)++;
}
// "genres":["Drama","Thriller"] e o mesmo formato para qualquer array de
// strings. Nada e escrito quando o array esta ausente/vazio.
static void cb_lista_textos(CbBuf *b, const char *chave, const char *ini,
                            const char *fim, const char *nomeArray) {
  const char *g = js_array(ini, fim, nomeArray);
  size_t antes;
  int n = 0;
  if (!g) return;
  antes = b->n;
  cb_cru(b, "\"");
  cb_cru(b, chave);
  cb_cru(b, "\":[");
  while (g) {
    char tmp[128] = "";
    const char *q;
    if (js_cadeia(g, tmp, sizeof tmp) && tmp[0]) {
      if (n) cb_cru(b, ",");
      cb_cru(b, "\"");
      cb_escapar(b, tmp);
      cb_cru(b, "\"");
      n++;
    }
    q = g + 1;
    while (*q) {
      if (*q == '\\' && q[1]) q += 2;
      else if (*q == '"') break;
      else q++;
    }
    if (*q != '"') break;
    g = js_prox(q + 1);
    if (n >= 20) break;
  }
  if (!n) { b->p[antes] = 0; b->n = antes; return; }
  cb_cru(b, "],");
}

static const char *cb_tipo(const char *t) {
  if (!t) return NULL;
  if (!strcasecmp(t, "movie")) return "movie";
  if (!strcasecmp(t, "series") || !strcasecmp(t, "anime") ||
      !strcasecmp(t, "cartoon") || !strcasecmp(t, "tvshow")) return "series";
  return NULL;
}
static int cb_primeiro_numero(const char *s) {
  if (!s) return 0;
  for (; *s; s++) if (isdigit((unsigned char)*s)) return atoi(s);
  return 0;
}
static int cb_ano_ms(long long ms) {
  time_t seg;
  struct tm tm;
  if (ms <= 0) return 0;
  seg = (time_t)(ms / 1000);
  if (!gmtime_r(&seg, &tm)) return 0;
  return tm.tm_year + 1900;
}

// UM item de catalogo. Poster e obrigatorio (deMeta descarta sem ele).
static void cb_item(CbBuf *b, const char *idNumerico, const char *tipo,
                    const char *titulo, const char *poster, const char *logo,
                    const char *fundo, const char *sinopse, const char *ano,
                    double nota, const char *iniGen, const char *fimGen) {
  char cbid[64];
  if (!titulo || !titulo[0] || !poster || !poster[0] || !tipo) return;
  snprintf(cbid, sizeof cbid, "cb:%s", idNumerico);
  cb_cru(b, "{");
  cb_campo(b, "id", cbid);
  cb_campo(b, "type", tipo);
  cb_campo(b, "name", titulo);
  cb_campo(b, "poster", poster);
  cb_campo(b, "background", fundo);
  cb_campo(b, "logo", logo);
  cb_campo(b, "description", sinopse);
  cb_campo(b, "releaseInfo", ano);
  if (nota > 0.0 && nota <= 10.0) cb_numero(b, "imdbRating", nota, 1);
  if (iniGen) cb_lista_textos(b, "genres", iniGen, fimGen, "genres");
  cb_fecha(b);
}

// ---------------------------------------------------------------- ids
// "cb:28846" -> 28846. 0 quando nao ha numero.
static long cb_id_numerico(const char *id) {
  const char *p = id ? strchr(id, ':') : NULL;
  return p ? atol(p + 1) : 0;
}
static void cb_url_decodar(const char *s, char *dst, size_t tam) {
  size_t o = 0;
  for (; *s && o + 1 < tam; s++) {
    if (*s == '%' && isxdigit((unsigned char)s[1]) && isxdigit((unsigned char)s[2])) {
      char h[3] = { s[1], s[2], 0 };
      dst[o++] = (char)strtol(h, NULL, 16);
      s += 2;
    } else if (*s == '+') dst[o++] = ' ';
    else dst[o++] = *s;
  }
  dst[o] = 0;
}

// ---------------------------------------------------------------- catalogos
// Cartao do CinemaBox (id numerico, type/title/style) -> item Stremio.
static void cb_item_cartao(CbBuf *b, int *contador, const char *ini, const char *fim) {
  char titulo[200] = "", poster[1024] = "", logo[1024] = "", fundo[1024] = "",
       sinopse[900] = "", t[24] = "", id[24] = "";
  const char *estilo;
  long nid;
  const char *tipo;
  if (*ini != '{') return;
  js_texto(ini, fim, "type", t, sizeof t);
  tipo = cb_tipo(t);
  if (!tipo) return;
  nid = (long)js_num(ini, fim, "id", 0);
  if (nid <= 0) return;
  js_texto(ini, fim, "title", titulo, sizeof titulo);
  js_texto(ini, fim, "description", sinopse, sizeof sinopse);
  estilo = strstr(ini, "\"style\"");
  if (estilo && estilo < fim) {
    const char *fe = strchr(estilo, '{');
    if (fe && fe < fim) {
      const char *ff = js_fim(fe);
      js_texto(fe, ff, "image", poster, sizeof poster);
      js_texto(fe, ff, "logo", logo, sizeof logo);
      js_texto(fe, ff, "background_image", fundo, sizeof fundo);
    }
  }
  snprintf(id, sizeof id, "%ld", nid);
  cb_segunda_virgula(b, contador);
  cb_item(b, id, tipo, titulo, poster, logo, fundo, sinopse, NULL, 0.0, NULL, NULL);
}

static char *cb_catalogo_fileira(const char *secaoId) {
  char *home = cb_home();
  CbBuf b = {0};
  const char *p;
  int escritos = 0;
  if (!home) return NULL;
  p = js_array(home, NULL, "sections");
  for (; p; p = js_prox(js_fim(p))) {
    const char *f = js_fim(p);
    const char *d;
    if ((long)js_num(p, f, "id", -1) != atol(secaoId)) continue;
    cb_cru(&b, "{\"metas\":[");
    d = js_array(p, f, "data");
    for (; d; d = js_prox(js_fim(d))) cb_item_cartao(&b, &escritos, d, js_fim(d));
    cb_cru(&b, "]}");
    free(home);
    return b.p;
  }
  free(home);
  free(b.p);
  return NULL;
}

static char *cb_catalogo_busca(const char *tipo, const char *termo) {
  CbBuf b = {0};
  char esc[600];
  size_t o = 0;
  const char *s;
  int pagina, escritos = 0;
  for (s = termo; *s && o + 4 < sizeof esc; s++) {
    unsigned char c = (unsigned char)*s;
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') esc[o++] = (char)c;
    else o += (size_t)snprintf(esc + o, sizeof esc - o, "%%%02X", c);
  }
  esc[o] = 0;
  cb_cru(&b, "{\"metas\":[");
  for (pagina = 1; pagina <= 3 && escritos < 75; pagina++) {
    char cam[900], *corpo;
    const char *r;
    snprintf(cam, sizeof cam, "search?page_size=25&page_number=%d&term=%s", pagina, esc);
    corpo = cb_baixar(cam, 15);
    if (!corpo) break;
    r = js_array(corpo, NULL, "results");
    for (; r; r = js_prox(js_fim(r))) {
      const char *f = js_fim(r);
      char tt[24] = "", titulo[200] = "", poster[1024] = "", ano[8] = "", id[24] = "";
      const char *estilo;
      long nid;
      if (*r != '{') continue;
      js_texto(r, f, "type", tt, sizeof tt);
      if (!cb_tipo(tt) || strcmp(cb_tipo(tt), tipo)) continue;
      nid = (long)js_num(r, f, "id", 0);
      js_texto(r, f, "title", titulo, sizeof titulo);
      estilo = strstr(r, "\"style\"");
      if (estilo && estilo < f) {
        const char *fe = strchr(estilo, '{');
        if (fe && fe < f) js_texto(fe, js_fim(fe), "image", poster, sizeof poster);
      }
      { double a = js_num(r, f, "year", 0); if (a > 0) snprintf(ano, sizeof ano, "%d", (int)a); }
      if (nid <= 0 || !titulo[0] || !poster[0]) continue;
      snprintf(id, sizeof id, "%ld", nid);
      cb_segunda_virgula(&b, &escritos);
      cb_item(&b, id, cb_tipo(tt), titulo, poster, NULL, NULL, NULL, ano, 0.0, NULL, NULL);
    }
    free(corpo);
  }
  cb_cru(&b, "]}");
  return b.p;
}

static char *cb_catalogo(const char *tipo, const char *id, const char *termo) {
  if (!strcmp(id, "search")) return termo && termo[0] ? cb_catalogo_busca(tipo, termo) : NULL;
  return cb_catalogo_fileira(id);
}

// ---------------------------------------------------------------- ficha
typedef struct { long sid; char *corpo; } CbEstacao;
static void *cb_estacao_fio(void *u) {
  CbEstacao *e = u;
  char cam[64];
  snprintf(cam, sizeof cam, "shows/seasons/player/%ld", e->sid);
  e->corpo = cb_baixar(cam, 20);
  return NULL;
}

// Episodios de uma temporada (resposta do /shows/seasons/player) -> videos[].
static void cb_videos_estacao(CbBuf *v, const char *corpo, int temporada, int *quantos) {
  const char *p = js_array(corpo, NULL, "episodes");
  int indice = 0;
  for (; p; p = js_prox(js_fim(p))) {
    const char *f = js_fim(p);
    char id[32] = "", titulo[200] = "", imagem[1024] = "";
    long nid;
    int numero;
    double dur;
    if (*p != '{') continue;
    indice++;
    nid = (long)js_num(p, f, "id", 0);
    if (nid <= 0) continue;
    numero = (int)js_num(p, f, "episode_number", 0);
    if (numero <= 0) numero = indice;
    js_texto(p, f, "title", titulo, sizeof titulo);
    js_texto(p, f, "image", imagem, sizeof imagem);
    dur = js_num(p, f, "length", 0);
    snprintf(id, sizeof id, "cb:%ld", nid);
    if (*quantos) cb_cru(v, ",");
    cb_cru(v, "{");
    cb_campo(v, "id", id);
    cb_numero(v, "season", temporada, 0);
    cb_numero(v, "episode", numero, 0);
    if (titulo[0]) cb_campo(v, "name", titulo);
    cb_campo(v, "thumbnail", imagem);
    if (dur > 0) cb_numero(v, "runtime", dur / 60.0, 0);
    cb_fecha(v);
    (*quantos)++;
  }
}

// A ficha de titulo do CinemaBox -> {"meta":{...}}, com videos[] na serie.
static char *cb_meta(const char *tipo, const char *id) {
  char cam[64], *corpo = NULL;
  const char *pi, *pif;
  CbBuf campos = {0}, videos = {0}, saida = {0};
  char titulo[200] = "", poster[1024] = "", logo[1024] = "", fundo[1024] = "",
       sinopse[900] = "", idade[16] = "", ano[8] = "";
  const char *tipoReal;
  long epFilme = 0;
  int nVideos = 0, nEst = 0, i;

  snprintf(cam, sizeof cam, "shows/shows/dynamic/%ld", cb_id_numerico(id));
  corpo = cb_baixar(cam, 20);
  if (!corpo) return NULL;
  pi = strstr(corpo, "\"post_info\"");
  pi = pi ? strchr(pi, '{') : NULL;
  if (!pi) { free(corpo); return NULL; }
  pif = js_fim(pi);
  if (!pif || pif <= pi) pif = corpo + strlen(corpo);
  js_texto(pi, pif, "title", titulo, sizeof titulo);
  if (!titulo[0]) { free(corpo); return NULL; }
  js_texto(pi, pif, "image", poster, sizeof poster);
  js_texto(pi, pif, "logo", logo, sizeof logo);
  js_texto(pi, pif, "background_image", fundo, sizeof fundo);
  js_texto(pi, pif, "description", sinopse, sizeof sinopse);
  { char t[24] = "";
    js_texto(pi, pif, "type", t, sizeof t);
    tipoReal = cb_tipo(t); }
  if (!tipoReal) tipoReal = cb_tipo(tipo);
  if (!tipoReal) tipoReal = "movie";
  { const char *r = strstr(pi, "\"rating\"");
    if (r && r < pif) {
      const char *re = strchr(r, '{');
      if (re && re < pif) {
        const char *rf = js_fim(re);
        double nota = js_num(re, rf, "value", 0.0);
        js_texto(re, rf, "age", idade, sizeof idade);
        if (nota > 0.0 && nota <= 10.0) cb_numero(&campos, "imdbRating", nota, 1);
      }
    } }
  { char ms[24] = "";
    if (js_texto(pi, pif, "release_date", ms, sizeof ms) && ms[0]) {
      int a = cb_ano_ms(atoll(ms));
      if (a > 0) snprintf(ano, sizeof ano, "%d", a);
    } }
  { double dur = js_num(pi, pif, "length", 0);
    if (dur > 0) cb_numero(&campos, "runtime", dur / 60.0, 0); }
  cb_campo(&campos, "id", id);
  cb_campo(&campos, "type", tipoReal);
  cb_campo(&campos, "name", titulo);
  cb_campo(&campos, "poster", poster);
  cb_campo(&campos, "background", fundo);
  cb_campo(&campos, "logo", logo);
  cb_campo(&campos, "description", sinopse);
  cb_campo(&campos, "releaseInfo", ano);
  cb_campo(&campos, "certification", idade);
  cb_lista_textos(&campos, "genres", pi, pif, "genres");

  if (!strcmp(tipoReal, "movie")) {
    long player;
    epFilme = (long)js_num(pi, pif, "episode_id", 0);
    player = epFilme > 0 ? epFilme : cb_id_numerico(id);
    free(corpo);
    cb_cru(&saida, "{\"meta\":{");
    cb_bruto(&saida, campos.p ? campos.p : "", campos.n);
    cb_cru(&saida, "\"videos\":[{\"id\":\"cb:");
    { char pid[24]; snprintf(pid, sizeof pid, "%ld", player); cb_cru(&saida, pid); }
    cb_cru(&saida, "\",\"season\":1,\"episode\":1}]}}");
    free(campos.p);
    return saida.p;
  }

  // SERIE: as capas de temporada estao em qualquer secao, em cartao "season".
  {
    long sid[CB_EST_MAX];
    int num[CB_EST_MAX];
    CbEstacao est[CB_EST_MAX];
    const char *sec = js_array(corpo, NULL, "sections");
    int indice = 0, k;
    for (; sec; sec = js_prox(js_fim(sec))) {
      const char *fs = js_fim(sec);
      const char *d = js_array(sec, fs, "data");
      for (; d; d = js_prox(js_fim(d))) {
        const char *fd = js_fim(d);
        char t[24] = "", ct[32] = "", tt[64] = "";
        long nid;
        int duplicado = 0, j;
        if (*d != '{') continue;
        js_texto(d, fd, "type", t, sizeof t);
        js_texto(d, fd, "card_type", ct, sizeof ct);
        if (strcasecmp(t, "season") && strcasecmp(ct, "seasonPoster")) continue;
        nid = (long)js_num(d, fd, "id", 0);
        if (nid <= 0 || nEst >= CB_EST_MAX) continue;
        for (j = 0; j < nEst; j++) if (sid[j] == nid) { duplicado = 1; break; }
        if (duplicado) continue;
        js_texto(d, fd, "title", tt, sizeof tt);
        sid[nEst] = nid;
        num[nEst] = cb_primeiro_numero(tt);
        if (num[nEst] <= 0) num[nEst] = indice + 1;
        nEst++;
      }
      indice++;
    }
    // Numeros repetidos: empurra para depois do maior ja visto.
    for (k = 0; k < nEst; k++) {
      int j, max = 0, repetido = 0;
      for (j = 0; j < k; j++) {
        if (num[j] == num[k]) repetido = 1;
        if (num[j] > max) max = num[j];
      }
      if (repetido) num[k] = max + 1;
    }
    // Nenhum cartao: a propria resposta ja tem a temporada corrente.
    if (nEst == 0) {
      long atual = (long)js_num(pi, pif, "current_season_id", 0);
      if (atual > 0) {
        int n = (int)js_num(pi, pif, "season_number", 1);
        sid[0] = atual;
        num[0] = n > 0 ? n : 1;
        nEst = 1;
      }
    }
    for (i = 0; i < nEst; i++) { est[i].sid = sid[i]; est[i].corpo = NULL; }
    for (i = 0; i < nEst; i += CB_FIOS) {
      pthread_t th[CB_FIOS];
      int feitos = 0, j;
      for (j = i; j < nEst && j < i + CB_FIOS; j++)
        if (pthread_create(&th[feitos], NULL, cb_estacao_fio, &est[j]) == 0) feitos++;
        else cb_estacao_fio(&est[j]);
      for (j = 0; j < feitos; j++) pthread_join(th[j], NULL);
    }
    free(corpo);
    for (i = 0; i < nEst; i++) {
      if (est[i].corpo) {
        cb_videos_estacao(&videos, est[i].corpo, num[i], &nVideos);
        free(est[i].corpo);
      }
    }
  }

  cb_cru(&saida, "{\"meta\":{");
  cb_bruto(&saida, campos.p ? campos.p : "", campos.n);
  cb_cru(&saida, "\"videos\":[");
  cb_bruto(&saida, videos.p ? videos.p : "", videos.n);
  cb_cru(&saida, "]}}");
  free(campos.p);
  free(videos.p);
  return saida.p;
}

// ---------------------------------------------------------------- fontes
static char *cb_stream(const char *tipo, const char *id) {
  long player = cb_id_numerico(id);
  char cam[64], *corpo;
  CbBuf b = {0};
  const char *v;
  int escritos = 0;
  if (player <= 0) return NULL;
  // Filme: o id do catalogo e o show; quem toca e o episode_id dele.
  if (!strcmp(tipo, "movie")) {
    char *d;
    snprintf(cam, sizeof cam, "shows/shows/dynamic/%ld", player);
    d = cb_baixar(cam, 20);
    if (!d) return NULL;
    { const char *pi = strstr(d, "\"post_info\"");
      pi = pi ? strchr(pi, '{') : NULL;
      if (pi) {
        const char *pif = js_fim(pi);
        long ep = (long)js_num(pi, pif && pif > pi ? pif : d + strlen(d), "episode_id", 0);
        if (ep > 0) player = ep;
      } }
    free(d);
  }
  snprintf(cam, sizeof cam, "shows/episodes/player/%ld", player);
  corpo = cb_baixar(cam, 20);
  if (!corpo) return NULL;
  cb_cru(&b, "{\"streams\":[");
  v = js_array(corpo, NULL, "videos");
  for (; v; v = js_prox(js_fim(v))) {
    const char *f = js_fim(v);
    char url[4096] = "", qualidade[32] = "", rotulo[128];
    if (*v != '{') continue;
    js_texto(v, f, "url", url, sizeof url);
    if (!url[0]) continue;
    js_texto(v, f, "quality", qualidade, sizeof qualidade);
    snprintf(rotulo, sizeof rotulo, "CinemaBox%s%s", qualidade[0] ? " " : "", qualidade);
    if (escritos) cb_cru(&b, ",");
    cb_cru(&b, "{\"name\":\"");
    cb_escapar(&b, rotulo);
    cb_cru(&b, "\",\"title\":\"");
    cb_escapar(&b, qualidade[0] ? qualidade : rotulo);
    cb_cru(&b, "\",\"url\":\"");
    cb_escapar(&b, url);
    cb_cru(&b, "\",\"behaviorHints\":{\"proxyHeaders\":{\"request\":{\"User-Agent\":\"");
    cb_escapar(&b, CB_UA);
    cb_cru(&b, "\"}}},\"addonName\":\"CinemaBox\"}");
    escritos++;
  }
  cb_cru(&b, "]}");
  free(corpo);
  return b.p;
}

// ---------------------------------------------------------------- rotas
static const char *cb_seg(const char **cursor, char *dst, size_t tam) {
  const char *p = *cursor;
  size_t n = 0;
  while (*p == '/') p++;
  while (*p && *p != '/') {
    if (n + 1 < tam) dst[n++] = *p;
    p++;
  }
  dst[n] = 0;
  *cursor = p;
  return n ? dst : NULL;
}

char *cinemabox_responder(const char *url, int *nosso) {
  char alvo[1024], tipo[16] = "", id[96] = "", termo[300] = "";
  const char *p, *q, *cursor;

  if (nosso) *nosso = 0;
  if (!url || strncmp(url, CB_PREFIXO, sizeof CB_PREFIXO - 1)) return NULL;
  if (nosso) *nosso = 1;
  p = url + sizeof CB_PREFIXO - 1;
  q = strchr(p, '?');
  { size_t n = q ? (size_t)(q - p) : strlen(p);
    if (n >= sizeof alvo) n = sizeof alvo - 1;
    memcpy(alvo, p, n);
    alvo[n] = 0; }
  if (!strncmp(alvo, "/manifest.json", 14)) {
    char *r = malloc(sizeof CB_MANIFESTO);
    if (r) memcpy(r, CB_MANIFESTO, sizeof CB_MANIFESTO);
    return r;
  }
  if (!strncmp(alvo, "/catalog/", 9)) {
    cursor = alvo + 8;
    if (!cb_seg(&cursor, tipo, sizeof tipo)) return NULL;
    if (!cb_seg(&cursor, id, sizeof id)) return NULL;
    { char *fim = strstr(id, ".json"); if (fim) *fim = 0; }
    if (!strncmp(cursor, "/search=", 8)) {
      snprintf(termo, sizeof termo, "%s", cursor + 8);
      { char *fim = strstr(termo, ".json"); if (fim) *fim = 0; }
      { char dec[300]; cb_url_decodar(termo, dec, sizeof dec); snprintf(termo, sizeof termo, "%s", dec); }
    }
    return cb_catalogo(tipo, id, termo);
  }
  if (!strncmp(alvo, "/meta/", 6) || !strncmp(alvo, "/stream/", 8)) {
    int ehMeta = alvo[1] == 'm';
    cursor = alvo + (ehMeta ? 5 : 7);
    if (!cb_seg(&cursor, tipo, sizeof tipo)) return NULL;
    if (!cb_seg(&cursor, id, sizeof id)) return NULL;
    { char *fim = strstr(id, ".json"); if (fim) *fim = 0; }
    return ehMeta ? cb_meta(tipo, id) : cb_stream(tipo, id);
  }
  return NULL;
}
