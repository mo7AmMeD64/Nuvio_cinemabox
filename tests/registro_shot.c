// CAPTURAS DO REGISTRO DO APP NO GLASS UI (logs-mockup.html, 03/10), sem
// rede e sem interacao: o painel do botao vermelho (ao vivo, pausado, filtros,
// etapas, vazio), Ajustes > Sobre e ajuda com o painel de envio (enviando,
// enviado, erro, sem internet), o aviso de primeira vez, o consentimento do
// envio automatico, a queda da sessao anterior e o medidor de desempenho.
//
// As linhas sao as do mockup (registro_shot_sessao.h); os estados sao
// montados a mao (registro.c e incluido). A arte de tras: NUVIO_SHOT_ARTE_LOG,
// _AJ, _TEL (os fundos do mockup) ou a embarcada. NUVIO_SHOT_VIDRO=0 = solido.
// Uso: bash tests/registro_shot.sh <pasta> [ids...]
#define REGISTRO_TESTE 1
#include "../src/registro.c"
#include "registro_shot_sessao.h"
#include "shot_arte.h"
#include "ilha.h"
#include "ajustes_ux.h"
#include "badges.h"
#include "desempenho.h"
#include "plrilha.h"
#include "telemetria.h"
#include <SDL2/SDL_image.h>
#include <assert.h>

extern int  ajustes_teste_quadro(const char *id);
extern void telemetria_teste_abrir(void);

static GLuint fbo, fboTex;
static const char *saida;
static int nIds;
static char **ids;

static int quer(const char *id) {
  int i;
  if (!nIds) return 1;
  for (i = 0; i < nIds; i++) if (!strcmp(ids[i], id)) return 1;
  return 0;
}

// Arte de tela cheia de um arquivo, com veu.
static GLuint artes[4];
static GLuint carregaArte(const char *c) {
  SDL_Surface *s, *t;
  GLuint tex;
  if (!c || !*c) return 0;
  s = IMG_Load(c);
  if (!s) return 0;
  t = SDL_ConvertSurfaceFormat(s, SDL_PIXELFORMAT_ABGR8888, 0);
  SDL_FreeSurface(s);
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, t->pitch / 4);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t->w, t->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, t->pixels);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  gfx_tex_aspect_atual = (float)t->w / (float)t->h;
  SDL_FreeSurface(t);
  return tex;
}
static void arte(int qual, float veu) {
  static const char *ENV[4] = { "NUVIO_SHOT_ARTE_LOG", "NUVIO_SHOT_ARTE_AJ", "NUVIO_SHOT_ARTE_TEL", NULL };
  if (!artes[qual] && ENV[qual]) artes[qual] = carregaArte(getenv(ENV[qual]));
  if (artes[qual]) {
    gfx_tex_aspect_atual = 16.0f / 9.0f;
    gfx_rect((GfxRect){ 0, 0, 1920, 1080 }, artes[qual], GFX_CARD, 0, 0, 0, 0, 0, 0, 0, 1);
    gfx_tex_aspect_atual = 0;
  } else shot_arte_desenhar(0);
  if (veu > 0) gfx_cor((GfxRect){ 0, 0, 1920, 1080 }, 0, 0, 0, 0, veu);
}

enum { CENA_PAINEL_AJ, CENA_PAINEL, CENA_AVISO, CENA_AJ, CENA_TEL, CENA_QUEDA, CENA_HUD, CENA_HUD_ATV, CENA_HUD_PLAYER };
// O medidor e conteudo da ilha do relogio (desempenho.h): a ilha no canto de
// sempre, sem ancora, como app.c a desenha na home.
static void relogioPadrao(void) { ilha_relogio_visivel(1); ilha_desenhar(SDL_GetTicks()); }

