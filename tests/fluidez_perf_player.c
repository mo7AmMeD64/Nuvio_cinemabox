// CUSTO DE DESENHO DO PLAYER, por estado, no Mac. Complemento de
// tests/fluidez_perf.c, que nao cobre o player. Roda a montagem de
// tests/player_glass_shot.c (os mesmos estados das capturas) e imprime, a cada
// quadro, os contadores de desenho do quadro anterior; o .sh resume os ultimos
// quadros de cada estado.
//
// So a 1.8.0 tem esses ganchos (-DNV_SHOT_HOOKS, video_simular): nao serve para
// comparar com a 1.7.x. O numero que vale e o preenchimento e a contagem, que
// sao deterministicos; o tempo de GPU do Mac nao e o da Mali.
#include "gfx.h"
#include "tex_cache.h"
#include "text.h"
#include <stdio.h>
static void perf_quadro(void) {
  printf("Q fill=%.3f vis=%.3f rects=%d progs=%d binds=%d cheios=%d mist=%d\n",
         gfx_fill, gfx_fill_vis, gfx_n_rect, gfx_n_prog, gfx_n_bind, gfx_n_cheio,
         gfx_n_cheio_mistura);
  gfx_novo_quadro();
}
// gfx_novo_quadro e o que main.c faz uma vez por quadro; o shot nao chama.
#define tex_novo_quadro() (perf_quadro(), tex_novo_quadro())
#include "player_glass_shot.c"
