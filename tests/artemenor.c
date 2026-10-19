// ARTE PEQUENA DECODIFICADA GRANDE (#5 da operacao 1.8, 05/10/2026).
//
// Os sitios de arte pequena pediam tex_obter — teto unico de 640, dimensionado
// pela maior arte de card — e desenhavam a 40-220 px: logo de servico (40),
// avatar do menu (44), capa de pasta em circulo (48), mini de salvos (84),
// logo de estudio (204), poster do cartao de amigo (220) e foto de pessoa
// (280). Um avatar natural de 640 sao 1,6 MB de textura (mais 33% de
// piramide, que bytesUsados nao conta) para um circulo de 44. Na C9 o
// gpu-cache vive encostado no teto (293,5 de ~300 MB) e cada textura dessa
// classe despeja arte que esta na tela.
//
// Agora esses sitios pedem pela largura com que desenham (tex_obter_larg): o
// cap sai de larg*escala*folga, arredondado a multiplo de 32 com piso 128
// (capDeLargura) — 128 para os quatro primeiros, 256/288/352 para os cartoes.
// O pedido continua com a chave da URL, a fila e a mesma (so cap > 660 e
// urgente) e a promocao continua SO PARA CIMA (pedido 25% maior re-decodifica;
// pedido menor nao rebaixa) — a mesma URL desenhada maior em outro canto
// promove de volta.
//
// Conferido aqui pelo caminho real (pedido -> fio de decode -> bombear ->
// publicacao), com as funcoes de verdade do tex_cache:
//   1. O CAP de cada classe de sitio, e o 640 do caminho antigo.
//   2. A MESMA ARTE natural de 640x360 (tests/amostra.jpg): 921600 bytes
//      decodificada a 640x360 pelo teto antigo, 36864 a 128x72 pelo do avatar.
//   3. O cartao de amigo (220): a mesma arte a 288x162 — e a promocao de volta
//      a 640 quando um canto maior pede, sem rebaixar quando um menor pede.
//   4. A ESCADA da URL (artetamanho.h): poster w780 do TMDB pedido a 220 vira
//      w300 no download; pedido a 640 continua w780; host sem escada intacto.
//   5. O RESAMPLE no tamanho do desenho: a mesma arte decodificada a 640 e a
//      128, ambas publicadas com a piramide e o filtro que o app escolheu,
//      desenhadas a 44 px — diferenca de poucos niveis por pixel.
//
// A copia de amostra.jpg chega por argv[1] (o .sh a faz): o cache e indexado
// pelo caminho, e as duas versoes da MESMA arte precisam de caminhos
// diferentes ao mesmo tempo.
#include "../src/sdlcompat.h"
#include <SDL2/SDL_image.h>
#include <unistd.h>
#include "../src/tex_cache.c"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// As classes de sitio da frente, com o cap esperado e a dimensao final da
// fixture (natural 288x432: abaixo do teto antigo o 640 nao mordia, e o cap
// novo so morde nas classes de 128 e 256 — a reducao de verdade, de natural
// 640, mede-se com a amostra).
static const struct { float larg; int cap, w, h; const char *nome; } PAR[] = {
  {  40.0f, 128, 128, 192, "deploy/app/art/poster/01.jpg" },  // streams.c, side
  {  44.0f, 128, 128, 192, "deploy/app/art/poster/02.jpg" },  // menu.c, NV_MENU_AVATAR
  {  48.0f, 128, 128, 192, "deploy/app/art/poster/03.jpg" },  // menu.c, TV_LOGO
  {  84.0f, 128, 128, 192, "deploy/app/art/poster/04.jpg" },  // salvosintro.c, SI_MINI_W
  { 204.0f, 256, 256, 384, "deploy/app/art/poster/05.jpg" },  // detail.c, EST_CARD_W-36
  { 220.0f, 288, 288, 432, "deploy/app/art/poster/06.jpg" },  // recomenda.c, RC_POSTER_W
  { 280.0f, 352, 288, 432, "deploy/app/art/poster/07.jpg" },  // detail.c, PES_FOTO_W
};
#define N_PAR ((int)(sizeof PAR / sizeof *PAR))

// Busca pelo caminho COM TRAVA PROPRIA — e nao BUSCA_MEDIDA, que toma o mutex
// e devolve a responsabilidade de solta-lo ao chamador do tex_cache: usada
// fora de la, cada chamada vaza um nivel de trava e o fio de decode (que so
// e desperto pelo signal do enqueue) morre esperando o mutex que a main
// segura sem saber. E o bug que o primeiro run deste teste pegou.
static int achar(const char *nome) {
  unsigned long h = hashCaminho(nome);
  int i;
  SDL_LockMutex(mtx);
  i = acharIndice(nome, h);
  SDL_UnlockMutex(mtx);
  return i;
}

