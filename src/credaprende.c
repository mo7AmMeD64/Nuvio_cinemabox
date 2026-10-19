#include "credaprende.h"
#include "credfonte.h"
#include "dados.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARQ "credits-offsets.txt"
#define MAXN 64
typedef struct { char imdb[24]; double resto; } Linha;
static Linha L[MAXN];
static int N, lido;

static void ler(void) {
  char *t, *p;
  if (lido) return;
  lido = 1;
  t = dados_ler(ARQ);
  if (!t) return;
  for (p = strtok(t, "\n"); p && N < MAXN; p = strtok(NULL, "\n")) {
    char id[24]; double r;
    if (sscanf(p, "%23s %lf", id, &r) == 2 && r >= 15.0 && r <= 900.0) {
      snprintf(L[N].imdb, sizeof L[N].imdb, "%s", id); L[N].resto = r; N++;
    }
  }
  free(t);
}

static void gravar(void) {
  char buf[MAXN * 40]; size_t k = 0; int i;
  buf[0] = 0;
  for (i = 0; i < N && k + 40 < sizeof buf; i++)
    k += (size_t)snprintf(buf + k, sizeof buf - k, "%s %.0f\n", L[i].imdb, L[i].resto);
  dados_gravar_leve(ARQ, buf);
}

static int achar(const char *imdb) {
  int i, n = (int)strcspn(imdb, ":");
  for (i = 0; i < N; i++) if ((int)strlen(L[i].imdb) == n && !strncmp(L[i].imdb, imdb, (size_t)n)) return i;
  return -1;
}

double cred_aprendido_resto(const char *imdb) {
  int i;
  if (!imdb || !*imdb) return 0.0;
  ler(); i = achar(imdb);
  return i >= 0 ? L[i].resto : 0.0;
}

void cred_aprender(const char *imdb, double durSeg, double posSeg, int fonteAtual) {
  double resto = durSeg - posSeg;
  int i;
  // Fora da faixa que um credito ocupa: fim de arquivo ou apertada no meio.
  if (!imdb || !*imdb || durSeg < 600.0 || resto < 15.0 || resto > 420.0 || resto > durSeg * 0.2) return;
  ler(); i = achar(imdb);
  if (i >= 0) {
    if (fonteAtual == CRED_FONTE_APRENDIDO && resto <= L[i].resto) return;
    if (resto == L[i].resto) return;
  } else {
    if (N >= MAXN) { memmove(L, L + 1, sizeof L - sizeof L[0]); N--; }
    i = N++;
    snprintf(L[i].imdb, sizeof L[i].imdb, "%.*s", (int)strcspn(imdb, ":") < 23 ? (int)strcspn(imdb, ":") : 23, imdb);
  }
  L[i].resto = resto;
  gravar();
  printf("[credits] learned offset for %s: %.0fs before the end\n", L[i].imdb, resto);
  fflush(stdout);
}
