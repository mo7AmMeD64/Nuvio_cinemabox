// Dois cards terminados alternando nao podem furar o limite de 30 s por
// titulo (registro 25007: 47 pedidos em ~7 min).
#include "../src/descoberta.h"
static int pedidos;
#define desc_episodios(i, f) ((void)(i), (void)(f), pedidos++)
#define desc_episodios_carregando(i) ((void)(i), 0)
#include "../src/continuar.c"
#include <assert.h>

int main(void) {
  int k, i;
  char id[16];
  for (k = 0; k < 50; k++) {
    pedirEpisodios(1, "tt39304754");
    pedirEpisodios(2, "tt33044444");
  }
  assert(pedidos == 2);
  // Mais titulos que vagas: o mais antigo cede, sem travar ninguem.
  for (i = 0; i < CW_PEDIDOS + 3; i++) {
    snprintf(id, sizeof id, "tt%d", 100 + i);
    pedirEpisodios(3, id);
  }
  assert(pedidos == 2 + CW_PEDIDOS + 3);
  puts("cw_pedidos: OK");
  return 0;
}
