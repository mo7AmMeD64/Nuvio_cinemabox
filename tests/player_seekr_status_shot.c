// Real player OSD/GL with fixed status/usage. No requests or quota dispatch.
#include "../src/seekr.h"
#include "../src/plrui.h"
static int statusFixture, temChaveFixture = 1, materialCalls;
static SeekrUso usoFixture;
static int estadoFixture(void) { return statusFixture; }
static int chaveFixture(void) { return temChaveFixture; }
static void obterUsoFixture(SeekrUso *u) { *u = usoFixture; }
static void pedirFixture(const char *id, int t, int e, long d) {
  (void)id; (void)t; (void)e; (void)d;
}
static void materialFixture(GfxRect r, float raio, int modal, float a) {
  materialCalls++;
  plrui_material(r, raio, modal, a);
}
#define seekr_estado estadoFixture
#define seekr_tem_chave chaveFixture
#define seekr_uso obterUsoFixture
#define seekr_pedir pedirFixture
#define plrui_material materialFixture
#include "../src/player.c"
#undef seekr_estado
#undef seekr_tem_chave
#undef seekr_uso
#undef seekr_pedir
#undef plrui_material
#define main iniciarFixtureGlass
#include "player_glass_shot.c"
#undef main

int main(int argc, char **argv) {
  assert(argc == 2);
  char *init[] = {argv[0], argv[1], "seekr-status-init"};
  assert(iniciarFixtureGlass(3, init) == 0);
  setenv("TZ", "America/Sao_Paulo", 1); tzset();
  ajustes_shot_valor("seekrChave", 1);
  ajustes_shot_valor("seekrLocal", 0);
  usoFixture = (SeekrUso){.usadas=50,.limite=50,.persistente=1,
    .reinicioUtc=1735776000LL,.retryUtc=1735779600LL};
  const int states[] = {SEEKR_LIMITE_LOCAL, SEEKR_LIMITE_PROVEDOR,
    SEEKR_CHAVE_RECUSADA, SEEKR_REDE_INDISPONIVEL,
    SEEKR_ARMAZENAMENTO_INDISPONIVEL, SEEKR_RELOGIO_INDISPONIVEL,
    SEEKR_SEM_PREVIA, SEEKR_DESLIGADO};
  const char *ids[] = {"seekr-local", "seekr-provider", "seekr-key", "seekr-network",
    "seekr-storage", "seekr-clock", "seekr-empty", "seekr-no-key"};
  abrir(&filme); simular(3840,1606,"",1,1);
  player_shot_estado(relogio,4360,9420,1,0,1,1);
  player_shot_buscando(1);
  for (size_t i=0;i<sizeof states/sizeof states[0];i++) {
    statusFixture=states[i]; temChaveFixture=states[i]!=SEEKR_DESLIGADO;
    ajustes_shot_valor("seekrChave", temChaveFixture);
    player_shot_estado(relogio,4360,9420,1,0,1,1);
    player_shot_buscando(1);
    materialCalls=0;
    quadros(80); assert(materialCalls > 0); salvar(ids[i]);
    materialCalls=0;
    seekrMiniatura(96,1728,.5f,850,1);
    assert(materialCalls == 1);
  }
  // The status is ephemeral: visual smoothing after release is not a new
  // blocked-preview alert. It also never shows when the option is disabled.
  statusFixture=SEEKR_LIMITE_LOCAL;
  scrubbing=0; posVisSolto=1; materialCalls=0;
  seekrMiniatura(96,1728,.5f,850,1);
  assert(!materialCalls);
  scrubbing=1; ajustes_shot_valor("seekrLocal",1);
  seekrMiniatura(96,1728,.5f,850,1);
  assert(!materialCalls);
  puts("player Seekr GL: PASS fixed states, no dispatch, no status after release/disabled");
}
