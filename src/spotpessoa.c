// Ver spotpessoa.h: o porque do debounce e do cache.
#include "spotpessoa.h"
#include "buscanorm.h"
#include "descoberta.h"
#include "rede.h"
#include "js.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SPP_CACHE 8
#define SPP_TERMO 96

typedef struct {
  char termo[SPP_TERMO];   // normalizado
  int  n;                  // -1 = falhou (nao fica no cache como resposta)
  SpotPessoa p[SPP_MAX];
  unsigned uso;
} Resp;

static pthread_mutex_t trava = PTHREAD_MUTEX_INITIALIZER;
static Resp cache[SPP_CACHE];
static unsigned relogioUso, geracao;
static char querido[SPP_TERMO];     // o que o campo tem agora (normalizado)
static unsigned queridoDesde;
static char emVoo[SPP_TERMO];
static int  fioVivo, disparos;
static char *(*baixarTeste)(const char *url);

// --- leitura ------------------------------------------------------------------
int spotpessoa_extrair(const char *json, SpotPessoa *out, int max) {
  const char *r;
  int n = 0;
  if (!json || !out || max <= 0) return 0;
  r = js_array(json, NULL, "results");
  while (r && n < max) {
    const char *fr = js_fim(r), *kf, *lim;
    SpotPessoa s;
    char cam[128] = "";
    memset(&s, 0, sizeof s);
    if (!fr) break;
    // Os campos da pessoa vem ANTES de known_for no TMDB, e known_for tem
    // "id" e "name" proprios: a leitura do id para no comeco dele.
    kf = js_array(r, fr, "known_for");
    lim = kf ? kf : fr;
    s.tmdb = (long)js_num(r, lim, "id", 0.0);
    if (s.tmdb <= 0) s.tmdb = (long)js_num(r, fr, "id", 0.0);
    if (!js_texto_raiz_em(r, fr, "name", s.nome, sizeof s.nome)) s.nome[0] = 0;
    if (js_texto_raiz_em(r, fr, "profile_path", cam, sizeof cam) && cam[0] == '/')
      snprintf(s.foto, sizeof s.foto, "https://image.tmdb.org/t/p/w185%s", cam);
    { const char *k = kf; size_t z = 0; int nk = 0;
      while (k && nk < 3) {
        const char *fk = js_fim(k);
        char t[96] = "", tipo[16] = "";
        if (!fk) break;
        if (!js_texto(k, fk, "title", t, sizeof t)) js_texto(k, fk, "name", t, sizeof t);
        js_texto(k, fk, "media_type", tipo, sizeof tipo);
        if (!s.tituloTmdb && (!strcmp(tipo, "movie") || !strcmp(tipo, "tv"))) {
          s.tituloTmdb = (long)js_num(k, fk, "id", 0.0);
          snprintf(s.tituloTipo, sizeof s.tituloTipo, "%s", tipo);
        }
        if (t[0]) {
          size_t l = strlen(t);
          if (z && z + 5 < sizeof s.conhecido) { memcpy(s.conhecido + z, " \xc2\xb7 ", 4); z += 4; }
          if (z + l + 1 >= sizeof s.conhecido) break;
          memcpy(s.conhecido + z, t, l); z += l; s.conhecido[z] = 0; nk++;
        }
        k = js_prox(fk);
      } }
    if (s.tmdb > 0 && s.nome[0] && s.tituloTmdb > 0) out[n++] = s;
    r = js_prox(fr);
  }
  return n;
}

// --- cache ----------------------------------------------------------------------
static Resp *achar(const char *termo) {
  int i;
  for (i = 0; i < SPP_CACHE; i++)
    if (cache[i].termo[0] && !strcmp(cache[i].termo, termo)) return &cache[i];
  return NULL;
}
static Resp *vaga(void) {
  int i, v = 0;
  for (i = 0; i < SPP_CACHE; i++) {
    if (!cache[i].termo[0]) return &cache[i];
    if (cache[i].uso < cache[v].uso) v = i;
  }
  return &cache[v];
}

static void codificar(const char *s, char *dst, size_t cap) {
  size_t z = 0;
  const unsigned char *c;
  for (c = (const unsigned char *)s; *c && z + 4 < cap; c++) {
    if ((*c >= 'a' && *c <= 'z') || (*c >= 'A' && *c <= 'Z') || (*c >= '0' && *c <= '9')) dst[z++] = (char)*c;
    else if (*c == ' ') dst[z++] = '+';
    else { snprintf(dst + z, 4, "%%%02X", *c); z += 3; }
  }
  dst[z] = 0;
}

