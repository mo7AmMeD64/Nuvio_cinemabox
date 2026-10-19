// CICLO COMPLETO E "POR ADD-ON" (src/vazao.c), sem rede: a selecao das
// candidatas (rapido x completo x por add-on, dedupe, teto, debrid), o
// agendador de uma fonte por vez (cancelar, orcamento, parar nas 3), o ranking,
// o "toca ou nao toca" e a censura de host. A rede e uma FAKE: o callback
// `medir` devolve o que o roteiro manda e registra a ordem das chamadas.
#include "vazao.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int falhas;
#define CONFERIR(cond, msg) do { if (cond) printf("ok  %s\n", msg); \
  else { printf("FALHA %s\n", msg); falhas++; } } while (0)

// ---- selecao ---------------------------------------------------------------
static void selecao(void) {
  // Ordem do automatico: add-on 0 tem 4 fontes, o 1 tem 3, o 2 tem 1; uma
  // repete o link da outra (chave 7); duas sao debrid (fora de cache/torrent).
  VazItem it[] = {
    { 0, 1, 1 }, { 0, 2, 1 }, { 1, 3, 1 }, { 1, 4, 0 }, { 0, 5, 1 }, { 2, 6, 1 },
    { 1, 7, 1 }, { 0, 7, 1 }, { 0, 9, 1 }, { 1, 10, 0 }, { 1, 11, 1 }
  };
  int n = (int)(sizeof it / sizeof it[0]), fila[64], q, k;
  VazSelecao s;
  q = vazao_selecionar(VCM_RAPIDO, it, n, 64, fila, &s);
  CONFERIR(q == 8 && s.debrid == 2 && s.duplicadas == 1 && s.candidatas == n,
           "rapido: todas as mediveis na ordem, sem debrid nem link repetido");
  CONFERIR(fila[0] == 0 && fila[1] == 1 && fila[2] == 2 && fila[3] == 4,
           "rapido: ordem do automatico preservada");

  q = vazao_selecionar(VCM_COMPLETO, it, n, 64, fila, &s);
  CONFERIR(q == 8 && s.fila == 8, "completo: entram as mesmas 8");
  // Rodadas: (0:#0, 1:#2, 2:#5) (0:#1, 1:#6) (0:#4, 1:#10) (0:#8)
  { static const int esperado[8] = { 0, 2, 5, 1, 6, 4, 10, 8 };
    int igual = 1;
    for (k = 0; k < 8; k++) if (fila[k] != esperado[k]) igual = 0;
    CONFERIR(igual, "completo: revezamento entre os add-ons (1a de cada, 2a de cada...)"); }

  q = vazao_selecionar(VCM_COMPLETO, it, n, 5, fila, &s);
  CONFERIR(q == 5 && s.foraDoLimite == 3, "completo: o teto corta e conta as que ficaram de fora");
  { int a0 = 0, a1 = 0, a2 = 0;
    for (k = 0; k < q; k++) { int a = it[fila[k]].addon; a0 += a == 0; a1 += a == 1; a2 += a == 2; }
    CONFERIR(a0 && a1 && a2, "completo: o teto nao deixa nenhum add-on sem fonte"); }

  q = vazao_selecionar(VCM_ADDON, it, n, 64, fila, &s);
  CONFERIR(q == 3 && fila[0] == 0 && fila[1] == 2 && fila[2] == 5,
           "por add-on: a melhor de cada, na ordem dos add-ons");

  // Add-on cuja 1a e debrid: a melhor MEDIVEL dele.
  { VazItem b[] = { { 0, 1, 0 }, { 0, 2, 1 }, { 1, 3, 1 } };
    q = vazao_selecionar(VCM_ADDON, b, 3, 8, fila, &s);
    CONFERIR(q == 2 && fila[0] == 1 && fila[1] == 2 && s.debrid == 1,
             "por add-on: pula a debrid e pega a proxima medivel do mesmo add-on"); }

  // Teto de 40 com 60 fontes de 3 add-ons.
  { VazItem g[60];
    int i;
    for (i = 0; i < 60; i++) { g[i].addon = i % 3; g[i].chave = (unsigned long)(i + 100); g[i].medivel = 1; }
    q = vazao_selecionar(VCM_COMPLETO, g, 60, VAZ_CICLO_MAX, fila, &s);
    CONFERIR(q == 40 && s.foraDoLimite == 20, "teto de 40 fontes, 20 contadas como de fora"); }

  q = vazao_selecionar(VCM_COMPLETO, it, 0, 8, fila, &s);
  CONFERIR(q == 0 && s.fila == 0, "sem candidatas: fila vazia");
}