static void relogio(float x) {
  ilha_relogio_visivel(1);
  ilha_posicionar(1); (void)x;
  ilha_desenhar(SDL_GetTicks());
}
static void captura(const char *id, int cena) {
  char nome[700];
  int i;
  if (!quer(id)) return;
  snprintf(nome, sizeof nome, "%s/%s.bmp", saida, id);
  for (i = 0; i < 70; i++) {
    SDL_PumpEvents();
    txt_novo_quadro();
    tex_novo_quadro();
    tex_bombear(6);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, 1920, 1080);
    glClearColor(0.043f, 0.047f, 0.055f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    switch (cena) {
      case CENA_PAINEL: arte(0, 0); registro_desenhar(); relogio(48); break;
      // The panel over the REAL previous screen (Settings), as app.c composes it:
      // Settings is drawn first and the panel's veil goes over it. Nothing of
      // Settings may show between the two panels (M sweep, 04/10/2026).
      case CENA_PAINEL_AJ:
        ajustes_atualizar(1.0f / 60.0f, SDL_GetTicks());
        ajustes_desenhar(SDL_GetTicks());
        registro_desenhar(); relogio(48); break;
      case CENA_AVISO:  arte(1, 0.30f); registro_desenhar(); break;
      case CENA_AJ:
        ajustes_atualizar(1.0f / 60.0f, SDL_GetTicks());
        ajustes_desenhar(SDL_GetTicks());
        if (!registro_envio_aberto()) relogio(ajustes_ilha_x());
        break;
      case CENA_TEL: arte(2, 0.30f); telemetria_atualizar(1.0f / 60.0f, SDL_GetTicks()); telemetria_desenhar(SDL_GetTicks()); break;
      case CENA_QUEDA: arte(1, 0.30f); avisos_atualizar(1.0f / 60.0f, SDL_GetTicks()); ilha_relogio_visivel(1); ilha_posicionar(1); ilha_desenhar(SDL_GetTicks()); break;
      case CENA_HUD: arte(1, 0.30f); relogioPadrao(); break;
      case CENA_HUD_ATV: arte(1, 0.30f); ilha_atividade("Carregando fileiras…", -1.0f); relogioPadrao(); break;
      case CENA_HUD_PLAYER: arte(2, 0); plrilha_relogio(1.0f, 2460.0); plrilha_desenhar(SDL_GetTicks()); break;
    }
    SDL_Delay(8);
    if (i == 69) {
      unsigned char *pix = malloc(1920 * 1080 * 4);
      SDL_Surface *s;
      int y;
      assert(pix);
      glFinish();
      glReadPixels(0, 0, 1920, 1080, GL_RGBA, GL_UNSIGNED_BYTE, pix);
      s = SDL_CreateRGBSurfaceWithFormat(0, 1920, 1080, 32, SDL_PIXELFORMAT_RGBA32);
      for (y = 0; y < 1080; y++) memcpy((char *)s->pixels + y * s->pitch, pix + (1079 - y) * 1920 * 4, 1920 * 4);
      assert(SDL_SaveBMP(s, nome) == 0);
      SDL_FreeSurface(s);
      free(pix);
    }
  }
  printf("captura: %s\n", nome);
}

// Os hosts do quadro "filtro-rede", como rede.c os contaria.
static void hostsDeMentira(void) {
  static const struct { const char *h; int n, f, ms; } H[] = {
    { "imagens.exemplo", 184, 1, 210 }, { "addon-a.exemplo", 12, 0, 812 }, { "addon-b.exemplo", 9, 3, 3000 },
    { "metadados.exemplo", 41, 1, 340 }, { "sync.exemplo", 23, 0, 180 }, { "legendas.exemplo", 4, 1, 520 } };
  int i, k;
  for (i = 0; i < 6; i++) {
    char u[120];
    snprintf(u, sizeof u, "https://%s/x", H[i].h);
    for (k = 0; k < H[i].n; k++) rede_hosts_nota(u, k < H[i].f ? 6 : 0, 200, (unsigned)H[i].ms);
  }
}

static const char ETAPAS_TPK40[] =
  "[tv] modelo=UN55RU7100 host=tizen-5.0 dotnet=2.0 tela=1920x1080\n"
  "[etapa-anterior] host begin nv_tpk_iniciar @0.51s\n"
  "[etapa-anterior] host ok nv_tpk_iniciar @1.90s\n"
  "[etapa-anterior] host begin first-frame @1.91s\n"
  "[etapa-anterior] host ok first-frame @3.40s\n"
  "[etapa-anterior] host note main-window visible=False @1843.20s\n"
  "[etapa-anterior] host note app ended: main returned @1843.22s\n"
  "[etapa] host note launch .NET Core 2.0.0 @0.00s\n"
  "[etapa] host begin read-so @0.04s\n"
  "[etapa] host ok read-so 4182016 bytes @0.29s\n"
  "[etapa] host begin memfd @0.29s\n"
  "[etapa] host ok memfd loader=memfd @0.35s\n"
  "[etapa] host begin video-window @0.36s\n"
  "[etapa] host ok video-window @0.52s\n"
  "[etapa] host begin nv_tpk_iniciar @0.52s\n"
  "[etapa] native note app-main started\n"
  "[etapa] native ok libcurl-dlopen libcurl.so.5\n"
  "[etapa] native note sign-in-thread started\n"
  "[etapa] host ok nv_tpk_iniciar @1.94s\n"
  "[etapa] host begin first-frame @1.95s\n"
  "[etapa] host ok first-frame @3.12s\n"
  "[etapa] host note main-window visible=True @3.13s\n";

