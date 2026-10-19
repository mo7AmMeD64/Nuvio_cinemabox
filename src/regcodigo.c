#include "regcodigo.h"
#include <stdlib.h>

// As constantes sao as de servidor/recomendacoes/src/codigo.js. A e impar, e
// por isso inversivel mod 2^30: dois ids nunca dao o mesmo codigo.
#define COD_A 0x2c5e1b3dULL
#define COD_B 0x15a4e3c7ULL
#define COD_M (1ULL << 30)

int regcodigo_de_id(const char *id, char *dst) {
  static const char ALFA[] = "0123456789ABCDEFGHJKMNPQRSTVWXYZ";
  unsigned long long n = 0, v;
  const char *p = id;
  int i;
  dst[0] = 0;
  if (!p || !*p) return 0;
  for (; *p; p++) {
    if (*p < '0' || *p > '9') return 0;
    n = n * 10 + (unsigned long long)(*p - '0');
    if (n >= COD_M) return 0;
  }
  if (!n) return 0;
  v = (n * COD_A + COD_B) % COD_M;
  for (i = 5; i >= 0; i--) { dst[i] = ALFA[v & 31]; v >>= 5; }
  dst[6] = 0;
  return 1;
}