// ---- agendador -------------------------------------------------------------
typedef struct {
  int chamadas, ordem[64], dentro, maxDentro, cancelarNaChamada, passos, ultimoFeitos;
  unsigned long relogio, custoMs;
  int roteiro[64];
} Fake;

static int fMedir(int i, void *u) {
  Fake *f = (Fake *)u;
  f->dentro++;
  if (f->dentro > f->maxDentro) f->maxDentro = f->dentro;
  f->ordem[f->chamadas++] = i;
  f->relogio += f->custoMs;
  f->dentro--;
  return f->roteiro[i] ? f->roteiro[i] : VS_OK;
}
static int fCancelado(void *u) {
  Fake *f = (Fake *)u;
  return f->cancelarNaChamada && f->chamadas >= f->cancelarNaChamada;
}
static unsigned long fAgora(void *u) { return ((Fake *)u)->relogio; }
static void fPasso(void *u, int feitos, int total) {
  Fake *f = (Fake *)u;
  (void)total;
  f->passos++;
  f->ultimoFeitos = feitos;
}

static void agendador(void) {
  int fila[40], k;
  Fake f;
  VazOps ops = { fMedir, fCancelado, fAgora, fPasso, &f };
  VazPlano pl;
  VazAgenda a;
  for (k = 0; k < 40; k++) fila[k] = k * 2;   // indices "reais" diferentes de k

  memset(&f, 0, sizeof f);
  memset(&pl, 0, sizeof pl);
  vazao_agendar(&pl, fila, 12, &ops, &a);
  CONFERIR(a.tentadas == 12 && a.medidas == 12 && f.maxDentro == 1,
           "completo: mede todas, uma de cada vez");
  { int ok = 1;
    for (k = 0; k < 12; k++) if (f.ordem[k] != fila[k]) ok = 0;
    CONFERIR(ok, "completo: na ordem da fila"); }
  CONFERIR(f.passos == 12 && f.ultimoFeitos == 12, "progresso: 12 de 12, um passo por fonte");

  // Rapido: para nas 3 medidas, host repetido nao conta como medida.
  memset(&f, 0, sizeof f);
  f.roteiro[2 * 1] = VS_HOST_REPETIDO;
  pl = (VazPlano){ 3, 6, 45000UL, 1 };
  vazao_agendar(&pl, fila, 12, &ops, &a);
  CONFERIR(a.medidas == 3 && a.hostRepetido == 1 && a.tentadas == 4 && a.restantes == 8,
           "rapido: 3 medidas de hosts diferentes; o repetido nao conta");

  memset(&f, 0, sizeof f);
  { int j; for (j = 0; j < 12; j++) f.roteiro[j * 2] = VS_FALHOU; }
  vazao_agendar(&pl, fila, 12, &ops, &a);
  CONFERIR(a.tentadas == 6 && a.medidas == 0 && a.falhas == 6, "rapido: desiste depois de 6 tentativas");

  // Cancelar no meio: nenhuma fonte comeca depois.
  memset(&f, 0, sizeof f);
  f.cancelarNaChamada = 5;
  memset(&pl, 0, sizeof pl);
  vazao_agendar(&pl, fila, 20, &ops, &a);
  CONFERIR(a.cancelado && f.chamadas == 5 && a.restantes == 15,
           "cancelar: para na fonte em curso, as outras nem comecam");

  // Cancelado antes de comecar.
  memset(&f, 0, sizeof f);
  f.cancelarNaChamada = -1;   // nunca: chamadas >= -1 e sempre verdade
  vazao_agendar(&pl, fila, 5, &ops, &a);
  CONFERIR(a.cancelado && f.chamadas == 0, "cancelar antes da 1a: nenhuma chamada");

  // Orcamento do ciclo: cada fonte custa 1 min, orcamento de 8 min.
  memset(&f, 0, sizeof f);
  f.custoMs = 60000UL;
  pl = (VazPlano){ 0, 0, VAZ_CICLO_ORCAMENTO_MS, 0 };
  vazao_agendar(&pl, fila, 40, &ops, &a);
  CONFERIR(a.semTempo && a.tentadas == 9 && a.restantes == 31,
           "orcamento de 8 min: a fonte que passaria do tempo nem comeca");

  // O rapido so aplica o orcamento depois da 1a medida.
  memset(&f, 0, sizeof f);
  f.custoMs = 50000UL;
  pl = (VazPlano){ 3, 6, 45000UL, 1 };
  vazao_agendar(&pl, fila, 12, &ops, &a);
  CONFERIR(a.medidas == 1 && a.semTempo, "rapido: 45 s depois da 1a medida nao comeca outra");
}

