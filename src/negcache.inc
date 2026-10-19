// Incluido por rede.c (como rede_pedido.inc): quem linka rede.c ganha o cache
// sem listar mais um arquivo, e sem depender de dados.c.
#include "negcache.h"
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>

#define NEG_MAX 512
#define NEG_TTL_S (24L * 3600L)
#define HOST_MAX 16
#define RECUO_BASE_S 30L
#define RECUO_TETO_S 600L

typedef struct { uint64_t h; long expira; } Neg;
typedef struct { char host[48]; int falhas; long ate; } Recuo;

static char *(*discoLer)(const char *);
static int (*discoGravar)(const char *, const char *);
void negcache_disco(char *(*ler)(const char *), int (*gravar)(const char *, const char *)) {
  discoLer = ler; discoGravar = gravar;
}

static pthread_mutex_t travaNeg = PTHREAD_MUTEX_INITIALIZER;
static Neg neg[NEG_MAX];
static int nNeg, proxNeg;
static Recuo recuo[HOST_MAX];
static int carregado, sujo;
static long ultimaGravacao, ultimoAgora;

// host -> quais status viram "nao existe" (bit a bit abaixo).
static const struct { const char *host; int c404, c400; } POLITICA[] = {
  { "api.theintrodb.org",    1, 1 },
  { "api.tiffara.com",       1, 1 },
  { "v3-cinemeta.strem.io",  1, 1 },
  { "api.themoviedb.org",    1, 0 },
};

static int hostDe(const char *u, char *h, size_t n) {
  const char *p = strstr(u, "://"), *f;
  size_t k;
  if (!p) return 0;
  p += 3;
  f = p;
  while (*f && *f != '/' && *f != ':' && *f != '?' && *f != '#') f++;
  k = (size_t)(f - p);
  if (!k || k >= n) return 0;
  memcpy(h, p, k); h[k] = 0;
  for (k = 0; h[k]; k++) if (h[k] >= 'A' && h[k] <= 'Z') h[k] = (char)(h[k] + 32);
  return 1;
}

static int politica(const char *url, char *host, size_t n) {
  size_t i;
  if (!url || !hostDe(url, host, n)) return -1;
  for (i = 0; i < sizeof POLITICA / sizeof POLITICA[0]; i++)
    if (!strcmp(host, POLITICA[i].host)) return (int)i;
  return -1;
}

// FNV-1a 64 do endereco SEM os parametros de credencial.
static uint64_t chave(const char *url) {
  uint64_t h = 1469598103934665603ULL;
  const char *p = url;
  while (*p) {
    if ((p == url || p[-1] == '?' || p[-1] == '&') &&
        (!strncasecmp(p, "api_key=", 8) || !strncasecmp(p, "apikey=", 7) ||
         !strncasecmp(p, "token=", 6))) {
      while (*p && *p != '&') p++;
      if (*p == '&') p++;
      continue;
    }
    h ^= (unsigned char)*p++;
    h *= 1099511628211ULL;
  }
  return h ? h : 1;
}

static void carregarLocked(long agora);

static int achar(uint64_t h) {
  int i;
  for (i = 0; i < nNeg; i++) if (neg[i].h == h) return i;
  return -1;
}

int negcache_barra(const char *url, long agora) {
  char host[48];
  int pol, i, r = 0;
  uint64_t h;
  if ((pol = politica(url, host, sizeof host)) < 0) return 0;
  h = chave(url);
  pthread_mutex_lock(&travaNeg);
  carregarLocked(agora);
  i = achar(h);
  if (i >= 0 && neg[i].expira > agora && neg[i].expira - agora <= NEG_TTL_S) r = 1;
  if (!r)
    for (i = 0; i < HOST_MAX; i++)
      if (recuo[i].host[0] && !strcmp(recuo[i].host, host) && recuo[i].ate > agora &&
          recuo[i].ate - agora <= RECUO_TETO_S) { r = 1; break; }
  pthread_mutex_unlock(&travaNeg);
  return r;
}

static void gravarLocked(long agora) {
  char *b = malloc((size_t)NEG_MAX * 32 + 8);
  int n;
  if (!b) return;
  n = 0;
  {
    int i;
    for (i = 0; i < nNeg; i++)
      if (neg[i].expira > agora)
        n += snprintf(b + n, 32, "%016llx %ld\n", (unsigned long long)neg[i].h, neg[i].expira);
  }
  b[n] = 0;
  if (discoGravar) discoGravar("negcache.txt", b);
  free(b);
  sujo = 0; ultimaGravacao = agora;
}

