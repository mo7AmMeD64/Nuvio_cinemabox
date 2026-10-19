// Capturas e regressões do onboarding da 1.5.1. Janela GL oculta, FBO 1080p.
#include "novidades151.h"
#include "dados.h"
#include "ajustes.h"
#include "gfx.h"
#include "text.h"
#include "tex_cache.h"
#include "player.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static GLuint fbo, fboTex;
static char outdir[512];

static void ajustesDeTeste(int ingles, int reduzidas) {
  char caminho[700]; FILE *f;
  snprintf(caminho, sizeof caminho, "%s/ajustes.txt", dados_dir());
  f = fopen(caminho, "w"); assert(f);
  fprintf(f, "idioma %d\nanimacoes %d\n", ingles, reduzidas);
  fclose(f);
  ajustes_dir(dados_dir());
  ajustes_iniciar();
  assert(ajustes_animacoes_reduzidas() == !!reduzidas);
}

static void tecla(SDL_Keycode k) {
  SDL_Event e; memset(&e, 0, sizeof e);
  e.type = SDL_KEYDOWN; e.key.keysym.sym = k;
  novidades151_evento(&e);
}

static void quadro(float dt) {
  SDL_PumpEvents(); txt_novo_quadro(); tex_novo_quadro(); tex_bombear(8); gfx_novo_quadro();
  novidades151_atualizar(dt, SDL_GetTicks());
  glBindFramebuffer(GL_FRAMEBUFFER, fbo); glViewport(0, 0, 1920, 1080);
  glClearColor(0.051f, 0.051f, 0.051f, 1.0f); glClear(GL_COLOR_BUFFER_BIT);
  novidades151_desenhar(SDL_GetTicks()); glFinish();
}

static unsigned long long gravarPNG(const char *nome) {
  unsigned char *pix = malloc(1920u * 1080u * 4u);
  SDL_Surface *s; unsigned long long h = 1469598103934665603ULL;
  assert(pix); glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadPixels(0, 0, 1920, 1080, GL_RGBA, GL_UNSIGNED_BYTE, pix);
  s = SDL_CreateRGBSurfaceWithFormat(0, 1920, 1080, 32, SDL_PIXELFORMAT_RGBA32);
  assert(s);
  for (int y = 0; y < 1080; y++) {
    unsigned char *row = pix + (1079 - y) * 1920u * 4u;
    memcpy((char *)s->pixels + y * s->pitch, row, 1920u * 4u);
    for (size_t i = 0; i < 1920u * 4u; i++) { h ^= row[i]; h *= 1099511628211ULL; }
  }
  assert(IMG_SavePNG(s, nome) == 0);
  SDL_FreeSurface(s); free(pix);
  printf("captura: %s\n", nome); return h;
}

static void caminho(char *dst, size_t n, const char *leaf) {
  snprintf(dst, n, "%s/%s", outdir, leaf);
}

static void estabilizar(float s) {
  int n = (int)(s * 60.0f + 0.5f);
  for (int i = 0; i < n; i++) quadro(1.0f / 60.0f);
}

static unsigned long long capturarCena(const char *nome) {
  char p[700];
  for (int i = 0; i < 36; i++) quadro(0.0f); // fontes e texturas assentam
  caminho(p, sizeof p, nome);
  return gravarPNG(p);
}

static void esperarCena(unsigned ms) {
  for (unsigned i = 0; i < ms; i += 8) { quadro(0.008f); SDL_Delay(1); }
}

static void regras(void) {
  // Primeira abertura, Back e marca persistida; a chamada repetida não reabre.
  dados_apagar("novidades-151-ui.txt");
  novidades151_primeira_vez(); assert(novidades151_aberto());
  tecla(SDLK_ESCAPE); assert(!novidades151_aberto());
  assert(novidades151_pedido() == N151_PEDIU_NADA);
  assert(novidades151_pedido() == N151_PEDIU_NADA);
  char *marca = dados_ler("novidades-152-ui.txt");
  assert(marca != NULL);
  free(marca);
  novidades151_primeira_vez(); assert(!novidades151_aberto());

  // Setas mudam a cena sem fechar; hash do quadro muda após a transição.
  novidades151_abrir(); assert(novidades151_aberto());
  unsigned long long c0 = capturarCena("assert-cena-0.png");
  tecla(SDLK_RIGHT); assert(novidades151_aberto()); esperarCena(500);
  unsigned long long c1 = capturarCena("assert-cena-1.png");
  tecla(SDLK_RIGHT); assert(novidades151_aberto()); esperarCena(500);
  unsigned long long c2 = capturarCena("assert-cena-2.png");
  assert(c0 != c1 && c1 != c2 && c0 != c2);
  // As tres cenas da 1.5.2 (barra lateral, trailer, correcoes) tambem sao
  // distintas entre si e das anteriores, e a sexta volta para a primeira.
  { unsigned long long h[3]; int i, j;
    for (i = 0; i < 3; i++) {
      char nome[40];
      tecla(SDLK_RIGHT); assert(novidades151_aberto()); esperarCena(500);
      snprintf(nome, sizeof nome, "assert-cena-%d.png", 3 + i);
      h[i] = capturarCena(nome);
      assert(h[i] != c0 && h[i] != c1 && h[i] != c2);
      for (j = 0; j < i; j++) assert(h[i] != h[j]);
    }
    // A cena 0 anima (o foco passeia entre as fontes), entao o quadro de volta
    // nao repete o hash do primeiro: basta ter saido da sexta.
    tecla(SDLK_RIGHT); esperarCena(500);
    assert(capturarCena("assert-cena-volta.png") != h[2]); }

  // CTA Explorar fecha e seu pedido é consumido exatamente uma vez.
  tecla(SDLK_RETURN); assert(!novidades151_aberto());
  assert(novidades151_pedido() == N151_PEDIU_AJUSTES);
  assert(novidades151_pedido() == N151_PEDIU_NADA);

  // Cima/baixo escolhe Agora não; Enter e Back não pedem Ajustes.
  novidades151_abrir(); tecla(SDLK_UP); tecla(SDLK_RETURN);
  assert(!novidades151_aberto());
  assert(novidades151_pedido() == N151_PEDIU_NADA);
  novidades151_abrir(); tecla(SDLK_ESCAPE);
  assert(!novidades151_aberto() && novidades151_pedido() == N151_PEDIU_NADA);
  puts("PASS: primeira vez, Back, cenas e CTAs da 1.5.2");
}

