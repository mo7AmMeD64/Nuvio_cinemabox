// tex_piramide_cpu: a piramide de mipmaps montada na CPU (#201, Android).
//
//   bash tests/mipcpu.sh        (com ASan)
//
// Confere em tamanhos NPOT e de borda (185x278 do w185 do TMDB, 342x513,
// 1x1, 1xN, Nx1): cada nivel tem o tamanho que o GL exige — max(1,
// floor(n/2)) ate 1x1, senao a textura fica INCOMPLETA no GLES2 —, a cor media
// de cada nivel fica perto da do nivel 0, cor solida continua exatamente a
// mesma, e nada le ou escreve fora da superficie (ASan).
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tex_cache.h"

static int falhas = 0;
#define CONFERE(c, ...) do { if (!(c)) { falhas++; printf("FALHA: " __VA_ARGS__); printf("\n"); } } while (0)

static SDL_Surface *nova(int w, int h, int solida) {
  SDL_Surface *s = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_ABGR8888);
  for (int y = 0; y < h; y++)
    for (int x = 0; x < w; x++) {
      unsigned char *q = (unsigned char *)s->pixels + (size_t)y * s->pitch + (size_t)x * 4;
      if (solida) { q[0] = 200; q[1] = 40; q[2] = 90; q[3] = 255; }
      else {
        q[0] = (unsigned char)(x * 255 / (w > 1 ? w - 1 : 1));
        q[1] = (unsigned char)(y * 255 / (h > 1 ? h - 1 : 1));
        q[2] = (unsigned char)(((x ^ y) & 1) ? 255 : 0);
        q[3] = 255;
      }
    }
  return s;
}

static void media(SDL_Surface *s, double m[4]) {
  double a[4] = {0, 0, 0, 0};
  for (int y = 0; y < s->h; y++)
    for (int x = 0; x < s->w; x++) {
      const unsigned char *q = (const unsigned char *)s->pixels + (size_t)y * s->pitch + (size_t)x * 4;
      for (int k = 0; k < 4; k++) a[k] += q[k];
    }
  for (int k = 0; k < 4; k++) m[k] = a[k] / ((double)s->w * s->h);
}

static void caso(int w, int h, int solida) {
  SDL_Surface *b = nova(w, h, solida);
  int esperado = 0, cw = w, ch = h;
  while (cw > 1 || ch > 1) { cw = cw > 1 ? cw / 2 : 1; ch = ch > 1 ? ch / 2 : 1; esperado++; }
  int n = tex_piramide_cpu(b);
  CONFERE(n == esperado, "%dx%d: %d niveis, esperado %d", w, h, n, esperado);
  double m0[4]; media(b, m0);
  int lw = w, lh = h, nivel = 0;
  for (SDL_Surface *s = (SDL_Surface *)b->userdata; s; s = (SDL_Surface *)s->userdata) {
    lw = lw > 1 ? lw / 2 : 1; lh = lh > 1 ? lh / 2 : 1; nivel++;
    CONFERE(s->w == lw && s->h == lh, "%dx%d nivel %d: %dx%d, esperado %dx%d",
            w, h, nivel, s->w, s->h, lw, lh);
    CONFERE(s->pitch == s->w * 4, "%dx%d nivel %d: pitch %d", w, h, nivel, s->pitch);
    double m[4]; media(s, m);
    // O canal xadrez (2) so tem media estavel enquanto o nivel ainda junta
    // pares; nos ultimos niveis de imagem impar a celula pega um pixel a mais
    // de um lado. Folga larga nele, apertada nos gradientes e no alfa.
    for (int k = 0; k < 4; k++) {
      double tol = solida ? 0.01 : (k == 2 ? 40.0 : 12.0);
      double d = m[k] - m0[k]; if (d < 0) d = -d;
      CONFERE(d <= tol, "%dx%d nivel %d canal %d: media %.1f, base %.1f", w, h, nivel, k, m[k], m0[k]);
    }
  }
  CONFERE(lw == 1 && lh == 1, "%dx%d: piramide parou em %dx%d", w, h, lw, lh);
  tex_piramide_liberar(b);
}

int main(void) {
  caso(185, 278, 0);
  caso(342, 513, 0);
  caso(342, 513, 1);
  caso(185, 278, 1);
  caso(1, 1, 0);
  caso(1, 9, 0);
  caso(7, 1, 0);
  caso(2, 3, 0);
  caso(256, 256, 0);
  caso(1000, 1500, 0);
  caso(781, 3, 1);
  // Formato que o envio nao sobe como esta: nao monta nada.
  { SDL_Surface *s = SDL_CreateRGBSurfaceWithFormat(0, 8, 8, 32, SDL_PIXELFORMAT_ARGB8888);
    CONFERE(tex_piramide_cpu(s) == 0 && !s->userdata, "ARGB8888 nao deveria ganhar piramide");
    SDL_FreeSurface(s); }
  // Chamada duas vezes: a segunda nao refaz nem vaza.
  { SDL_Surface *s = nova(33, 17, 0);
    int a = tex_piramide_cpu(s), b = tex_piramide_cpu(s);
    CONFERE(a == 5 && b == 0, "segunda chamada: %d e %d", a, b);
    tex_piramide_liberar(s); }
  if (falhas) { printf("mipcpu: %d falha(s)\n", falhas); return 1; }
  printf("mipcpu: ok\n");
  return 0;
}
