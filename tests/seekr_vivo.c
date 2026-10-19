// SEEKR CONTRA A API DE VERDADE. Sem SEEKR_API_KEY no ambiente, confere so o
// caminho da chave recusada (uma chave falsa: 401 no /sprites, valid:false no
// /v1/keys/validate). Com a chave, pede o Matrix e baixa/recorta um quadro.
//
//   bash tests/seekr_vivo.sh                       # sem chave
//   SEEKR_API_KEY=... bash tests/seekr_vivo.sh     # com a sua (nunca no log)
#include "../src/seekr.h"
#include "../src/dados.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
void gfx_tex_esquecer(GLuint t) { (void)t; }

static int esperar(int de) {
  int e = de, i;
  for (i = 0; i < 200 && (e = seekr_estado()) == SEEKR_BUSCANDO; i++) usleep(100000);
  return e;
}

int main(void) {
  dados_iniciar("deploy/app/art");
  const char *k = getenv("SEEKR_API_KEY");
  int com = k && *k, v, e;
  if (!com) k = "chave-falsa-de-teste";
  v = seekr_validar(k);
  printf("validar: %d\n", v);
  if (v < 0) { printf("SEM REDE\n"); return 2; }
  if (!com && v != 0) { printf("FALHA: chave falsa aceita\n"); return 1; }
  if (com && v != 1) { printf("FALHA: a sua chave foi recusada\n"); return 1; }
  seekr_definir_chave(k);
  seekr_pedir("tt0133093", 0, 0, 8160000L);   // The Matrix, 2 h 16 min
  e = esperar(SEEKR_BUSCANDO);
  printf("estado: %d\n", e);
  if (!com) {
    if (e != SEEKR_CHAVE_RECUSADA) { printf("FALHA: esperava chave recusada\n"); return 1; }
    printf("seekr_vivo: ok (sem chave: so o caminho da recusa)\n");
    return 0;
  }
  if (e != SEEKR_PRONTO) { printf("FALHA: sem previa para o Matrix\n"); return 1; }
  printf("seekr_vivo: ok (lookup + VTT)\n");
  return 0;
}
