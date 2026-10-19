// A fila de primeira vez a partir da 1.8.0 — ver novidadesfila.h.
#include "novidadesfila.h"
#include "dados.h"
#include <stdlib.h>

// Cada marca com o conteudo que o proprio cartao grava (o da apresentacao do
// diagnostico e "versao=1"; os outros, "1").
static const struct { const char *arq, *conteudo; } ANTIGAS[] = {
  { "novidades-guia.txt", "1\n" },              // novidades.c (Guia de TV)
  { "novidades-11.txt", "1\n" },                // novidades11.c
  { "novidades-12.txt", "1\n" },
  { "novidades-13.txt", "1\n" },
  { "novidades-131.txt", "1\n" },
  { "novidades-132.txt", "1\n" },
  { "novidades-133.txt", "1\n" },
  { "novidades-134.txt", "1\n" },
  { "novidades-139.txt", "1\n" },
  { "novidades-14-celebracao.txt", "1\n" },     // novidades1312.c
  { "novidades-142.txt", "1\n" },
  { "novidades-150.txt", "1\n" },               // novidades148.c (saiu como 1.5.0)
  { "novidades-152-ui.txt", "1\n" },            // novidades151.c
  { "novidades-160-ui.txt", "1\n" },            // novidades160.c
  { "novidades-170-ui.txt", "1\n" },            // novidades170.c
  { "novidades-172-ui.txt", "1\n" },            // novidades172.c (published as 1.7.3)
  { "novidades-174-ui.txt", "1\n" },            // novidades174.c
  { "salvos-intro.txt", "1\n" },                // salvosintro.c
  { "aviso-log.txt", "1\n" },                   // registro.c
  { "recintro-social.txt", "1\n" },             // recintro.c
  { "diagnostico-otimizacao-intro.cfg", "versao=1\n" },   // diagnostico.c
};
#define NF_N ((int)(sizeof ANTIGAS / sizeof *ANTIGAS))

int novidadesfila_n(void) { return NF_N; }
const char *novidadesfila_arquivo(int i) { return i >= 0 && i < NF_N ? ANTIGAS[i].arq : NULL; }

static int existe(const char *arq) {
  char *s = dados_ler(arq);
  if (!s) return 0;
  free(s);
  return 1;
}

int novidadesfila_preparar(void) {
  int i, alguma = 0;
  if (existe(NF_ARQ_180)) return NF_NADA;
  for (i = 0; i < NF_N; i++)
    if (existe(ANTIGAS[i].arq)) alguma = 1;
    else dados_gravar(ANTIGAS[i].arq, ANTIGAS[i].conteudo);
  return alguma ? NF_ATUALIZOU : NF_NOVA;
}
