#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned realocacoes;
static int falhar;
static void *medirRealloc(void *p, size_t n) {
  realocacoes++;
  return falhar ? NULL : realloc(p, n);
}
#define realloc medirRealloc
#include "../src/rede.c"
#undef realloc

int main(void) {
  Balde b = {0};
  char bloco[2048];
  memset(bloco, 'x', sizeof bloco);
  for (int i = 0; i < 512; i++) assert(receber(bloco, 1, sizeof bloco, &b) == sizeof bloco);
  assert(b.n == 1024 * 1024 && b.p[b.n] == 0);
  assert(!memcmp(b.p, bloco, sizeof bloco));
  assert(baldeFinal(&b) == b.p && b.cap == b.n + 1);
  printf("rede buffer: %u realocacoes para 512 blocos\n", realocacoes);
  assert(realocacoes <= 12);
  free(b.p); memset(&b, 0, sizeof b);

  rede_teto = 7;
  assert(receber(bloco, 1, sizeof bloco, &b) == 7);
  assert(b.n == 7 && b.p[7] == 0 && redeLimitouLocal);
  assert(!receber(bloco, 1, 1, &b));
  free(b.p); memset(&b, 0, sizeof b); rede_teto = 0;

  falhar = 1;
  assert(!receber(bloco, 1, sizeof bloco, &b));
  assert(!b.p && !b.n);
  falhar = 0;
  assert(!receber(bloco, SIZE_MAX, 2, &b));
  assert(!b.p && !b.n);
  b.n = (size_t)LONG_MAX;
  assert(!receber(bloco, 1, 2, &b));
  memset(&b, 0, sizeof b);
  assert(receber(bloco, 1, sizeof bloco, &b) == sizeof bloco);
  char *anterior = b.p;
  falhar = 1;
  assert(!receber(bloco, 1, sizeof bloco, &b));
  assert(b.p == anterior && b.n == sizeof bloco && b.p[b.n] == 0);
  free(b.p);
  puts("rede buffer: limites e falha de alocacao ok");
  return 0;
}