static long bytesAgora(void) {
  long b;
  SDL_LockMutex(mtx);
  b = bytesUsados;
  SDL_UnlockMutex(mtx);
  return b;
}

// Espera o item ficar PRONTO re-pedindo como o desenho real faz: o pedido vem
// PRIMEIRO em cada quadro — conferir o estado antes de pedir perderia justamente
// a promocao (o item PRONTO na versao pequena nunca receberia o pedido maior).
static void pedidoEBomba(const char *nome, float larg) {
  int t;
  for (t = 0; t < 600; t++) {
    int i;
    Estado e = VAZIO;
    if (larg > 0.0f) tex_obter_larg(nome, larg); else tex_obter(nome);
    i = achar(nome);
    if (i >= 0) {
      SDL_LockMutex(mtx);
      e = itens[i].estado;
      SDL_UnlockMutex(mtx);
      if (e == PRONTO) return;
      if (e == FALHOU) {
        printf("[artemenor] decode falhou: %s\n", nome);
        fflush(stdout);
        assert(!"decode falhou");
      }
    }
    tex_novo_quadro();
    tex_bombear(8);
    SDL_Delay(5);
  }
  printf("[artemenor] timeout esperando %s\n", nome);
  fflush(stdout);
  assert(!"timeout no decode");
}

// Confere o item publicado: cap pedido (itens[].limite) e dimensao final.
static void conferir(const char *nome, int limite, int w, int h) {
  int i = achar(nome);
  Estado e;
  int L, W, H;
  assert(i >= 0);
  SDL_LockMutex(mtx);
  e = itens[i].estado; L = itens[i].limite; W = itens[i].w; H = itens[i].h;
  SDL_UnlockMutex(mtx);
  assert(e == PRONTO);
  assert(L == limite);
  assert(W == w);
  assert(H == h);
}

// Desenha a textura publicada em um quadrado de 44 px, recorte cover do
// centro (como o avatar: aspecto forcado a 1.0) — a mesma regiao relativa
// nos dois tetos, porque o aspecto da arte e o mesmo. A piramide e o filtro
// sao os que o upload real deixou na textura.
#define QUAD 44
static GLuint fbo, fboTex;
static void desenhar44(GLuint tex) {
  glBindTexture(GL_TEXTURE_2D, tex);
  glEnable(GL_TEXTURE_2D);
  glBegin(GL_QUADS);
  glTexCoord2f(0.21875f, 0.0f); glVertex2f(0.0f, 0.0f);
  glTexCoord2f(0.78125f, 0.0f); glVertex2f((float)QUAD, 0.0f);
  glTexCoord2f(0.78125f, 1.0f); glVertex2f((float)QUAD, (float)QUAD);
  glTexCoord2f(0.21875f, 1.0f); glVertex2f(0.0f, (float)QUAD);
  glEnd();
}

