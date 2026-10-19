// Ver psparede.h.
#include "psparede.h"
#include "perfis.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef PSPAREDE_SO_PURO
#include "progresso.h"
#include "catalogo.h"
#include "dados.h"
#endif

#define ARQ "perfilparede.txt"

int psparede_juntar(const char *const *cand, int n, char saida[][PSPAREDE_URL], int max) {
  int i, j, k = 0;
  for (i = 0; i < n && k < max; i++) {
    const char *u = cand[i];
    int repetida = 0;
    if (!u || !u[0] || strlen(u) >= PSPAREDE_URL || strchr(u, '\t') || strchr(u, '\n')) continue;
    for (j = 0; j < k; j++) if (!strcmp(saida[j], u)) { repetida = 1; break; }
    if (repetida) continue;
    snprintf(saida[k++], PSPAREDE_URL, "%s", u);
  }
  return k;
}

#ifndef PSPAREDE_SO_PURO

typedef struct {
  int  perfil, n;
  char url[PSPAREDE_MAX][PSPAREDE_URL];
} Parede;

static Parede paredes[CONTA_PERFIL_MAX];
static int nParedes, carregado;
static CatItem rascunho;   // static: CatItem e grande demais para a pilha

static Parede *achar(int perfil) {
  int i;
  for (i = 0; i < nParedes; i++) if (paredes[i].perfil == perfil) return &paredes[i];
  return NULL;
}

static void carregar(void) {
  char *buf, *ctx = NULL, *linha;
  if (carregado) return;
  carregado = 1;
  nParedes = 0;
  buf = dados_ler(ARQ);
  if (!buf) return;
  for (linha = strtok_r(buf, "\n", &ctx); linha; linha = strtok_r(NULL, "\n", &ctx)) {
    char *tab = strchr(linha, '\t');
    int perfil;
    Parede *p;
    if (!tab) continue;
    *tab = 0;
    perfil = atoi(linha);
    if (perfil <= 0 || !tab[1] || strlen(tab + 1) >= PSPAREDE_URL) continue;
    p = achar(perfil);
    if (!p) {
      if (nParedes >= CONTA_PERFIL_MAX) continue;
      p = &paredes[nParedes++];
      memset(p, 0, sizeof *p);
      p->perfil = perfil;
    }
    if (p->n < PSPAREDE_MAX) snprintf(p->url[p->n++], PSPAREDE_URL, "%s", tab + 1);
  }
  free(buf);
}

static void gravar(void) {
  size_t cap = (size_t)nParedes * PSPAREDE_MAX * (PSPAREDE_URL + 8) + 16, n = 0;
  char *b = malloc(cap);
  int i, k;
  if (!b) return;
  b[0] = 0;
  for (i = 0; i < nParedes; i++)
    for (k = 0; k < paredes[i].n; k++)
      n += (size_t)snprintf(b + n, cap - n, "%d\t%s\n", paredes[i].perfil, paredes[i].url[k]);
  dados_gravar(ARQ, b);
  free(b);
}

void psparede_registrar(void) {
  static ProgRegistro regs[PROG_MAX];
  const char *cand[PSPAREDE_MAX * 4];
  static char posters[PSPAREDE_MAX * 2][PSPAREDE_URL];
  char nova[PSPAREDE_MAX][PSPAREDE_URL];
  const ContaPerfil *ativo = perfis_item_ativo();
  int perfil = perfis_ativo(), nr, i, nc = 0, np = 0, k;
  Parede *p;
  carregar();
  p = achar(perfil);
  // PIN: o que a pessoa viu e dela. A parede some, e nao fica uma velha.
  if (!ativo || ativo->temPin) {
    if (p) { *p = paredes[--nParedes]; gravar(); }
    return;
  }
  if (cat_n() <= 0) return;   // catalogo ainda vazio: nao apaga a parede boa
  // 1) O que ele assistiu, do mais recente para o mais antigo.
  nr = prog_ler(regs, PROG_MAX);
  for (i = 0; i < nr && np < PSPAREDE_MAX * 2; i++) {
    memset(&rascunho, 0, sizeof rascunho);
    if (cat_copiar_por_id(regs[i].contentId, regs[i].tipo, &rascunho) && rascunho.poster[0] &&
        strlen(rascunho.poster) < PSPAREDE_URL) {
      snprintf(posters[np], PSPAREDE_URL, "%s", rascunho.poster);
      cand[nc++] = posters[np++];
    }
  }
  // 2) A lista dele, 3) o comeco da Home dele.
  for (i = 0; i < cat_n() && nc < PSPAREDE_MAX * 4; i++) {
    const CatItem *c = cat_item(i);
    if (c && c->naLista && c->poster[0]) cand[nc++] = c->poster;
  }
  for (i = 0; i < cat_n() && nc < PSPAREDE_MAX * 4; i++) {
    const CatItem *c = cat_item(i);
    if (c && c->poster[0]) cand[nc++] = c->poster;
  }
  k = psparede_juntar(cand, nc, nova, PSPAREDE_MAX);
  if (k <= 0) return;
  if (p && p->n == k) {
    int igual = 1;
    for (i = 0; i < k && igual; i++) igual = !strcmp(p->url[i], nova[i]);
    if (igual) return;
  }
  if (!p) {
    if (nParedes >= CONTA_PERFIL_MAX) return;
    p = &paredes[nParedes++];
  }
  memset(p, 0, sizeof *p);
  p->perfil = perfil;
  p->n = k;
  for (i = 0; i < k; i++) memcpy(p->url[i], nova[i], PSPAREDE_URL);
  gravar();
}

int psparede_n(int indice) {
  const Parede *p;
  carregar();
  p = achar(indice);
  return p ? p->n : 0;
}

const char *psparede_url(int indice, int k) {
  const Parede *p;
  carregar();
  p = achar(indice);
  return p && k >= 0 && k < p->n ? p->url[k] : NULL;
}

void psparede_esquecer(void) {
  nParedes = 0;
  carregado = 1;
  dados_apagar(ARQ);
}

#endif