// ---- ranking e suficiencia ----------------------------------------------------
static VazCicloRes res(int addon, int sit, int kbps, int espera) {
  VazCicloRes r;
  memset(&r, 0, sizeof r);
  r.addon = addon;
  r.sit = sit;
  r.esperaMs = espera;
  r.r.medianaKbps = kbps;
  return r;
}

static void ranking(void) {
  VazCicloRes r[8];
  int ordem[8], u;
  r[0] = res(0, VS_DEBRID, 0, 0);
  r[1] = res(0, VS_OK, 8000, 300);
  r[2] = res(1, VS_FALHOU, 0, 0);
  r[3] = res(1, VS_OK, 48000, 900);
  r[4] = res(1, VS_OK, 8000, 100);
  r[5] = res(2, VS_OK, 22000, 50);
  r[6] = res(2, VS_AVISO, 0, 0);
  r[7] = res(0, VS_DEBRID, 0, 0);
  vazao_ordenar(r, 8, ordem);
  CONFERIR(ordem[0] == 3 && ordem[1] == 5 && ordem[2] == 4 && ordem[3] == 1,
           "ranking: vazao decrescente, empate pela menor espera");
  CONFERIR(ordem[4] == 2 && ordem[5] == 6 && ordem[6] == 0 && ordem[7] == 7,
           "ranking: falhas depois das medidas, debrid por ultimo, ordem estavel");

  CONFERIR(vazao_mediana_addon(r, 8, 1, &u) == 28000 && u == 2, "mediana do add-on: das fontes medidas");
  CONFERIR(vazao_mediana_addon(r, 8, 2, &u) == 22000 && u == 1, "mediana do add-on com uma so fonte");
  CONFERIR(vazao_mediana_addon(r, 8, 9, &u) == 0 && u == 0, "add-on sem medida: 0");

  { VazaoResumo x = { 8, 30000, 20000, 15000, 27000 };
    int need4k = vazao_necessario_kbps(2160, 0, 7200);
    CONFERIR(need4k == 25000 && vazao_necessario_kbps(1080, 0, 7200) == 8000 &&
             vazao_necessario_kbps(720, 0, 7200) == 4000 && vazao_necessario_kbps(0, 0, 7200) == 0,
             "bitrate tipico por altura: 4K 25, 1080p 8, 720p 4, sem pista 0");
    CONFERIR(vazao_suficiencia(&x, 8000) == VSU_OK, "otimo cobre o bitrate: toca sem parar");
    CONFERIR(vazao_suficiencia(&x, 16000) == VSU_JUSTO, "so o maximo cobre: pode pausar");
    CONFERIR(vazao_suficiencia(&x, 40000) == VSU_NAO, "nada cobre: insuficiente");
    CONFERIR(vazao_suficiencia(&x, 0) == VSU_SEM_REF, "sem bitrate: sem referencia"); }

  // 57,8 GB de um filme de 2 h ~ 69 Mbps (o tamanho do nome vale mais que a altura).
  { int k = vazao_necessario_kbps(2160, (long)(57.8 * 1024), 7200);
    CONFERIR(k > 68000 && k < 70000, "bitrate pelo tamanho do arquivo: 57,8 GB em 2 h ~ 69 Mbps"); }
}

// ---- redacao ---------------------------------------------------------------
static void redacao(void) {
  char h[96];
  CONFERIR(vazao_host_publico("https://cdn.exemplo.com/d/CHAVE-SECRETA/filme.mkv?token=abc#x", h, sizeof h) &&
           !strcmp(h, "cdn.exemplo.com"), "host publico: sem esquema, caminho, consulta nem fragmento");
  CONFERIR(vazao_host_publico("http://usuario:senha@cdn.exemplo.com:8443/x", h, sizeof h) &&
           !strcmp(h, "cdn.exemplo.com:8443") && !strstr(h, "senha"), "host publico: sem usuario:senha@, com a porta");
  CONFERIR(vazao_host_publico("https://cdn.exemplo.com/a@b/c", h, sizeof h) && !strcmp(h, "cdn.exemplo.com"),
           "host publico: '@' no caminho nao e credencial");
  CONFERIR(!vazao_host_publico("sem esquema", h, sizeof h) && !h[0], "host publico: texto sem :// devolve vazio");
  CONFERIR(vazao_chave("https://a/b") == vazao_chave("https://a/b") &&
           vazao_chave("https://a/b") != vazao_chave("https://a/c") && vazao_chave("") == 0,
           "chave do link: estavel, distingue, vazio = 0");
}

int main(void) {
  selecao();
  agendador();
  ranking();
  redacao();
  if (falhas) { printf("%d falha(s)\n", falhas); return 1; }
  puts("vazao_ciclo: tudo ok");
  return 0;
}
