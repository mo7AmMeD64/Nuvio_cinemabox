// cw_frente_compor (cwfrente.h): o titulo que saiu do player no meio vai para a
// frente do "Continuar assistindo" sem rede, e uma fileira que ja era essa nao
// e republicada (a home nao remonta).
#include "cwfrente.h"
#include <assert.h>
#include <stdio.h>

static CatItem it(const char *imdb, int p) {
  CatItem c;
  memset(&c, 0, sizeof c);
  snprintf(c.imdb, sizeof c.imdb, "%s", imdb);
  c.progresso = p;
  return c;
}

int main(void) {
  static CatItem atual[12], saida[12];
  CatItem novo;
  int n, mudou;

  // 1. Fileira vazia: o titulo entra sozinho.
  novo = it("tt1", 40);
  n = cw_frente_compor(atual, 0, &novo, saida, 12, &mudou);
  assert(n == 1 && mudou && !strcmp(saida[0].imdb, "tt1"));

  // 2. Nao estava: entra na frente, as outras andam uma casa.
  atual[0] = it("tt2", 10); atual[1] = it("tt3", 20);
  n = cw_frente_compor(atual, 2, &novo, saida, 12, &mudou);
  assert(n == 3 && mudou);
  assert(!strcmp(saida[0].imdb, "tt1") && !strcmp(saida[1].imdb, "tt2") &&
         !strcmp(saida[2].imdb, "tt3"));

  // 3. Estava no meio: vai para a frente, sem duplicar.
  atual[0] = it("tt2", 10); atual[1] = it("tt1", 30); atual[2] = it("tt3", 20);
  n = cw_frente_compor(atual, 3, &novo, saida, 12, &mudou);
  assert(n == 3 && mudou && !strcmp(saida[0].imdb, "tt1") && saida[0].progresso == 40 &&
         !strcmp(saida[1].imdb, "tt2") && !strcmp(saida[2].imdb, "tt3"));

  // 4. Ja na frente com a mesma barra: nada a publicar.
  atual[0] = it("tt1", 40); atual[1] = it("tt2", 10);
  n = cw_frente_compor(atual, 2, &novo, saida, 12, &mudou);
  assert(n == 2 && !mudou);
  // ...com a barra nova: publica (o card mostra o progresso de agora).
  atual[0].progresso = 35;
  cw_frente_compor(atual, 2, &novo, saida, 12, &mudou);
  assert(mudou);

  // 5. Serie: o card de outro episodio da mesma obra sai; "kitsu:41370" e um
  // id inteiro, nao o prefixo "kitsu".
  novo = it("tt9:1:4", 12);
  atual[0] = it("kitsu:41370", 50); atual[1] = it("tt9:1:3", 80); atual[2] = it("kitsu:41371", 5);
  n = cw_frente_compor(atual, 3, &novo, saida, 12, &mudou);
  assert(n == 3 && mudou && !strcmp(saida[0].imdb, "tt9:1:4") &&
         !strcmp(saida[1].imdb, "kitsu:41370") && !strcmp(saida[2].imdb, "kitsu:41371"));

  // 6. Fileira cheia: o mais antigo (o ultimo) sai, como no corte de montarContinuar.
  { int i;
    for (i = 0; i < 12; i++) { char id[16]; snprintf(id, sizeof id, "tt%d", 100 + i); atual[i] = it(id, 10); }
    novo = it("tt1", 40);
    n = cw_frente_compor(atual, 12, &novo, saida, 12, &mudou);
    assert(n == 12 && mudou && !strcmp(saida[0].imdb, "tt1") && !strcmp(saida[11].imdb, "tt110")); }

  puts("PASS: Continuar assistindo otimista na saida do player");
  return 0;
}
