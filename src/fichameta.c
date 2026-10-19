#include "fichameta.h"
#include "dados.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define FM_VAGAS 64
#define FM_MAX_BYTES (256 * 1024)

static void nomeVaga(char *dst, size_t n, const char *chave) {
  unsigned h = 2166136261u;
  for (; *chave; chave++) { h ^= (unsigned char)*chave; h *= 16777619u; }
  snprintf(dst, n, "ficha-%02u.json", h % FM_VAGAS);
}

// Formato: "<epoch>\n<chave>\n<corpo>".
char *fichameta_ler(const char *chave, long ttlSeg) {
  char nome[32], *raw, *nl1, *nl2, *r;
  long quando;
  nomeVaga(nome, sizeof nome, chave);
  raw = dados_ler(nome);
  if (!raw) return NULL;
  nl1 = strchr(raw, '\n');
  nl2 = nl1 ? strchr(nl1 + 1, '\n') : NULL;
  if (!nl2) { free(raw); return NULL; }
  *nl1 = 0; *nl2 = 0;
  quando = atol(raw);
  if (strcmp(nl1 + 1, chave) || time(NULL) - quando > ttlSeg || quando > time(NULL) + 60) {
    free(raw); return NULL;
  }
  r = strdup(nl2 + 1);
  free(raw);
  return r;
}

void fichameta_gravar(const char *chave, const char *corpo) {
  char nome[32], *buf;
  size_t n;
  if (!corpo || !*corpo || !chave) return;
  n = strlen(corpo);
  if (n > FM_MAX_BYTES) return;
  buf = malloc(n + strlen(chave) + 32);
  if (!buf) return;
  sprintf(buf, "%ld\n%s\n%s", (long)time(NULL), chave, corpo);
  nomeVaga(nome, sizeof nome, chave);
  dados_gravar_leve(nome, buf);
  free(buf);
}
