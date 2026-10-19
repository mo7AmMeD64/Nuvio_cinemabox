// #204: NO WEBOS, 484 LONGE DA SETA NAO APAGA A SETA DO SISTEMA.
//
//   bash tests/ponteiro_webos.sh
//
// O log da 50NANO75 (PowerVR BXE-4-32) na 1.6.5: depois de navegar de seta,
// cada vez que a mao pegava o controle o webOS mandava 484 (cursor mostrou) e
// o ponteiro.c respondia apagando a seta do sistema (485 ~40 ms depois). Com a
// seta apagada aquele webOS nao manda movimento: o limiar da seta nunca vence
// e o ponteiro nao volta. 12 vezes numa sessao, 3 movimentos, 0 cliques.
//
// Compila src/ponteiro.c com -DNV_PONT_WEBOS_TESTE (o ramo do webOS) e uma
// SDL_webOSCursorVisibility de mentira que guarda o ultimo pedido.
#include "ponteiro.h"
#include "gfx.h"
#include <stdio.h>
#include <string.h>

float gfx_opacidade_grupo = 1.0f;
void gfx_sem_recorte(void) {}
float gfx_escala(void) { return 1.0f; }   // layer scale (a8392eaa): identity here
void gfx_cor(GfxRect r, float raio, float cr, float cg, float cb, float ca) {
  (void)r; (void)raio; (void)cr; (void)cg; (void)cb; (void)ca;
}

static int falhas;
#define CONFERE(c, msg) do { if (!(c)) { printf("FALHA: %s (linha %d)\n", msg, __LINE__); falhas++; } } while (0)

static Uint32 relogio = 1000;
static Uint32 agora(void) { return relogio; }
static void entregar(const SDL_Event *e) { (void)e; }

// -1 = nunca pedido; 0/1 = o ultimo pedido ao compositor.
static int sistema = -1, nApagar;
static SDL_bool cursorSistema(SDL_bool v) { sistema = v ? 1 : 0; if (!v) nApagar++; return SDL_TRUE; }

static int focoA = -1, nFocar;
static void focar(int a, int b) { (void)b; focoA = a; nFocar++; }
static void home(void) {
  for (int c = 0; c < 3; c++) ponteiro_alvo(100 + c * 220.0f, 100, 200, 100, focar, NULL, c, 0);
}
static void quadro(void) { ponteiro_quadro(relogio); home(); ponteiro_desenhar(); relogio += 16; }
static void mover(int x, int y) {
  SDL_Event e; SDL_zero(e);
  e.type = SDL_MOUSEMOTION; e.motion.x = x; e.motion.y = y;
  ponteiro_evento(&e, entregar);
}
static int tecla(SDL_Keycode k, int sc) {
  SDL_Event e; SDL_zero(e);
  e.type = SDL_KEYDOWN; e.key.keysym.sym = k; e.key.keysym.scancode = (SDL_Scancode)sc;
  return ponteiro_evento(&e, entregar);
}

int main(void) {
  ponteiro_teste_relogio(agora);
  ponteiro_teste_janela(1920, 1080);
  ponteiro_teste_cursor_sistema(cursorSistema);

  mover(110, 110); quadro(); quadro();
  CONFERE(ponteiro_ativo(), "movimento mostra");

  // Seta: o nosso e o do sistema somem.
  CONFERE(tecla(SDLK_RIGHT, 0) == 0, "a seta segue para a tela");
  CONFERE(!ponteiro_ativo() && sistema == 0, "seta apaga a seta do sistema");

  // Tremor colado na seta: 484 em 100 ms -> apaga de novo (9cf3351, mantido).
  relogio += 100; nApagar = 0;
  CONFERE(tecla(0, 484) == 1, "484 consumido");
  CONFERE(!ponteiro_ativo() && sistema == 0 && nApagar == 1, "484 do tremor da seta reapaga");

  // A sequencia do log: segundos depois, 484 (mao no controle) e o webOS
  // poe o cursor no centro (MOUSEMOTION 960,540 rel=0,0).
  relogio += 16000; nApagar = 0;
  CONFERE(tecla(0, 484) == 1, "484 consumido");
  CONFERE(nApagar == 0, "484 longe da seta nao apaga a seta do sistema");
  CONFERE(sistema == 1, "e devolve a que nos apagamos");
  mover(960, 540);
  CONFERE(!ponteiro_ativo(), "o centro sozinho ainda nao reacende o hover");
  // Com a seta do sistema na tela, o movimento chega e vence o limiar.
  relogio += 16; mover(940, 560);
  relogio += 16; mover(900, 600);
  CONFERE(ponteiro_ativo(), "gesto de verdade reacende");
  quadro(); quadro();
  relogio += 200; nFocar = 0;
  mover(340, 150);
  CONFERE(nFocar == 1 && focoA == 1, "e o hover volta a focar");

  // Parado: dorme e apaga a do sistema; o 484 seguinte (sem seta) devolve.
  relogio += 5000; quadro();
  CONFERE(!ponteiro_ativo() && sistema == 0, "parado 5 s, dorme junto com a do sistema");
  CONFERE(tecla(0, 484) == 1 && ponteiro_ativo() && sistema == 1, "484 depois do sono acorda");

  if (falhas) { printf("%d falha(s)\n", falhas); return 1; }
  printf("ponteiro_webos: ok\n");
  return 0;
}
