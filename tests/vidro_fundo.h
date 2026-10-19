// ARTE DE VERDADE ATRAS DO VIDRO, para as capturas do teste de opacidade e do
// vidro fosco (NUVIO_SHOT_VIDRO_OPAC / NUVIO_SHOT_VIDRO_FOSCO). Sem arte o
// vidro nao tem o que deixar passar e a comparacao nao diz nada. Registra a
// paleta da arte (o que o decode faz no app) e pinta o fundo pelo mesmo
// fundo_desenhar_modo do app, nitido; com o fosco ligado, o fundo assa a luz
// borrada que o vidro desenha dentro do painel.
#ifndef NV_TESTE_VIDRO_FUNDO_H
#define NV_TESTE_VIDRO_FUNDO_H
#include "corviva.h"
#include "fundo.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdlib.h>

#define VIDRO_ARTE "deploy/app/art/05.jpg"
static int vidroFundoAtivo(void) {
  return getenv("NUVIO_SHOT_VIDRO_OPAC") || getenv("NUVIO_SHOT_VIDRO_FOSCO");
}
static void vidroFundoPreparar(void) {
  SDL_Surface *b, *t;
  CorvivaPaleta p;
  if (!vidroFundoAtivo()) return;
  IMG_Init(IMG_INIT_JPG);
  b = IMG_Load(VIDRO_ARTE);
  if (!b) return;
  t = SDL_ConvertSurfaceFormat(b, SDL_PIXELFORMAT_ABGR8888, 0);
  if (t && corviva_extrair(t->pixels, t->w, t->h, t->pitch, &p)) corviva_anotar(VIDRO_ARTE, &p);
  if (t) SDL_FreeSurface(t);
  SDL_FreeSurface(b);
}
static void vidroFundoDesenhar(void) {
  if (!vidroFundoAtivo()) return;
  fundo_desenhar_modo(FUNDO_ARTE, (GfxRect){ 0, 0, 1920, 1080 }, 0.0f, VIDRO_ARTE, 1.0f);
}
#endif
