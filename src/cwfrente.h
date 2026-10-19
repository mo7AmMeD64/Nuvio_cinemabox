// "CONTINUAR ASSISTINDO" OTIMISTA NA SAIDA DO PLAYER (pedido do dono, 03/10:
// "se o filme nao ta no continue watching ele so entra depois que ele sair da
// ilha"). O titulo que acabou de sair no meio vai para a FRENTE da fileira ja
// no quadro em que a home reaparece, por baixo do voo ate a ilha — e a mesma
// posicao que montarContinuar da a ele (o instante mais novo ganha), entao a
// refacao que vem depois (fecharSessao -> desc_refazer_continuar) republica a
// mesma ordem e a home nao remonta nada (hash por identidade, home.c).
//
// Composicao pura, sem trava nem catalogo, para a regressao (tests/cwfrente.c).
#ifndef NV_CWFRENTE_H
#define NV_CWFRENTE_H
#include "catalogo.h"
#include "idbase.h"
#include <string.h>

// `novo` na frente, as outras obras na ordem em que estavam (a mesma obra em
// outro episodio sai: uma serie entra uma vez), ate `max`. *mudou = 0 quando a
// fileira ja era essa (mesma ordem e mesma barra na frente): nada a publicar.
static inline int cw_frente_compor(const CatItem *atual, int n, const CatItem *novo,
                                   CatItem *saida, int max, int *mudou) {
  size_t L = idbase_len(novo->imdb);
  int i, k = 0;
  *mudou = 0;
  if (max < 1 || !novo->imdb[0]) return 0;
  saida[k++] = *novo;
  for (i = 0; i < n && k < max; i++) {
    if (idbase_len(atual[i].imdb) == L && !strncmp(atual[i].imdb, novo->imdb, L)) continue;
    saida[k++] = atual[i];
  }
  if (k != n || atual[0].progresso != novo->progresso) *mudou = 1;
  for (i = 0; i < k && !*mudou; i++)
    if (strcmp(saida[i].imdb, atual[i].imdb)) *mudou = 1;
  return k;
}
#endif
