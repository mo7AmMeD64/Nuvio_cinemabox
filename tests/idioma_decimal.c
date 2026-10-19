// idioma_decimal_texto: virgula decimal nos idiomas que a usam, ponto em en/ja/zh.
#include "idiomacod.h"
#include <stdio.h>
#include <string.h>

static int falhas;
static void caso(int idioma, const char *entrada, const char *esperado) {
  char b[128];
  snprintf(b, sizeof b, "%s", entrada);
  idioma_decimal_texto(b, idioma);
  if (strcmp(b, esperado)) { printf("FALHOU: '%s' -> '%s' (esperado '%s')\n", entrada, b, esperado); falhas++; }
}

int main(void) {
  caso(IDIOMA_PT, "Melhor: T1E2  ·  8.5", "Melhor: T1E2  ·  8,5");
  caso(IDIOMA_PT, "24.1 mi", "24,1 mi");
  caso(IDIOMA_DE, "7.9 pro Zuschauer, 1.5 und 2.25", "7,9 pro Zuschauer, 1,5 und 2,25");
  caso(IDIOMA_PTPT, "R$ 1.25 bi", "R$ 1,25 bi");
  caso(IDIOMA_EN, "24.1 mi", "24.1 mi");
  caso(IDIOMA_JA, "8.5", "8.5");
  caso(IDIOMA_ZHCN, "8.5", "8.5");
  caso(IDIOMA_PT, "E1. Piloto", "E1. Piloto");        // ponto de frase nao e decimal
  caso(IDIOMA_PT, ".5 e 5.", ".5 e 5.");              // sem digito dos dois lados
  caso(IDIOMA_PT, "", "");
  idioma_decimal_texto(NULL, IDIOMA_PT);               // nao pode cair
  if (!falhas) printf("idioma_decimal: ok\n");
  return falhas != 0;
}
