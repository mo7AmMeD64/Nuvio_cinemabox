// CUSTO DE DESENHO DO GUIA DE TV, por estado. Complemento de tests/fluidez_perf.c
// e de tests/fluidez_perf_player.c: o guia ganhou o Glass UI na 1.8.0 (chips,
// grade em ilha, celulas em alfa, vidro) e nenhuma medida o cobria.
//
// Roda a montagem de tests/guia_shot.c (que inclui src/guia.c e semeia canais e
// grade sem rede) e imprime, a cada quadro trocado, os contadores de desenho do
// quadro. O .sh agrupa pelos marcadores "captura:" que o guia_shot ja imprime e
// resume os ultimos quadros de cada estado.
//
// O numero que vale e o preenchimento e a contagem, que sao deterministicos; o
// tempo de GPU do Mac nao e o da Mali. Compila tambem numa arvore antiga (o
// contador de tela cheia com mistura so existe com -DNV_FLUIDEZ_PERF).
#include "gfx.h"
#include "tex_cache.h"
#include "text.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#ifdef NV_PERF_BASE
static int gfx_n_cheio_mistura = -1;
#endif
static void perf_quadro(void) {
  printf("Q fill=%.3f vis=%.3f rects=%d progs=%d binds=%d cheios=%d mist=%d\n",
         gfx_fill, gfx_fill_vis, gfx_n_rect, gfx_n_prog, gfx_n_bind, gfx_n_cheio,
         gfx_n_cheio_mistura);
}
// Cada quadro do guia_shot termina em SDL_GL_SwapWindow, depois de gfx_novo_quadro
// ter zerado os contadores no comeco dele: aqui eles somam o quadro inteiro.
#define SDL_GL_SwapWindow(w) (perf_quadro(), SDL_GL_SwapWindow(w))
#include "guia_shot.c"
