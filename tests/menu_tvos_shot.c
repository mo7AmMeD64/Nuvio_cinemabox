// PNGs of the Apple TV (Dinamica) sidebar at UI scale 80/90/100, Streaming
// collapsed, D-pad expanded, and after an upward mobile swipe.
//   bash tests/menu_tvos_shot.sh /Volumes/ExternalSSD/tmp/rev/menutvos/<tag>
#define home_streaming_barra stub_barra
#define col_folder stub_folder
#include "../src/menu.c"
#include "dados.h"
#include "tex_cache.h"
#include "gl_compat.h"
#include <SDL2/SDL_image.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define W 1920
#define H 1080
int stub_barra(const int **p) { static const int v[4] = {0, 1, 2, 3}; if (p) *p = v; return 4; }
const ColFolder *stub_folder(int i) {
  static ColFolder f[4];
  static const char *t[4] = {"Netflix", "Prime Video", "Disney+", "Max"};
  snprintf(f[i & 3].title, sizeof f[i & 3].title, "%s", t[i & 3]);
  return &f[i & 3];
}

static void quadro(Uint32 now) {
  txt_novo_quadro(); tex_novo_quadro(); gfx_novo_quadro();
  glClearColor(0.22f, 0.28f, 0.38f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  menu_atualizar(1.0f / 60, now);
  menu_desenhar(now);
}

int main(int argc, char **argv) {
  const char *out = argc > 1 ? argv[1] : "/tmp/menutvos";
  const int pct[3] = {80, 90, 100};
  char path[1024];
  int i, k, j;
  SDL_SetHint("SDL_MAC_BACKGROUND_APP", "1");
  assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) == 0);
  IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  SDL_Window *win = SDL_CreateWindow("m", 0, 0, W, H, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
  assert(win && SDL_GL_CreateContext(win));
  glViewport(0, 0, W, H); gfx_tamanho_alvo(W, H);
  assert(gfx_iniciar() && txt_iniciar("deploy/app", 1));
  dados_iniciar("deploy/app/art");
  snprintf(path, sizeof path, "%s/ajustes.txt", getenv("NUVIO_DADOS"));
  FILE *f = fopen(path, "w"); assert(f);
  fputs("homeLayoutLocal 2\nvidroLocal 0\nidioma 0\n", f);
  fclose(f); ajustes_dir(getenv("NUVIO_DADOS"));
  tex_iniciar(64); gfx_icones_dir("deploy/app/art");
  assert(ajustes_home_layout() == HOME_LAYOUT_DINAMICA);
  menu_iniciar();
  for (i = 0; i < 3; i++) for (k = 0; k < 3; k++) {
    layout_modo_definir(k == 2);
    gfx_escala_ui_definir(pct[i] / 100.0f);
    menu_fechar(); menu_abrir();
    linha = k == 1 ? MENU_ST0 : MENU_AJUSTES;
    for (j = 0; j < 90; j++) quadro(1000 + 16 * j);
    if (k == 2) {
      PonteiroRolagem e = {PONT_ROL_INICIO, 1, 0, 0,
                          tvToqueRegiao.x + 10, tvToqueRegiao.y + 10};
      assert(tvToqueRolar(&e));
      e.fase = PONT_ROL_MOVER; e.delta = -1000;
      tvToqueRolar(&e);
      for (j = 0; j < 90; j++) quadro(2500 + 16 * j);
    }
    unsigned char *px = malloc((size_t)W * H * 4);
    SDL_Surface *s = SDL_CreateRGBSurfaceWithFormat(0, W, H, 32, SDL_PIXELFORMAT_RGBA32);
    assert(px && s);
    glReadPixels(0, 0, W, H, GL_RGBA, GL_UNSIGNED_BYTE, px);
    for (j = 0; j < H; j++) memcpy((char *)s->pixels + j * s->pitch, px + (size_t)(H - 1 - j) * W * 4, W * 4);
    snprintf(path, sizeof path, "%s/menutvos-%d-%s.png", out, pct[i], k == 2 ? "touch-swipe" : k ? "expanded" : "collapsed");
    assert(IMG_SavePNG(s, path) == 0);
    puts(path);
    SDL_FreeSurface(s); free(px);
  }
  SDL_Quit();
  return 0;
}