int main(int argc, char **argv) {
  const char *dir = getenv("NUVIO_DADOS");
  SDL_Window *w;
  SDL_GLContext gl;
  assert(argc > 1 && dir && *dir);
  saida = argv[1];
  nIds = argc - 2; ids = argv + 2;
  SDL_SetHint("SDL_MAC_BACKGROUND_APP", "1");
  assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) == 0);
  IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
  dados_iniciar(dir);
  { char caminho[600];
    FILE *f;
    snprintf(caminho, sizeof caminho, "%s/ajustes.txt", dir);
    f = fopen(caminho, "w");
    assert(f);
    fprintf(f, "idioma 0\nanimacoes 0\n");
    // NUVIO_SHOT_FONTE=3: Montserrat, the interface font of the owner's TV.
    if (getenv("NUVIO_SHOT_FONTE")) fprintf(f, "fonteInterface %d\n", atoi(getenv("NUVIO_SHOT_FONTE")));
    // Oceano (#42a5f5), o acento do mockup; NUVIO_SHOT_TEMA=<indice> troca.
    fprintf(f, "selected_theme %s\n", getenv("NUVIO_SHOT_TEMA") ? getenv("NUVIO_SHOT_TEMA") : "2");
    shot_arte_material(f);
    fclose(f); }
  ajustes_dir(dir);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  w = SDL_CreateWindow("Nuvio: captura do registro", 0, 0, 64, 64, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
  assert(w);
  gl = SDL_GL_CreateContext(w);
  assert(gl);
  SDL_GL_SetSwapInterval(0);
  glGenTextures(1, &fboTex);
  glBindTexture(GL_TEXTURE_2D, fboTex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1920, 1080, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTex, 0);
  assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
  glViewport(0, 0, 1920, 1080);
  gfx_tamanho_alvo(1920, 1080);
  assert(gfx_iniciar());
  assert(txt_iniciar("deploy/app", 1));
  ajustes_recursos(getenv("NUVIO_SHOT_ARTE_DIR") ? getenv("NUVIO_SHOT_ARTE_DIR") : "deploy/app/art");
  gfx_icones_dir("deploy/app/art");
  badges_carregar("deploy/app/art");
  tex_iniciar(64);
  ajustes_iniciar();
  (void)gl;

  // --- painel -------------------------------------------------------------
  registro_teste_texto(REG_SHOT_SESSAO);
  registro_abrir();
  hostsDeMentira();
  avisos_teste_envio_auto(40, 200);
  registro_teste_estado(RG_TUDO, 0, 0, 0, 0, 0);
  if (quer("painel-sobre-ajustes")) {
    ajustes_teste_quadro("op:-registro");
    captura("painel-sobre-ajustes", CENA_PAINEL_AJ);
  }
  captura("painel", CENA_PAINEL);
  registro_teste_estado(RG_TUDO, 0, 1, registro_teste_achar("decode falhou", 1), 1, 12);
  captura("pausado", CENA_PAINEL);
  // Many new lines: the pill gets wider than the room right of the area tabs
  // and must move to the header instead of covering them.
  registro_teste_estado(RG_TUDO, 0, 1, registro_teste_achar("decode falhou", 1), 1, 123456);
  captura("pausado-largo", CENA_PAINEL);
  registro_teste_estado(RG_PROB, 1, 0, 0, 0, 0);
  captura("filtro-problemas", CENA_PAINEL);
  registro_teste_estado(RG_REDE, 0, 0, 0, 0, 0);
  captura("filtro-rede", CENA_PAINEL);
  registro_teste_texto(ETAPAS_TPK40);
  registro_abrir();
  aberto = 0; registro_abrir();
  registro_teste_estado(RG_SISTEMA, 0, 0, 0, 0, 0);
  registro_teste_foco_etapa(4);
  captura("etapas", CENA_PAINEL);
  registro_teste_sem_fonte(1);
  aberto = 0; registro_abrir();
  captura("vazio", CENA_PAINEL);
  aberto = 0;
  registro_teste_sem_fonte(0);
  registro_teste_texto(REG_SHOT_SESSAO);
  carregar();

  // --- Ajustes > Sobre e ajuda e o painel de envio ---------------------------
  { char l[64];
    time_t ontem = time(NULL) - 86400;
    struct tm t;
    localtime_r(&ontem, &t);
    t.tm_hour = 22; t.tm_min = 47; t.tm_sec = 0;
    snprintf(l, sizeof l, "K7QM2X %ld\n", (long)mktime(&t));
    dados_gravar("registro-codigo.txt", l); }
  if (quer("sobre") || quer("enviando") || quer("enviado") || quer("enviar-erro") || quer("enviar-offline")) {
    ajustes_teste_quadro("op:-registro");
    jaAberto = 0;   // o "NOVO" da linha: ninguem abriu o registro por Ajustes
    captura("sobre", CENA_AJ);
    registro_teste_envio(1);
    avisos_teste_envio(1, 0, 0, "", 186L * 1024, 2214);
    captura("enviando", CENA_AJ);
    avisos_teste_envio(2, AVISOS_ENVIO_OK, 200, "K7QM2X", 186L * 1024, 2214);
    captura("enviado", CENA_AJ);
    avisos_teste_envio(3, AVISOS_ENVIO_SERVIDOR, 500, "", 186L * 1024, 2214);
    registro_teste_envio_foco(0);
    captura("enviar-erro", CENA_AJ);
    { int k;   // a TV sem internet: seis falhas de transporte em tres hosts
      for (k = 0; k < 6; k++) rede_saude_nota(6, k % 3 == 0 ? "https://addon-b.exemplo/x" : k % 3 == 1 ? "https://imagens.exemplo/x" : "https://sync.exemplo/x"); }
    avisos_teste_envio(3, AVISOS_ENVIO_OFFLINE, 0, "", 186L * 1024, 2214);
    captura("enviar-offline", CENA_AJ);
    registro_teste_envio(0);
    avisos_teste_envio(0, 0, 0, "", 0, 0);
  }

  // --- aviso de primeira vez --------------------------------------------------
  registro_teste_aviso(1);
  captura("aviso", CENA_AVISO);
  registro_teste_aviso(0);

  // --- a queda da sessao anterior, na ilha do relogio ---------------------------
  if (quer("queda")) {
    int k;
    ajustes_definir_envio_auto(0);
    avisos_teste_queda("Em 2026-10-02 22:41 o Nuvio parou sem avisar. Se quiser, envie o registro daquela sessão para ajudar a encontrar a causa.");
    for (k = 0; k < 30; k++) { ilha_desenhar(SDL_GetTicks()); SDL_Delay(10); }
    { SDL_Event e = { 0 };   // AZUL/CH+: a pilula cresce no modal do aviso
      e.type = SDL_KEYDOWN; e.key.keysym.sym = SDLK_PAGEUP; e.key.keysym.scancode = SDL_SCANCODE_PAGEUP;
      ilha_evento(&e);
      e.type = SDL_KEYUP; ilha_evento(&e); }
    for (k = 0; k < 90; k++) { txt_novo_quadro(); ilha_desenhar(SDL_GetTicks()); SDL_Delay(10); }   // a mola assenta
    captura("queda", CENA_QUEDA);
    ilha_modal_fechar(1);
    ajustes_definir_envio_auto(1);
  }

  // --- consentimento do envio automatico (Samsung) ------------------------------
  telemetria_teste_abrir();
  captura("consentimento", CENA_TEL);
  telemetria_desenhar(0);

  // --- medidor de desempenho ----------------------------------------------------
  { static const float PIOR[36] = { 21,19,22,24,20,18,22,26,21,19,20,23,31,38,61,44,33,24,20,19,21,22,20,25,19,18,20,21,22,19,18,20,23,19,18,18 };
    desempenho_amostra(59.8f, 21, 0, 38, 96, 2, 41, 0, 212);
    desempenho_teste_serie(PIOR, 36);
    desempenho_teste_forma(DS_MINIMO);  captura("hud-minimo", CENA_HUD);
    desempenho_teste_forma(DS_MENOR);   captura("hud-menor", CENA_HUD);
    desempenho_teste_forma(DS_GRANDE);  captura("hud-grande", CENA_HUD);
    // Ilha ocupada: a atividade passa na frente e o medidor sai; ela some e ele volta.
    desempenho_teste_forma(DS_MINIMO);  captura("hud-minimo-atividade", CENA_HUD_ATV);
    captura("hud-minimo-volta", CENA_HUD);
    desempenho_teste_forma(DS_GRANDE);  captura("hud-grande-atividade", CENA_HUD_ATV);
    desempenho_amostra(38.0f, 61, 4, 41, 99, 0, 44, 0, 241);   // lento: ambar
    desempenho_teste_forma(DS_MINIMO);  captura("hud-player-minimo", CENA_HUD_PLAYER);
    desempenho_teste_forma(DS_MENOR);   captura("hud-player-menor", CENA_HUD_PLAYER);
    desempenho_teste_forma(DS_GRANDE);  captura("hud-player-grande", CENA_HUD_PLAYER);
    desempenho_teste_forma(-1); }
  return 0;
}
