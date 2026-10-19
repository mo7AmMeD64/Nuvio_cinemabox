#include "player.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>

int main(void) {
  assert(player_regra_retomada_inicial(612.345, 1800, 34, 90) == 612.345);
  assert(player_regra_retomada_inicial(1224, 3600, 34, 90) == 1224);
  assert(!player_regra_retomada_inicial(0, 1800, 34, 90));
  assert(!player_regra_retomada_inicial(612, 0, 34, 90));
  assert(!player_regra_retomada_inicial(612, 1, 34, 90));
  assert(!player_regra_retomada_inicial(612, 1800, 50, 90));
  assert(!player_regra_retomada_inicial(1620, 1800, 90, 90));
  assert(!player_regra_retomada_inicial(1800, 1800, 89, 90));
  assert(!player_regra_retomada_inicial(1801, 1800, 89, 90));
  assert(!player_regra_retomada_inicial(-1, 1800, 34, 90));
  assert(!player_regra_retomada_inicial(NAN, 1800, 34, 90));
  assert(!player_regra_retomada_inicial(612, INFINITY, 34, 90));
  assert(!player_regra_retomada_inicial(INFINITY, 1800, 34, 90));
  assert(!player_regra_retomada_inicial(2147484, 5000000, 43, 90));
  puts("retomada regra: PASS (posicao exata, duracao, percentual e limites)");
}
