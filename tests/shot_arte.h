// ARTE DE VERDADE ATRAS DAS CAPTURAS DO GLASS UI (tests/*_shot.c).
//
// O vidro e um miolo translucido: sobre o fundo liso de glClearColor ele sai
// igual ao solido, e a captura "prova" um painel que na TV, com a arte do
// titulo atras, fica outro. Por isso as capturas do visual "Glass UI — ilha"
// pintam uma arte de tela cheia antes da tela testada.
//
// A imagem: NUVIO_SHOT_ARTE (caminho de um JPG/PNG) ou, sem ela, a primeira
// arte embarcada (deploy/app/art/00.jpg, a mesma de tests/ilha_shot.c). A arte
// dos mockups (design/glass-ilha/arte.jpg) tem um borrao no canto de cima a
// esquerda — o logo apagado — que nas capturas parece defeito da ilha do
// relogio, bem em cima dela; use-a so pelo ambiente. Sem arquivo nenhum cai num
// fundo de faixas quentes com um bloco claro, o pior caso de contraste (o mesmo
// de tests/fontepref_shot.c). NUVIO_SHOT_ARTE=- desliga (fundo liso).
#ifndef NV_SHOT_ARTE_H
#define NV_SHOT_ARTE_H
#include "gfx.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdlib.h>
#include <string.h>

static GLuint shotArteTex;
static int    shotArteFeito;

static GLuint shot_arte_carregar(void) {
  const char *c = getenv("NUVIO_SHOT_ARTE");
  SDL_Surface *s, *t;
  if (shotArteFeito) return shotArteTex;
  shotArteFeito = 1;
  if (c && !strcmp(c, "-")) return 0;
  s = IMG_Load(c && *c ? c : "deploy/app/art/00.jpg");
  if (!s) return 0;
  t = SDL_ConvertSurfaceFormat(s, SDL_PIXELFORMAT_ABGR8888, 0);
  SDL_FreeSurface(s);
  if (!t) return 0;
  glGenTextures(1, &shotArteTex);
  glBindTexture(GL_TEXTURE_2D, shotArteTex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, t->pitch / 4);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t->w, t->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, t->pixels);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  SDL_FreeSurface(t);
  return shotArteTex;
}

// Pinta a arte (ou o fundo de reserva) na tela toda, com o veu leve dos
// mockups (preto a `veu`) por cima.
static void shot_arte_desenhar(float veu) {
  GLuint tex = shot_arte_carregar();
  const char *c = getenv("NUVIO_SHOT_ARTE");
  if (c && !strcmp(c, "-")) return;
  if (tex) {
    gfx_tex_aspect_atual = 16.0f / 9.0f;   // as artes do pacote sao 16:9
    gfx_rect((GfxRect){ 0, 0, 1920, 1080 }, tex, GFX_CARD, 0, 0, 0, 0, 0, 0, 0, 1);
  } else {
    gfx_cor((GfxRect){ 0, 0, 1920, 1080 }, 0, .26f, .17f, .12f, 1);
    gfx_cor((GfxRect){ 0, 0, 1920, 360 }, 0, .55f, .36f, .22f, 1);
    gfx_cor((GfxRect){ 900, 420, 900, 260 }, 0, .82f, .78f, .70f, 1);
  }
  if (veu > 0.0f) gfx_cor((GfxRect){ 0, 0, 1920, 1080 }, 0, 0, 0, 0, veu);
}

// Material da captura: NUVIO_SHOT_VIDRO=0 grava `vidroLocal 1` (vidro
// desligado = solido) no ajustes.txt; qualquer outro valor deixa o vidro.
static void shot_arte_material(FILE *ajustesTxt) {
  const char *v = getenv("NUVIO_SHOT_VIDRO");
  fprintf(ajustesTxt, "vidroLocal %d\n", v && *v == '0' ? 1 : 0);
}
#endif