static void *fio(void *arg) {
  char termo[SPP_TERMO], url[700], enc[300], *corpo;
  const char *chave = desc_chave_tmdb();
  SpotPessoa p[SPP_MAX];
  int n = -1;
  (void)arg;
  pthread_mutex_lock(&trava);
  snprintf(termo, sizeof termo, "%s", emVoo);
  pthread_mutex_unlock(&trava);
  if ((!chave || !chave[0]) && baixarTeste) chave = "TESTE";
  if (chave && chave[0]) {
    codificar(termo, enc, sizeof enc);
    snprintf(url, sizeof url, "https://api.themoviedb.org/3/search/person?api_key=%s"
             "&language=%s&include_adult=false&page=1&query=%s", chave, desc_tmdb_idioma(), enc);
    corpo = baixarTeste ? baixarTeste(url) : rede_baixar(url, 10);
    if (corpo) { n = spotpessoa_extrair(corpo, p, SPP_MAX); free(corpo); }
  }
  pthread_mutex_lock(&trava);
  if (n >= 0) {
    Resp *r = achar(termo);
    if (!r) r = vaga();
    snprintf(r->termo, sizeof r->termo, "%s", termo);
    r->n = n;
    memcpy(r->p, p, sizeof(SpotPessoa) * (size_t)n);
    r->uso = ++relogioUso;
    geracao++;
  }
  emVoo[0] = 0;
  fioVivo = 0;
  pthread_mutex_unlock(&trava);
  printf("[spotlight] pessoas tmdb: %d\n", n);
  fflush(stdout);
  return NULL;
}

void spotpessoa_pedir(const char *termo, unsigned agora) {
  char t[SPP_TERMO];
  busca_normalizar(termo ? termo : "", t, sizeof t);
  pthread_mutex_lock(&trava);
  if (strcmp(t, querido)) { snprintf(querido, sizeof querido, "%s", t); queridoDesde = agora; }
  pthread_mutex_unlock(&trava);
}

void spotpessoa_atualizar(unsigned agora) {
  pthread_t t;
  pthread_mutex_lock(&trava);
  if (fioVivo || busca_codepoints(querido) < SPP_MIN_CP || agora - queridoDesde < SPP_ESPERA_MS ||
      achar(querido) || (!desc_chave_tmdb()[0] && !baixarTeste)) {
    pthread_mutex_unlock(&trava);
    return;
  }
  snprintf(emVoo, sizeof emVoo, "%s", querido);
  fioVivo = 1;
  disparos++;
  pthread_mutex_unlock(&trava);
  if (pthread_create(&t, NULL, fio, NULL) != 0) {
    pthread_mutex_lock(&trava);
    fioVivo = 0; emVoo[0] = 0;
    // sem fio: a mesma espera antes de tentar de novo, nao um pedido por quadro
    queridoDesde = agora;
    pthread_mutex_unlock(&trava);
  } else pthread_detach(t);
}

int spotpessoa_n(const char *termo) {
  char t[SPP_TERMO];
  Resp *r;
  int n;
  busca_normalizar(termo ? termo : "", t, sizeof t);
  pthread_mutex_lock(&trava);
  r = achar(t);
  n = r ? r->n : -1;
  if (r) r->uso = ++relogioUso;
  pthread_mutex_unlock(&trava);
  return n;
}

int spotpessoa_item(const char *termo, int i, SpotPessoa *out) {
  char t[SPP_TERMO];
  Resp *r;
  int ok = 0;
  busca_normalizar(termo ? termo : "", t, sizeof t);
  pthread_mutex_lock(&trava);
  r = achar(t);
  if (r && i >= 0 && i < r->n && out) { *out = r->p[i]; ok = 1; }
  pthread_mutex_unlock(&trava);
  return ok;
}

unsigned spotpessoa_geracao(void) {
  unsigned g;
  pthread_mutex_lock(&trava);
  g = geracao;
  pthread_mutex_unlock(&trava);
  return g;
}

void spotpessoa_teste(char *(*baixar)(const char *url)) { baixarTeste = baixar; }
int  spotpessoa_disparos(void) { return disparos; }
