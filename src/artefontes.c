#include "artefontes.h"
#include "js.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void af_codificar(const char *s, char *dst, size_t n) {
  static const char hex[] = "0123456789ABCDEF";
  size_t k = 0;
  if (!dst || !n) return;
  for (; s && *s && k + 4 < n; s++) {
    unsigned char c = (unsigned char)*s;
    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
        c == '-' || c == '_' || c == '.' || c == '~') dst[k++] = (char)c;
    else { dst[k++] = '%'; dst[k++] = hex[c >> 4]; dst[k++] = hex[c & 15]; }
  }
  dst[k] = 0;
}

static int hexval(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -1;
}

const char *af_decodificar(const char *s, char *dst, size_t n) {
  size_t k = 0;
  if (!s) { if (dst && n) dst[0] = 0; return s; }
  while (*s && *s != '/') {
    char c = *s;
    if (c == '%' && hexval(s[1]) >= 0 && hexval(s[2]) >= 0) {
      c = (char)(hexval(s[1]) * 16 + hexval(s[2]));
      s += 3;
    } else s++;
    if (dst && k + 1 < n) dst[k++] = c;
  }
  if (dst && n) dst[k] = 0;
  return s;
}

// ---------------------------------------------------------------- TMDB
//
// POR QUE "OUTRO" E NAO O PADRAO (medido em 23/09 em 15 titulos do topo do
// Cinemeta, comparando as imagens reduzidas a 64x36): o fundo do metahub — que
// e o do card nas fileiras do Cinemeta — e UM dos backdrops do TMDB,
// reencodado (bytes diferentes, mesma foto). Em 3 de 15 ele e o proprio
// backdrop_path padrao; o fanart do Trakt e o padrao do TMDB em quase todos.
// Ou seja, "TMDB" e "Trakt" no destaque mostravam a foto do card com outro
// arquivo. O outro backdrop sem texto e a unica foto do TMDB que so coincide
// por acaso (2 de 15 no mesmo teste).
int af_tmdb_fundos(const char *corpo, const char *evitar,
                   char *padrao, size_t np, char *outro, size_t no) {
  const char *el;
  char melhor[160] = "";
  int melhorTexto = 2;
  double melhorNota = -1.0, melhorVotos = -1.0;
  if (padrao && np) padrao[0] = 0;
  if (outro && no) outro[0] = 0;
  if (!corpo) return 0;
  if (padrao && np) js_texto_raiz(corpo, "backdrop_path", padrao, np);
  for (el = js_array(corpo, NULL, "backdrops"); el; el = js_prox(js_fim(el))) {
    const char *fim = js_fim(el);
    char caminho[160], lingua[16];
    int texto;
    double nota, votos;
    if (!js_texto(el, fim, "file_path", caminho, sizeof caminho) || caminho[0] != '/') continue;
    if (padrao && padrao[0] && !strcmp(caminho, padrao)) continue;
    if (evitar && evitar[0] && !strcmp(caminho, evitar)) continue;
    // iso_639_1 null (ou ausente) = foto sem titulo escrito.
    texto = js_texto(el, fim, "iso_639_1", lingua, sizeof lingua) ? 1 : 0;
    nota = js_num(el, fim, "vote_average", 0.0);
    votos = js_num(el, fim, "vote_count", 0.0);
    if (texto < melhorTexto ||
        (texto == melhorTexto && (nota > melhorNota ||
                                  (nota == melhorNota && votos > melhorVotos)))) {
      snprintf(melhor, sizeof melhor, "%s", caminho);
      melhorTexto = texto; melhorNota = nota; melhorVotos = votos;
    }
  }
  if (!melhor[0] || !outro || !no) return 0;
  snprintf(outro, no, "%s", melhor);
  return 1;
}