void negcache_nota(const char *url, int curl, int http, long agora) {
  char host[48];
  int pol, i, ruim, estavel = 0;
  if ((pol = politica(url, host, sizeof host)) < 0) return;
  ruim = curl != 0 || http == 0;
  pthread_mutex_lock(&travaNeg);
  ultimoAgora = agora;
  carregarLocked(agora);
  // Recuo do host: so transporte falho conta; qualquer resposta HTTP zera.
  for (i = 0; i < HOST_MAX; i++) if (recuo[i].host[0] && !strcmp(recuo[i].host, host)) break;
  if (i == HOST_MAX && ruim) {
    for (i = 0; i < HOST_MAX; i++) if (!recuo[i].host[0]) break;
    if (i == HOST_MAX) i = 0;
    snprintf(recuo[i].host, sizeof recuo[i].host, "%s", host);
    recuo[i].falhas = 0; recuo[i].ate = 0;
  }
  if (i < HOST_MAX) {
    if (ruim) {
      long s = RECUO_BASE_S << (recuo[i].falhas < 5 ? recuo[i].falhas : 5);
      if (s > RECUO_TETO_S) s = RECUO_TETO_S;
      recuo[i].falhas++;
      recuo[i].ate = agora + s;
    } else { recuo[i].falhas = 0; recuo[i].ate = 0; }
  }
  if (!ruim) {
    if (http == 404 || http == 410) estavel = POLITICA[pol].c404;
    else if (http == 400) estavel = POLITICA[pol].c400;
  }
  if (estavel) {
    uint64_t h = chave(url);
    int k = achar(h);
    if (k < 0) {
      if (nNeg < NEG_MAX) k = nNeg++;
      else { k = proxNeg; proxNeg = (proxNeg + 1) % NEG_MAX; }
      printf("[negcache] HTTP %d em %s: nao repete por 24 h\n", http, host);
      fflush(stdout);
    }
    neg[k].h = h; neg[k].expira = agora + NEG_TTL_S;
    sujo = 1;
    if (agora - ultimaGravacao >= 60) gravarLocked(agora);
  }
  pthread_mutex_unlock(&travaNeg);
}

static void carregarTextoLocked(const char *t, long agora) {
  const char *p = t;
  while (p && *p) {
    unsigned long long h; long e;
    if (sscanf(p, "%llx %ld", &h, &e) == 2 && h && e > agora && e - agora <= NEG_TTL_S &&
        nNeg < NEG_MAX && achar((uint64_t)h) < 0) {
      neg[nNeg].h = (uint64_t)h; neg[nNeg].expira = e; nNeg++;
    }
    p = strchr(p, '\n');
    if (p) p++;
  }
}

static void carregarLocked(long agora) {
  char *t;
  if (carregado) return;
  carregado = 1;
  t = discoLer ? discoLer("negcache.txt") : NULL;
  if (t) { carregarTextoLocked(t, agora); free(t); }
}

void negcache_carregar_texto(const char *texto, long agora) {
  pthread_mutex_lock(&travaNeg);
  carregado = 1;
  carregarTextoLocked(texto, agora);
  pthread_mutex_unlock(&travaNeg);
}

void negcache_gravar(void) {
  pthread_mutex_lock(&travaNeg);
  if (sujo) gravarLocked(ultimoAgora ? ultimoAgora : (long)time(NULL));
  pthread_mutex_unlock(&travaNeg);
}

void negcache_zerar(void) {
  pthread_mutex_lock(&travaNeg);
  nNeg = proxNeg = 0; sujo = 0; ultimaGravacao = 0; carregado = 0;
  memset(recuo, 0, sizeof recuo);
  pthread_mutex_unlock(&travaNeg);
}

int negcache_n(void) { return nNeg; }

int negcache_serializar(char *dst, int tam) {
  int i, n = 0;
  pthread_mutex_lock(&travaNeg);
  for (i = 0; i < nNeg && n + 32 < tam; i++)
    n += snprintf(dst + n, (size_t)(tam - n), "%016llx %ld\n", (unsigned long long)neg[i].h, neg[i].expira);
  pthread_mutex_unlock(&travaNeg);
  if (tam > 0) dst[n < tam ? n : tam - 1] = 0;
  return n;
}
