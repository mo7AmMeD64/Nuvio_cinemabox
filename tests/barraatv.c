// Deterministic top-bar clock and 1080p proof; no network or device required.
#include "gfx.h"
#include "text.h"
#include <assert.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int hints, labels, measures;
static float labelA[16], labelY[16];
static void probeText(TxtLinha l, float x, float y, float r, float g, float b, float a) {
  assert(labels < 16); labelA[labels] = a; labelY[labels++] = y;
  txt_desenhar_tom(l, x, y, r, g, b, a);
}
static int probeWidth(TxtEstilo e, const char *s) { measures++; return txt_largura(e, s); }
static void probeIcon(GfxRect r, const char *name, float cr, float cg, float cb, float a) {
  if (!strcmp(name, "aj_chevrons-down") && a > .001f) hints++;
  gfx_icone(r, name, cr, cg, cb, a);
}
#define gfx_icone probeIcon
#define txt_desenhar_tom probeText
#define txt_largura probeWidth
#include "../src/menu.c"
#undef gfx_icone
#undef txt_desenhar_tom
#undef txt_largura
#include "dados.h"
#include "tex_cache.h"

static void config(int layout, int reduced) {
  char path[1024];
  snprintf(path, sizeof path, "%s/ajustes.txt", getenv("NUVIO_DADOS"));
  FILE *f = fopen(path, "w"); assert(f);
  fprintf(f, "homeLayoutLocal %d\nmenuTopoLocal 1\nidioma 1\nanimacoes %d\nvidroLocal 0\n", layout, reduced);
  fclose(f); ajustes_dir(getenv("NUVIO_DADOS"));
}
static void draw(const char *out, const char *name) {
  char path[1024];
  txt_novo_quadro(); tex_novo_quadro(); tex_bombear(6); gfx_novo_quadro();
  glClearColor(.025f, .030f, .045f, 1); glClear(GL_COLOR_BUFFER_BIT);
  hints = labels = 0; menu_desenhar(0);
  assert(hints == (ajustes_home_layout() == HOME_LAYOUT_DINAMICA && topoEntrada(8) > .001f));
  if (!name) return;
  unsigned char *px = malloc(1920 * 1080 * 4);
  SDL_Surface *s = SDL_CreateRGBSurfaceWithFormat(0, 1920, 1080, 32, SDL_PIXELFORMAT_RGBA32);
  assert(px && s);
  glReadPixels(0, 0, 1920, 1080, GL_RGBA, GL_UNSIGNED_BYTE, px);
  for (int y = 0; y < 1080; y++) memcpy((char *)s->pixels + y * s->pitch, px + (1079-y) * 1920 * 4, 1920 * 4);
  snprintf(path, sizeof path, "%s/%s.png", out, name);
  assert(IMG_SavePNG(s, path) == 0); puts(path);
  free(px); SDL_FreeSurface(s);
}
int main(int argc, char **argv) {
  assert(argc == 2);
  SDL_SetHint("SDL_MAC_BACKGROUND_APP", "1");
  assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) == 0);
  assert(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG);
  SDL_Window *win = SDL_CreateWindow("barraatv", 0, 0, 1920, 1080, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
  assert(win && SDL_GL_CreateContext(win));
  glViewport(0, 0, 1920, 1080); gfx_tamanho_alvo(1920, 1080);
  assert(gfx_iniciar() && txt_iniciar("deploy/app", 1));
  char art[4096]; assert(realpath("deploy/app/art", art));
  dados_iniciar(art); tex_iniciar(64); gfx_icones_dir(art);
  config(2, 0); assert(ajustes_menu_topo());
  menu_iniciar(); menu_pilula_mostrar(1); menu_atualizar(1, 0);
  // Warm cached glyphs/icons before the deterministic sequence.
  for (int n = 0; n < 30; n++) {
    draw(argv[1], NULL);
    gfx_icone((GfxRect){0, 0, 16, 16}, "aj_chevrons-down", 1, 1, 1, 1);
    gfx_icone((GfxRect){0, 0, 26, 26}, "mt_busca", 1, 1, 1, 1);
    SDL_Delay(4);
  }
  char icon[4200]; snprintf(icon, sizeof icon, "%s/icones/aj_chevrons-down.png", art);
  assert(tex_obter_larg(icon, 16));
  draw(argv[1], "t000"); menu_abrir();
  float prev[9] = {0}, lastWidth = 0, prevY[16], prevA[16] = {0};
  for (int n = 0; n < 16; n++) prevY[n] = 10000;
  measures = 0;
  for (int ms = 8; ms <= 320; ms += 8) {
    menu_atualizar(.008f, ms);
    MenuGeo g = topoForma();
    assert(g.w >= lastWidth); lastWidth = g.w;
    for (int k = 0; k < 9; k++) {
      float a = topoEntrada(k), dy = 8 * (1-a);
      assert(a >= prev[k] && a <= 1 && dy <= 8 * (1-prev[k]));
      if (k) assert(a <= topoEntrada(k-1));
      prev[k] = a;
    }
    if (ms == 80) assert(prev[0] > 0 && prev[8] == 0);
    draw(argv[1], ms == 80 ? "t080" : ms == 160 ? "t160" : NULL);
    for (int k = 0; k < labels; k++) {
      assert(labelA[k] + .00001f >= prevA[k] && labelY[k] <= prevY[k] + .00001f);
      prevA[k] = labelA[k]; prevY[k] = labelY[k];
    }
    assert(measures == 0);
  }
  assert(topoEntrada(8) == 1); draw(argv[1], "t320");
  menu_atualizar(.5f, 820); draw(argv[1], "open");
  for (int k = 0; k < labels; k++) { prevA[k] = labelA[k]; prevY[k] = labelY[k]; }
  menu_fechar();
  for (int ms = 8; ms <= 200; ms += 8) {
    menu_atualizar(.008f, 820 + ms);
    for (int k = 0; k < 9; k++) { float a = topoEntrada(k); assert(a <= prev[k]); prev[k] = a; }
    draw(argv[1], ms == 96 ? "closing-mid" : NULL);
    for (int k = 0; k < labels; k++) {
      assert(labelA[k] <= prevA[k] + .00001f && labelY[k] + .00001f >= prevY[k]);
      prevA[k] = labelA[k]; prevY[k] = labelY[k];
    }
  }
  assert(topoEntrada(0) == 0);
  config(2, 1); menu_abrir(); menu_atualizar(.001f, 0);
  assert(topoEntrada(8) == 1);
  menu_fechar(); menu_atualizar(.001f, 0); assert(topoEntrada(0) == 0);
  config(2, 0); anim_politica_reduzida = 1;
  menu_abrir(); menu_atualizar(.001f, 0); assert(topoEntrada(8) == 1);
  menu_fechar(); menu_atualizar(.001f, 0); assert(topoEntrada(0) == 0);
  anim_politica_reduzida = 0;
  for (int layout = 0; layout < 2; layout++) {
    config(layout, 0); menu_abrir(); menu_atualizar(1, 0); draw(argv[1], NULL);
    menu_fechar(); menu_atualizar(1, 0);
  }
  puts("barraatv: clock, stagger, drawing scope and reduced motion OK");
  SDL_Quit(); return 0;
}