static void capturasIdioma(int ingles, int reduzidas, const char *tag) {
  char nome[128];
  ajustesDeTeste(ingles, reduzidas);
  novidades151_abrir();
  for (int cena = 0; cena < 6; cena++) {
    if (cena) { tecla(SDLK_RIGHT); esperarCena(500); }
    snprintf(nome, sizeof nome, "cena%d-%s.png", cena, tag);
    capturarCena(nome);
  }
}

static void sequencia(void) {
  char nome[700];
  ajustesDeTeste(0, 0);
  novidades151_abrir();
  for (int frame = 0; frame < 240; frame++) {
    quadro(1.0f / 12.0f);
    snprintf(nome, sizeof nome, "%s/frame-%04d.png", outdir, frame);
    gravarPNG(nome);
  }
  puts("sequência: 240 PNG, 12 fps simulados, 20 s");
}

int main(int argc, char **argv) {
  const char *dir = getenv("NUVIO_DADOS");
  SDL_Window *w; SDL_GLContext gl;
  TxtFamilia interfaceAntes; int familiaPlayerAntes;
  if (!dir || !dir[0] || argc < 2) return 2;
  snprintf(outdir, sizeof outdir, "%s", argv[1]);
  dados_iniciar(dir); if (strcmp(dados_dir(), dir)) return 2;
  ajustes_dir(dados_dir());
  SDL_SetHint("SDL_MAC_BACKGROUND_APP", "1");
  assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) == 0);
  IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  w = SDL_CreateWindow("novidades151-shot", 0, 0, 64, 64,
                       SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN); assert(w);
  gl = SDL_GL_CreateContext(w); assert(gl);
  glGenTextures(1, &fboTex); glBindTexture(GL_TEXTURE_2D, fboTex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1920, 1080, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, NULL);
  glGenFramebuffers(1, &fbo); glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTex, 0);
  assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
  glViewport(0, 0, 1920, 1080); gfx_tamanho_alvo(1920, 1080); assert(gfx_iniciar());
  assert(txt_iniciar("deploy/app", 1)); tex_iniciar(64);
  ajustes_iniciar();
  gfx_tex_esquecer(0); gfx_icones_dir("deploy/app/art");
  interfaceAntes = txt_fonte_interface();
  familiaPlayerAntes = player_leg_estilo()->familia;

  regras();
  capturasIdioma(0, 0, "pt");
  capturasIdioma(1, 0, "en");
  capturasIdioma(0, 1, "reduced");

  // A demo pode mostrar amostras, mas nunca gravar fonte da UI/player.
  assert(txt_fonte_interface() == interfaceAntes);
  assert(player_leg_estilo()->familia == familiaPlayerAntes);

  // Em modo reduzido não há autoavanço. A mesma cena conserva os pixels depois
  // de uma passagem equivalente a mais que um ciclo completo.
  ajustesDeTeste(0, 1); novidades151_abrir();
  estabilizar(8.0f);
  unsigned long long fixa0 = capturarCena("reduced-static-a.png");
  estabilizar(8.0f);
  unsigned long long fixa1 = capturarCena("reduced-static-b.png");
  assert(fixa0 == fixa1);
  assert(txt_fonte_interface() == interfaceAntes);
  assert(player_leg_estilo()->familia == familiaPlayerAntes);

  sequencia();
  assert(txt_fonte_interface() == interfaceAntes);
  assert(player_leg_estilo()->familia == familiaPlayerAntes);
  tex_encerrar(); txt_encerrar(); gfx_encerrar();
  SDL_GL_DeleteContext(gl); SDL_DestroyWindow(w); IMG_Quit(); SDL_Quit();
  puts("PASS: capturas e regressões da 1.5.1");
  return 0;
}
