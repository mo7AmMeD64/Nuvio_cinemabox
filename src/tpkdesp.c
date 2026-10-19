#include "tpkdesp.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>

static char arq[640];
static int estado;            // 1 = "oculto" gravado por nos
static pthread_mutex_t trava = PTHREAD_MUTEX_INITIALIZER;

void tpkdesp_iniciar(const char *dirDados) {
  pthread_mutex_lock(&trava);
  if (dirDados && dirDados[0]) snprintf(arq, sizeof arq, "%s/despedida.txt", dirDados);
  else arq[0] = 0;
  estado = 0;
  pthread_mutex_unlock(&trava);
}

// Linhas do host (tizen-tpk/Program.cs): Etapa manda "[etapa] host note ..."
// (com teto de 400 por sessao) e Janela manda "[janela] ..." (sem teto).
int tpkdesp_classificar_linha(const char *l) {
  if (!l) return 0;
  if (strstr(l, "[janela] principal visivel=False") ||
      strstr(l, "[janela] saida:") ||
      strstr(l, "host note pause") ||
      strstr(l, "host note terminate")) return 1;
  if (strstr(l, "[janela] principal visivel=True") ||
      strstr(l, "host note resume")) return -1;
  return 0;
}

int tpkdesp_classificar_tecla(const char *nome) {
  if (!nome || !nome[0]) return 0;
  if (!strcmp(nome, "XF86Exit") || !strcmp(nome, "XF86PowerOff")) return 1;
  return -1;
}

static void aplicar(int v) {
  char b[8] = "";
  FILE *f;
  if (!v) return;
  pthread_mutex_lock(&trava);
  if (!arq[0] || (v > 0) == (estado == 1)) { pthread_mutex_unlock(&trava); return; }
  f = fopen(arq, "rb");
  if (f) { size_t r = fread(b, 1, 3, f); b[r] = 0; fclose(f); }
  if (!strncmp(b, "fim", 3)) { pthread_mutex_unlock(&trava); return; }
  if (v > 0) {
    f = fopen(arq, "wb");
    if (f) { fputs("oculto\n", f); fflush(f); fclose(f); estado = 1; }
  } else {
    if (!strncmp(b, "ocu", 3)) remove(arq);
    estado = 0;
  }
  pthread_mutex_unlock(&trava);
}

void tpkdesp_linha(const char *linha) { aplicar(tpkdesp_classificar_linha(linha)); }
void tpkdesp_tecla(const char *nome) { aplicar(tpkdesp_classificar_tecla(nome)); }
