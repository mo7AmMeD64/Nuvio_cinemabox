// BARRA DE PROGRESSO ANIMADA e ONDA DE GRADE (revela.h): a barra cresce do
// valor anterior ate o novo em NV_PROGRESSO_MS, sem animar na primeira vez
// que a chave aparece, e obedece animacoes reduzidas.
//
//   bash tests/revelaprog.sh
#include "../src/revela.h"
#include <stdio.h>
#include <math.h>

static int falhas;
static void perto(const char *o, float v, float esperado) {
  if (fabsf(v - esperado) > 0.01f) {
    printf("FALHOU: %s = %.3f (esperado %.3f)\n", o, v, esperado);
    falhas++;
  }
}

int main(void) {
  RevelaProgresso tab[4];
  SDL_memset(tab, 0, sizeof tab);

  // Primeira vez: nasce assentada.
  perto("primeiro desenho", revela_progresso_em(tab, 4, "tt1", 1, 2, 30, 1000), 30);
  perto("mesmo valor", revela_progresso_em(tab, 4, "tt1", 1, 2, 30, 1100), 30);

  // Valor novo: parte de 30 e chega a 60 em NV_PROGRESSO_MS, ease-out.
  perto("inicio", revela_progresso_em(tab, 4, "tt1", 1, 2, 60, 2000), 30);
  { float meio = revela_progresso_em(tab, 4, "tt1", 1, 2, 60, 2000 + (Uint32)(NV_PROGRESSO_MS / 2));
    // ease-out cubico em t=0.5: 1 - 0.125 = 0.875 do caminho
    perto("meio (ease-out)", meio, 30 + 30 * 0.875f); }
  perto("fim", revela_progresso_em(tab, 4, "tt1", 1, 2, 60, 2000 + (Uint32)NV_PROGRESSO_MS), 60);
  perto("depois", revela_progresso_em(tab, 4, "tt1", 1, 2, 60, 5000), 60);

  // Valor novo no meio da animacao: parte de onde a barra esta.
  revela_progresso_em(tab, 4, "tt1", 1, 2, 80, 6000);
  { float onde = revela_progresso_em(tab, 4, "tt1", 1, 2, 80, 6100);
    float de = revela_progresso_em(tab, 4, "tt1", 1, 2, 90, 6100);
    perto("retoma sem salto", de, onde); }

  // Outra temporada/episodio e outra chave: nao herda.
  perto("outra chave", revela_progresso_em(tab, 4, "tt1", 1, 3, 10, 7000), 10);

  // Tabela cheia despeja o menos usado; a chave despejada volta sem animar.
  revela_progresso_em(tab, 4, "tt2", 0, 0, 5, 8000);
  revela_progresso_em(tab, 4, "tt3", 0, 0, 5, 8001);
  revela_progresso_em(tab, 4, "tt4", 0, 0, 5, 8002);   // despeja tt1:1:2
  perto("despejada volta assentada", revela_progresso_em(tab, 4, "tt1", 1, 2, 20, 8003), 20);

  // Sem imdb: devolve o alvo.
  perto("sem chave", revela_progresso_em(tab, 4, "", 1, 1, 42, 9000), 42);

  // Animacoes reduzidas: vai direto ao alvo.
  anim_politica_reduzida = 1;
  perto("reduzida", revela_progresso_em(tab, 4, "tt2", 0, 0, 70, 9100), 70);
  if (!revela_onda_fim(9000, 9001)) { printf("FALHOU: onda reduzida\n"); falhas++; }
  anim_politica_reduzida = 0;

  // Onda: atraso cresce por coluna e fileira e para no teto.
  perto("onda 0,0", revela_onda_atraso(0, 0), 0);
  perto("onda 2,1", revela_onda_atraso(2, 1), 2 * NV_ENTRA_PASSO_MS + NV_ENTRA_FIL_MS);
  perto("onda teto", revela_onda_atraso(99, 99),
        NV_ENTRA_MAX_COL * NV_ENTRA_PASSO_MS + NV_ONDA_MAX_FIL * NV_ENTRA_FIL_MS);
  perto("onda negativa", revela_onda_atraso(-3, -1), 0);
  if (revela_onda_fim(1000, 1100)) { printf("FALHOU: onda terminou cedo\n"); falhas++; }
  if (!revela_onda_fim(1000, 3000)) { printf("FALHOU: onda nao terminou\n"); falhas++; }

  if (falhas) return 1;
  printf("revelaprog: ok\n");
  return 0;
}
