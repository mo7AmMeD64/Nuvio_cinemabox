// Escala do retangulo de video: layout 1920x1080 -> superficie (#176).
#include <assert.h>
#include <stdio.h>
#include "../src/video_escala.h"

static void igual(NvRetInt a, int x, int y, int w, int h) {
  if (a.x != x || a.y != y || a.w != w || a.h != h)
    printf("esperava %d,%d %dx%d, veio %d,%d %dx%d\n", x, y, w, h, a.x, a.y, a.w, a.h);
  assert(a.x == x && a.y == y && a.w == w && a.h == h);
}

int main(void) {
  NvRetInt cheia = { 0, 0, 1920, 1080 };
  NvRetInt pip   = { 1400, 700, 480, 270 };
  // C9: drawable 1920x1080, escala 1, retangulo intocado.
  igual(nv_video_escalar(cheia, 1920, 1080, 1920, 1080), 0, 0, 1920, 1080);
  igual(nv_video_escalar(pip,   1920, 1080, 1920, 1080), 1400, 700, 480, 270);
  // G5 com drawable 3840x2160: tela cheia tem que cobrir o painel todo.
  igual(nv_video_escalar(cheia, 1920, 1080, 3840, 2160), 0, 0, 3840, 2160);
  igual(nv_video_escalar(pip,   1920, 1080, 3840, 2160), 2800, 1400, 960, 540);
  // Bordas arredondadas: sem fresta entre vizinhos e sem passar da tela.
  { NvRetInt a = { 0, 0, 641, 1080 }, b = { 641, 0, 1279, 1080 };
    NvRetInt ea = nv_video_escalar(a, 1920, 1080, 2560, 1440);
    NvRetInt eb = nv_video_escalar(b, 1920, 1080, 2560, 1440);
    assert(ea.x + ea.w == eb.x);
    assert(eb.x + eb.w == 2560); }
  // Superficie invalida: devolve como veio.
  igual(nv_video_escalar(pip, 1920, 1080, 0, 0), 1400, 700, 480, 270);
  igual(nv_video_escalar(pip, 1920, 1080, -1, 2160), 1400, 700, 480, 270);
  // Nunca some: retangulo minusculo continua com 1px.
  { NvRetInt t = { 10, 10, 1, 1 }; NvRetInt e = nv_video_escalar(t, 1920, 1080, 1280, 720);
    assert(e.w >= 1 && e.h >= 1); }
  printf("video_escala: ok\n");
  return 0;
}
