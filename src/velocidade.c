// Velocidade de reproducao — ver velocidade.h.
#include "velocidade.h"
#include <stdio.h>
#include <string.h>

const int VEL_LISTA[VEL_N] = { 75, 100, 125, 150, 175, 200 };

int vel_indice(int cent) {
  int i, melhor = 1, dMelhor = 1 << 30;
  for (i = 0; i < VEL_N; i++) {
    int d = VEL_LISTA[i] - cent;
    if (d < 0) d = -d;
    if (d < dMelhor) { dMelhor = d; melhor = i; }
  }
  return melhor;
}

int vel_passo(int cent, int dir) {
  int i = vel_indice(cent);
  if (dir > 0 && i + 1 < VEL_N) i++;
  else if (dir < 0 && i > 0) i--;
  return VEL_LISTA[i];
}

double vel_tempo_real(double segMidia, int cent) {
  if (cent <= 0) cent = VEL_NORMAL;
  return segMidia * (double)VEL_NORMAL / (double)cent;
}

void vel_rotulo(char *b, size_t n, int cent) {
  int inteiro = cent / 100, resto = cent % 100;
  if (!b || !n) return;
  if (cent <= 0) { snprintf(b, n, "1x"); return; }
  if (!resto) snprintf(b, n, "%dx", inteiro);
  else if (resto % 10 == 0) snprintf(b, n, "%d.%dx", inteiro, resto / 10);
  else snprintf(b, n, "%d.%02dx", inteiro, resto);
}

void vel_log(int cent, const char *estado) {
  printf("[player] velocidade %d.%02dx (%s)\n", cent / 100, cent % 100, estado ? estado : "?");
  fflush(stdout);
}

void velmed_zerar(VelMedidor *m) {
  memset(m, 0, sizeof *m);
  m->pedida = m->efetiva = VEL_NORMAL;
}

void velmed_pedir(VelMedidor *m, int cent) {
  if (cent <= 0) cent = VEL_NORMAL;
  if (cent == m->pedida) return;
  m->pedida = cent;
  m->medindo = 0; m->uns = 0;
  // Voltar a 1x nao precisa de prova: e o estado em que o pipeline nasce, e
  // contar 1x a mais por 5 s seria pior que acertar na hora.
  if (cent == VEL_NORMAL) m->efetiva = VEL_NORMAL;
}

static int perto(double taxa, double alvo, double tol) {
  double d = taxa - alvo;
  if (d < 0) d = -d;
  return d <= tol * alvo;
}

int velmed_passo(VelMedidor *m, double agora, double pos, int valido) {
  double taxa, alvo;
  if (!valido) { m->medindo = 0; return VELMED_NADA; }
  if (m->medindo) {
    double dt = agora - m->tUlt, dp = pos - m->pUlt;
    // Salto (busca que nao passou por `valido`, troca de fonte): a janela recomeca.
    if (dp < -0.5 || dp > dt * 3.0 + 1.0) m->medindo = 0;
  }
  if (!m->medindo) {
    m->medindo = 1; m->t0 = m->tUlt = agora; m->p0 = m->pUlt = pos;
    return VELMED_NADA;
  }
  m->tUlt = agora; m->pUlt = pos;
  if (agora - m->t0 < VELMED_JANELA_S) return VELMED_NADA;
  taxa = (pos - m->p0) / (agora - m->t0);
  m->t0 = agora; m->p0 = pos;   // monitora sempre: a janela seguinte comeca aqui
  if (m->pedida == VEL_NORMAL) { m->uns = 0; return VELMED_NADA; }
  alvo = m->pedida / 100.0;
  if (perto(taxa, alvo, 0.12)) {
    m->uns = 0;
    if (m->efetiva != m->pedida) { m->efetiva = m->pedida; return VELMED_CONFIRMOU; }
    return VELMED_NADA;
  }
  if (perto(taxa, 1.0, 0.08)) {
    if (++m->uns >= 2) { m->uns = 0; m->efetiva = VEL_NORMAL; return VELMED_NAO_ANDOU; }
    return VELMED_NADA;
  }
  m->uns = 0;   // nem uma nem outra: travou no meio, nao conta
  return VELMED_NADA;
}