int main(int argc, char **argv) {
  SDL_Window *w;
  SDL_GLContext gl;
  const char *copia = argc > 1 ? argv[1] : NULL;   // a MESMA arte da amostra, caminho proprio
  long b0, b1;
  int i;
  GLuint texVelha, texNova;
  static unsigned char pa[QUAD * QUAD * 4], pb[QUAD * QUAD * 4];

  assert(copia && copia[0]);
  assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) == 0);
  IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  w = SDL_CreateWindow("artemenor", 0, 0, 64, 64, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
  assert(w);
  gl = SDL_GL_CreateContext(w);
  assert(gl);

  assert(tex_iniciar(64) == 1);   // devolve thr != NULL
  tex_escala(1.0f);   // a escala da TV: na C9 o pixel de layout e o do buffer

  // 1. O CAP de cada classe de sitio — e o 640 do caminho antigo.
  for (i = 0; i < N_PAR; i++) {
    b0 = bytesAgora();
    pedidoEBomba(PAR[i].nome, PAR[i].larg);
    b1 = bytesAgora();
    conferir(PAR[i].nome, PAR[i].cap, PAR[i].w, PAR[i].h);
    printf("[artemenor] classe larg=%.0f cap=%d: %dx%d, %ld bytes (fixture natural 288x432)\n",
           PAR[i].larg, PAR[i].cap, PAR[i].w, PAR[i].h, b1 - b0);
    fflush(stdout);
  }
  b0 = bytesAgora();
  pedidoEBomba("deploy/app/art/poster/00.jpg", 0.0f);   // 0 = caminho antigo
  b1 = bytesAgora();
  conferir("deploy/app/art/poster/00.jpg", 640, 288, 432);
  printf("[artemenor] caminho antigo tex_obter: cap=640, natural 288 nao mordida, %ld bytes\n", b1 - b0);
  fflush(stdout);

  // 2. A reducao de verdade: a mesma arte natural 640x360 no teto do cartao
  //    de amigo (220 -> cap 288) e no do avatar (40 -> cap 128).
  b0 = bytesAgora();
  pedidoEBomba(copia, 220.0f);
  b1 = bytesAgora();
  conferir(copia, 288, 288, 162);
  printf("[artemenor] cartao de amigo (cap 288): 640x360 -> 288x162, %ld bytes (teto antigo: 921600)\n", b1 - b0);
  fflush(stdout);

  b0 = bytesAgora();
  pedidoEBomba("tests/amostra.jpg", 40.0f);
  b1 = bytesAgora();
  conferir("tests/amostra.jpg", 128, 128, 72);
  printf("[artemenor] avatar (cap 128): 640x360 -> 128x72, %ld bytes (teto antigo: 921600)\n", b1 - b0);
  fflush(stdout);

  // 3. A promocao SO PARA CIMA: o teto antigo (640) pedido depois re-sobe; o
  //    pedido menor (40) que vem a seguir NAO rebaixa — sem churn quando a
  //    mesma URL e desenhada maior em um canto e menor em outro.
  pedidoEBomba(copia, 0.0f);
  conferir(copia, 640, 640, 360);
  printf("[artemenor] promocao: cap 640 pedido depois -> 640x360 de volta\n");
  { int t;
    for (t = 0; t < 20; t++) {
      tex_obter_larg(copia, 40.0f);
      tex_novo_quadro();
      tex_bombear(8);
      SDL_Delay(5);
    } }
  conferir(copia, 640, 640, 360);
  printf("[artemenor] pedido menor depois de promovido: continua 640x360 PRONTO, sem re-decode\n");
  fflush(stdout);

  // 4. A escada da URL: o download tambem emagrece (artetamanho.h).
  { char s[600];
    assert(arte_tamanho_url("https://image.tmdb.org/t/p/w780/fIhXD6m9LJgC7yDMOHMpYgkYAJ.jpg", 288, s, sizeof s) == 1);
    assert(!strcmp(s, "https://image.tmdb.org/t/p/w300/fIhXD6m9LJgC7yDMOHMpYgkYAJ.jpg"));
    assert(arte_tamanho_url("https://image.tmdb.org/t/p/w780/fIhXD6m9LJgC7yDMOHMpYgkYAJ.jpg", 640, s, sizeof s) == 0);
    assert(arte_tamanho_url("https://pub-9a1b2c3d4e5f6g.r2.dev/avatar.png", 128, s, sizeof s) == 0);
    printf("[artemenor] escada: poster w780 pedido a 220 baixa w300; a 640 continua w780; r2.dev sem escada\n");
    fflush(stdout); }

  // 5. O resample no tamanho do desenho: as duas versoes publicadas da mesma
  //    arte (640 e 128), desenhadas a 44 px como o avatar.
  texVelha = tex_obter(copia);
  texNova = tex_obter_larg("tests/amostra.jpg", 40.0f);
  assert(texVelha && texNova);
  glGenTextures(1, &fboTex);
  glBindTexture(GL_TEXTURE_2D, fboTex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, QUAD, QUAD, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTex, 0);
  assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
  glViewport(0, 0, QUAD, QUAD);
  glMatrixMode(GL_PROJECTION); glLoadIdentity(); glOrtho(0, QUAD, 0, QUAD, -1, 1);
  glMatrixMode(GL_MODELVIEW); glLoadIdentity();
  glDisable(GL_BLEND);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  desenhar44(texVelha);
  glReadPixels(0, 0, QUAD, QUAD, GL_RGBA, GL_UNSIGNED_BYTE, pa);
  glClear(GL_COLOR_BUFFER_BIT);
  desenhar44(texNova);
  glReadPixels(0, 0, QUAD, QUAD, GL_RGBA, GL_UNSIGNED_BYTE, pb);
  { int maxd = 0; long soma = 0; int k;
    for (k = 0; k < QUAD * QUAD * 4; k++) {
      int d = pa[k] > pb[k] ? pa[k] - pb[k] : pb[k] - pa[k];
      soma += d;
      if (d > maxd) maxd = d;
    }
    printf("[artemenor] resample a 44 px (mesma arte, teto 640 vs 128): delta max %d, medio %.3f por byte\n",
           maxd, (double)soma / (QUAD * QUAD * 4.0));
    fflush(stdout);
    assert(maxd <= 48);   // apertar ao medido: o numero acima e o teto honesto
  }

  printf("[artemenor] tudo conferido\n");
  fflush(stdout);
  return 0;
}