// IDIOMA DA ARTE AUTOMATICA (relato do .tpk 4/5: "as vezes arabe, as vezes
// coreano"). O backdrop_path da raiz e o fundo que o TMDB elege, e ele pode ter
// letreiro em qualquer lingua; o logo tambem vinha por ordem de lista. A regra:
// o idioma da pessoa, depois o sem texto (fundo) / o ingles (logo), e NADA
// alem disso — melhor manter a arte atual que mostrar uma lingua que ninguem
// pediu.
static int linguaDe(const char *el, const char *fim, char *iso, size_t n) {
  iso[0] = 0;
  js_texto(el, fim, "iso_639_1", iso, n);
  return iso[0] != 0;
}

int af_tmdb_fundo_padrao(const char *corpo, const char *base, const char *raiz,
                         char *out, size_t n) {
  const char *el;
  char melhor[2][160] = { "", "" };
  double nota[2] = { -1.0, -1.0 }, votos[2] = { -1.0, -1.0 };
  int temLista = 0;
  if (!out || !n) return 0;
  out[0] = 0;
  if (!corpo) return 0;
  for (el = js_array(corpo, NULL, "backdrops"); el; el = js_prox(js_fim(el))) {
    const char *fim = js_fim(el);
    char caminho[160], iso[16];
    int k;
    double nt, vt;
    temLista = 1;
    if (!js_texto(el, fim, "file_path", caminho, sizeof caminho) || caminho[0] != '/') continue;
    if (!linguaDe(el, fim, iso, sizeof iso)) k = 1;
    else if (base && base[0] && !strncmp(iso, base, 2) && !iso[2]) k = 0;
    else continue;
    if (raiz && !strcmp(caminho, raiz)) { snprintf(out, n, "%s", raiz); return 1; }
    nt = js_num(el, fim, "vote_average", 0.0);
    vt = js_num(el, fim, "vote_count", 0.0);
    if (nt > nota[k] || (nt == nota[k] && vt > votos[k])) {
      snprintf(melhor[k], sizeof melhor[k], "%s", caminho);
      nota[k] = nt; votos[k] = vt;
    }
  }
  // Sem a lista nao ha como julgar: fica o da raiz.
  if (!temLista) { if (raiz && raiz[0] == '/') { snprintf(out, n, "%s", raiz); return 1; } return 0; }
  if (melhor[0][0]) { snprintf(out, n, "%s", melhor[0]); return 1; }
  if (melhor[1][0]) { snprintf(out, n, "%s", melhor[1]); return 1; }
  return 0;
}

int af_tmdb_logo(const char *corpo, const char *base, char *fp, size_t n,
                 char *iso, size_t ni) {
  const char *el;
  char achado[3][160] = { "", "", "" };
  int k;
  if (fp && n) fp[0] = 0;
  if (iso && ni) iso[0] = 0;
  if (!corpo || !fp || !n) return 0;
  for (el = js_array(corpo, NULL, "logos"); el; el = js_prox(js_fim(el))) {
    const char *fim = js_fim(el);
    char caminho[160], l[16];
    size_t L;
    if (!js_texto(el, fim, "file_path", caminho, sizeof caminho) || caminho[0] != '/') continue;
    L = strlen(caminho);
    if (L > 4 && !strcmp(caminho + L - 4, ".svg")) continue;   // nao decodifica
    if (!linguaDe(el, fim, l, sizeof l)) k = 2;
    else if (base && base[0] && !strncmp(l, base, 2) && !l[2]) k = 0;
    else if (!strcmp(l, "en")) k = 1;
    else continue;
    // A lista vem da mais votada para a menos: a primeira de cada classe fica.
    if (!achado[k][0]) snprintf(achado[k], sizeof achado[k], "%s", caminho);
  }
  for (k = 0; k < 3; k++)
    if (achado[k][0]) {
      snprintf(fp, n, "%s", achado[k]);
      if (iso && ni) snprintf(iso, ni, "%s", k == 0 ? base : k == 1 ? "en" : "und");
      return 1;
    }
  return 0;
}

