// txt_largura (medida sem rasterizar) tem de bater com a largura da linha
// rasterizada, e txt_bloco/txt_linha_corta so podem rasterizar as linhas FINAIS
// (antes rasterizavam cada prefixo tentado). Fora da suite: requer GL.
#include "gfx.h"
#include "text.h"
#include <SDL2/SDL.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
  SDL_SetHint("SDL_MAC_BACKGROUND_APP", "1");
  assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) == 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  SDL_Window *win = SDL_CreateWindow("t", 0, 0, 64, 64, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
  assert(win);
  SDL_GLContext gl = SDL_GL_CreateContext(win); assert(gl);
  glViewport(0, 0, 1920, 1080); gfx_tamanho_alvo(1920, 1080); assert(gfx_iniciar());
  assert(txt_iniciar("deploy/app", 1));

  const char *amostras[] = {
    "A", "A vida de", "Uma familia comum descobre um segredo antigo",
    "Ação · Aventura · Ficção científica", "The quick brown fox jumps over",
    "Diretor: Christopher Nolan", "çãõéÉ àü — “aspas” …",
  };
  const TxtEstilo estilos[] = { TXT_DET_SIN, TXT_DET_META2, TXT_CAPTION, TXT_TITULO1, TXT_DET_BOTAO };
  int n = 0;
  for (unsigned e = 0; e < sizeof estilos / sizeof *estilos; e++)
    for (unsigned i = 0; i < sizeof amostras / sizeof *amostras; i++) {
      txt_novo_quadro();
      int m = txt_largura(estilos[e], amostras[i]);
      TxtLinha l = txt_linha(estilos[e], amostras[i], 255, 255, 255, 255);
      if (m != l.w) { printf("DIVERGE est=%d '%s': medida %d, linha %d\n", estilos[e], amostras[i], m, l.w); return 1; }
      n++;
    }
  printf("larguras iguais em %d combinacoes\n", n);

  const char *sin = "Quando uma familia comum descobre um segredo antigo escondido no porao da casa, "
    "todos precisam decidir entre a verdade e a seguranca, enquanto lá fora alguem observa cada "
    "movimento e espera o momento certo para agir contra eles e contra tudo o que construiram juntos.";
  txt_novo_quadro();
  int r0 = txt_rasterizadas;
  float h = txt_bloco(TXT_DET_SIN, sin, 255, 255, 255, 0, 0, 1040.0f, 40.0f, 1.0f, 5);
  int usadas = txt_rasterizadas - r0;
  int linhas = (int)(h / 40.0f + 0.5f);
  printf("sinopse: %d linhas, %d rasterizadas\n", linhas, usadas);
  assert(linhas >= 2 && usadas <= linhas);

  txt_novo_quadro();
  r0 = txt_rasterizadas;
  TxtLinha c = txt_linha_corta(TXT_DET_SIN, sin, 255, 255, 255, 255, 600.0f);
  printf("corta: w=%d (<=600), %d rasterizadas\n", c.w, txt_rasterizadas - r0);
  assert(c.w > 0 && c.w <= 600 && txt_rasterizadas - r0 <= 2);
  puts("ok");
  return 0;
}
