// Avisos dispensados — ver avisodisp.h.
#include "avisodisp.h"
#include "dados.h"
#include "perfis.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define AVD_SESSAO 96

typedef struct { int perfil; char chave[AVD_CHAVE]; } AvdItem;

static AvdItem disp[AVD_MAX];
static int     nDisp = -1;          // -1 = arquivo ainda nao lido
static AvdItem sessao[AVD_SESSAO];
static int     nSessao;
static pthread_mutex_t trava = PTHREAD_MUTEX_INITIALIZER;

// A chave vai numa linha: quebra de linha viraria duas.
static void copiar(char *dst, const char *s) {
  size_t i;
  for (i = 0; s[i] && i + 1 < AVD_CHAVE; i++) dst[i] = (s[i] == '\n' || s[i] == '\r') ? ' ' : s[i];
  dst[i] = 0;
}

// "<perfil> <chave ate o fim da linha>": a chave do crash tem espaco ("crash:2026-09-19 18:57").
static void carregar(void) {
  char *s, *l;
  nDisp = 0;
  s = dados_ler(AVD_ARQ);
  if (!s) return;
  l = s;
  while (*l && nDisp < AVD_MAX) {
    char *fim = strchr(l, '\n'), *esp;
    if (fim) *fim = 0;
    esp = strchr(l, ' ');
    if (esp && esp[1] && atoi(l) > 0) {
      disp[nDisp].perfil = atoi(l);
      copiar(disp[nDisp].chave, esp + 1);
      nDisp++;
    }
    if (!fim) break;
    l = fim + 1;
  }
  free(s);
}

static void gravar(void) {
  char buf[AVD_MAX * (AVD_CHAVE + 8)], *w = buf;
  int i;
  buf[0] = 0;
  for (i = 0; i < nDisp; i++)
    w += snprintf(w, sizeof buf - (size_t)(w - buf), "%d %s\n", disp[i].perfil, disp[i].chave);
  dados_gravar(AVD_ARQ, buf);
}

static int achar(const AvdItem *v, int n, int pf, const char *chave) {
  int i;
  char c[AVD_CHAVE];
  copiar(c, chave);
  for (i = 0; i < n; i++) if (v[i].perfil == pf && !strcmp(v[i].chave, c)) return i;
  return -1;
}

static int perfil(void) { int p = perfis_ativo(); return p > 0 ? p : 1; }

int avisodisp_tem(const char *chave) {
  int r;
  if (!chave || !chave[0]) return 0;
  pthread_mutex_lock(&trava);
  if (nDisp < 0) carregar();
  r = achar(disp, nDisp, perfil(), chave) >= 0;
  pthread_mutex_unlock(&trava);
  return r;
}

void avisodisp_por(const char *chave) {
  int pf;
  if (!chave || !chave[0]) return;
  pthread_mutex_lock(&trava);
  if (nDisp < 0) carregar();
  pf = perfil();
  if (achar(disp, nDisp, pf, chave) < 0) {
    if (nDisp == AVD_MAX) { memmove(&disp[0], &disp[1], sizeof disp[0] * (AVD_MAX - 1)); nDisp--; }
    disp[nDisp].perfil = pf;
    copiar(disp[nDisp].chave, chave);
    nDisp++;
    gravar();
  }
  pthread_mutex_unlock(&trava);
}

int avisodisp_sessao_tem(const char *chave) {
  int r;
  if (!chave || !chave[0]) return 0;
  pthread_mutex_lock(&trava);
  r = achar(sessao, nSessao, perfil(), chave) >= 0;
  pthread_mutex_unlock(&trava);
  return r;
}

void avisodisp_sessao_por(const char *chave) {
  int pf;
  if (!chave || !chave[0]) return;
  pthread_mutex_lock(&trava);
  pf = perfil();
  if (achar(sessao, nSessao, pf, chave) < 0) {
    if (nSessao == AVD_SESSAO) { memmove(&sessao[0], &sessao[1], sizeof sessao[0] * (AVD_SESSAO - 1)); nSessao--; }
    sessao[nSessao].perfil = pf;
    copiar(sessao[nSessao].chave, chave);
    nSessao++;
  }
  pthread_mutex_unlock(&trava);
}

void avisodisp_esquecer(void) {
  pthread_mutex_lock(&trava);
  nDisp = -1;
  nSessao = 0;
  pthread_mutex_unlock(&trava);
}
