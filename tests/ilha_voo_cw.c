// O VOO DO PLAYER ATE A ILHA COM A FILEIRA "CONTINUAR ASSISTINDO" SENDO
// REFEITA (pedido do dono, 03/10: "quando fecha um filme quebra a animacao do
// filme indo pra ilha porque quando fecha a fileira de continue watching
// carrega").
//
// Ao fechar o player, fecharSessao (player.c) chama desc_refazer_continuar():
// cwVivo=1 -> desc_montando()=1 -> desc_home_carga().ativo=1 -> app_desenhar
// alimenta ilha_atividade("Carregando fileiras…") a cada quadro. A atividade
// tem prioridade sobre o cartao na pilula (ilha.c: alvo = M_ATIVIDADE), entao
// o quadro em voo pousa numa mini capa que nao esta la: a pilula mostra o
// texto de carga com outra largura, e o cartao so volta quando a carga acaba.
//
// Cenario A reproduz a fiacao antiga (atividade em todo quadro do voo).
// Cenario B e a fiacao nova (app.c nao manda a carga da Home a ilha durante o
// voo e o pulso, e a refacao so do Continuar nao conta como carga da Home).
// Janela GL do Mac; nao entra na suite.
#include "ilha.h"
#include "ilha_voo.h"
#include "ajustes.h"
#include "anim.h"
#include "gfx.h"
#include "text.h"
#include "tex_cache.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static SDL_Window *win;
static int carga;          // a Home "carregando" (app.c: desc_home_carga().ativo)
static int fiacaoNova;     // B: ilha_voo_silencio, a mesma regra de app.c
static IlhaVooSilencio silencio;

static float quadros(int n) {
  float x = 0, y = 0, w = 0, h = 0;
  int i;
  for (i = 0; i < n; i++) {
    Uint32 agora = SDL_GetTicks();
    tex_novo_quadro();
    tex_bombear(3);
    glClearColor(0.05f, 0.05f, 0.055f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    int calar = ilha_voo_silencio(&silencio, ilha_minimizando(), agora);
    if (carga && !(fiacaoNova && calar))
      ilha_atividade("Carregando fileiras… · Detalhes", -1);
    ilha_relogio_visivel(1);
    ilha_desenhar(agora);
    SDL_GL_SwapWindow(win);
    SDL_Delay(16);
  }
  if (!ilha_rect(&x, &y, &w, &h)) return -1.0f;
  return w;
}

int main(void) {
  const char *dir = getenv("NUVIO_DADOS");
  char ajustes[700];
  SDL_GLContext gl;
  FILE *f;
  IlhaCartao vivo;
  float wCartao, wA, wB, wAmeio, wBmeio;
  assert(dir && *dir);
  snprintf(ajustes, sizeof ajustes, "%s/ajustes.txt", dir);
  f = fopen(ajustes, "w"); assert(f);
  fprintf(f, "idioma 0\nselected_theme 2\n");
  fclose(f);
  ajustes_dir(dir);
  assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) == 0);
  IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  win = SDL_CreateWindow("Nuvio: voo cw", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                         1920, 1080, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
  assert(win);
  gl = SDL_GL_CreateContext(win);
  assert(gl);
  SDL_GL_SetSwapInterval(0);
  glViewport(0, 0, 1920, 1080);
  gfx_tamanho_alvo(1920, 1080);
  assert(gfx_iniciar());
  gfx_snap_iniciar(1920, 1080);
  assert(txt_iniciar("deploy/app", 1));
  tex_iniciar(64);
  gfx_icones_dir("deploy/app/art");
  ajustes_definir_vidro(1);

  memset(&vivo, 0, sizeof vivo);
  snprintf(vivo.chave, sizeof vivo.chave, "vivo:1");
  snprintf(vivo.imdb, sizeof vivo.imdb, "tt0111161");
  snprintf(vivo.titulo, sizeof vivo.titulo, "The Shawshank Redemption");
  snprintf(vivo.poster, sizeof vivo.poster, "deploy/app/art/poster/00.jpg");
  snprintf(vivo.arte, sizeof vivo.arte, "deploy/app/art/00.jpg");
  vivo.progresso = 0.42f; vivo.restanteMin = 80;

  // Referencia: a pilula assentada com o cartao, sem carga nenhuma.
  ilha_cartao(ILHA_VIVO, &vivo);
  wCartao = quadros(90);

  // A: fiacao antiga. O voo comeca com a refacao do Continuar no ar.
  ilha_cartao(ILHA_VIVO, NULL); carga = 0; quadros(60);
  ilha_cartao(ILHA_VIVO, &vivo);
  carga = 1; fiacaoNova = 0;
  assert(ilha_minimizar("deploy/app/art/00.jpg"));
  wAmeio = quadros(30);              // ~0,5 s: o quadro chegando na capa
  wA = quadros(10);                  // pousou (560 ms)
  printf("A (antiga): pilula no pouso %.0f px (meio %.0f), cartao %.0f px, voo %s\n",
         wA, wAmeio, wCartao, ilha_minimizando() ? "no ar" : "terminou");

  // B: fiacao nova.
  carga = 0; ilha_cartao(ILHA_VIVO, NULL); quadros(60);
  ilha_cartao(ILHA_VIVO, &vivo);
  carga = 1; fiacaoNova = 1;
  assert(ilha_minimizar("deploy/app/art/00.jpg"));
  wBmeio = quadros(30);
  wB = quadros(10);
  { float wDepois = quadros(30);   // ~0,5 s depois do pouso: ainda calado
    printf("B (nova):   0,5 s depois do pouso %.0f px\n", wDepois);
    assert(wDepois >= wCartao * 0.97f && wDepois <= wCartao * 1.03f); }
  printf("B (nova):   pilula no pouso %.0f px (meio %.0f), cartao %.0f px, voo %s\n",
         wB, wBmeio, wCartao, ilha_minimizando() ? "no ar" : "terminou");

  // O DEFEITO: com a atividade no ar, a pilula do pouso nao e a do cartao.
  assert(wA > 0 && (wA < wCartao * 0.9f || wA > wCartao * 1.1f));
  // O CONSERTO: a pilula do pouso e a do cartao (o pulso de 6% pode estar no ar).
  assert(wB >= wCartao * 0.97f && wB <= wCartao * 1.07f);

  tex_encerrar(); txt_encerrar(); gfx_encerrar();
  SDL_GL_DeleteContext(gl); SDL_DestroyWindow(win); SDL_Quit();
  puts("PASS: voo ate a ilha com a refacao do Continuar assistindo");
  return 0;
}