// ---------------------------------------------------------------- fanart.tv
int af_fanart_fundo(const char *corpo, int serie, char *url, size_t n) {
  const char *el;
  char melhor[400] = "";
  int melhorLingua = 2;
  long melhorLikes = -1;
  if (url && n) url[0] = 0;
  if (!corpo || !url || !n) return 0;
  for (el = js_array(corpo, NULL, serie ? "showbackground" : "moviebackground");
       el; el = js_prox(js_fim(el))) {
    const char *fim = js_fim(el);
    char u[400], lingua[16] = "", likes[16] = "";
    int temLingua;
    long l;
    if (!js_texto(el, fim, "url", u, sizeof u) || strncmp(u, "http", 4)) continue;
    js_texto(el, fim, "lang", lingua, sizeof lingua);
    js_texto(el, fim, "likes", likes, sizeof likes);
    temLingua = lingua[0] && strcmp(lingua, "00") ? 1 : 0;
    l = atol(likes);
    if (temLingua < melhorLingua || (temLingua == melhorLingua && l > melhorLikes)) {
      snprintf(melhor, sizeof melhor, "%s", u);
      melhorLingua = temLingua; melhorLikes = l;
    }
  }
  if (!melhor[0]) return 0;
  snprintf(url, n, "%s", melhor);
  return 1;
}

// ---------------------------------------------------------------- Kitsu
static int kitsuItem(const char *el, int ano, int serie, char *url, size_t n) {
  const char *fim = js_fim(el), *cap;
  char sub[24] = "", data[16] = "", u[400];
  js_texto(el, fim, "subtype", sub, sizeof sub);
  js_texto(el, fim, "startDate", data, sizeof data);
  if (serie ? !strcmp(sub, "movie") : (sub[0] && strcmp(sub, "movie"))) return 0;
  if (ano > 0 && data[0]) {
    int a = atoi(data);
    if (a > 0 && abs(a - ano) > 1) return 0;
  }
  cap = strstr(el, "\"coverImage\"");
  if (!cap || cap > fim) return 0;
  if (!js_texto(cap, fim, "large", u, sizeof u) && !js_texto(cap, fim, "original", u, sizeof u))
    return 0;
  if (strncmp(u, "http", 4)) return 0;
  snprintf(url, n, "%s", u);
  return 1;
}

int af_kitsu_capa(const char *corpo, int ano, int serie, char *url, size_t n) {
  const char *el;
  if (url && n) url[0] = 0;
  if (!corpo || !url || !n) return 0;
  el = js_array(corpo, NULL, "data");
  if (el) {
    for (; el; el = js_prox(js_fim(el)))
      if (*el == '{' && kitsuItem(el, ano, serie, url, n)) return 1;
    return 0;
  }
  // /anime/{id}: "data" e um objeto so.
  el = strstr(corpo, "\"data\"");
  if (!el) return 0;
  el = strchr(el + 6, '{');
  return el ? kitsuItem(el, 0, serie, url, n) : 0;
}

// ---------------------------------------------------------------- AniList
int af_anilist_banner(const char *corpo, int ano, char *url, size_t n) {
  const char *m, *sd;
  char u[400];
  if (url && n) url[0] = 0;
  if (!corpo || !url || !n) return 0;
  m = strstr(corpo, "\"Media\"");
  if (!m) return 0;
  if (!js_texto(m, NULL, "bannerImage", u, sizeof u) || strncmp(u, "http", 4)) return 0;
  if (ano > 0 && (sd = strstr(m, "\"startDate\"")) != NULL) {
    int a = (int)js_num(sd, NULL, "year", 0.0);
    if (a > 0 && abs(a - ano) > 1) return 0;
  }
  snprintf(url, n, "%s", u);
  return 1;
}

// ---------------------------------------------------------------- Apple
int af_apple_tamanho(const char *modelo, int larg, char *url, size_t n) {
  const char *m;
  int alt;
  if (url && n) url[0] = 0;
  if (!modelo || !url || !n || larg <= 0) return 0;
  m = strstr(modelo, "{w}x{h}");
  if (!m || strncmp(modelo, "http", 4)) return 0;
  alt = larg * 9 / 16;
  snprintf(url, n, "%.*s%dx%d.jpg", (int)(m - modelo), modelo, larg, alt);
  return 1;
}
