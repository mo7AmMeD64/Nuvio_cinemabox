// "A seguir" do Trakt (issue #213): serie marcada inteira de uma vez nao vira
// "a seguir" de um episodio do meio, e o next_episode do Trakt manda.
#include "traktult.h"
#include <stdio.h>

static int falhas;
#define CONFERE(c, ...) do { if (!(c)) { printf("FALHOU: " __VA_ARGS__); printf("\n"); falhas++; } } while (0)

int main(void) {
  TkUltimo v[4];
  int n = 0, t = -1, e = -1;
  const long long marcou = 1790924940000LL, antes = 1790885880000LL;

  // O LOG 16676/16678: a serie inteira marcada no mesmo instante, e o
  // historico devolvendo T2E1 antes de T2E9. Antes ficava T2E1 (primeira
  // ocorrencia) e o card dizia "a seguir T2E2" de uma serie vista.
  tk_ult_anotar(v, &n, 4, "tt2661044", 2, 1, marcou);
  tk_ult_anotar(v, &n, 4, "tt2661044", 2, 9, marcou);
  tk_ult_anotar(v, &n, 4, "tt2661044", 1, 10, marcou);
  tk_ult_anotar(v, &n, 4, "tt2661044", 2, 4, marcou);
  tk_ult_anotar(v, &n, 4, "tt2661044", 0, 3, marcou);    // especial empatado
  tk_ult_anotar(v, &n, 4, "tt2661044", 2, 12, antes);    // mais velho: passado
  CONFERE(n == 1, "uma serie, %d entradas", n);
  CONFERE(v[0].temporada == 2 && v[0].episodio == 9 && v[0].quandoMs == marcou,
          "empate: esperava T2E9, veio T%dE%d", v[0].temporada, v[0].episodio);

  // SEM EMPATE a ordem do historico decide, como sempre: reassistiu o T1E3
  // depois do T1E5 -> o ultimo visto e o T1E3.
  tk_ult_anotar(v, &n, 4, "tt0000002", 1, 3, 2000);
  tk_ult_anotar(v, &n, 4, "tt0000002", 1, 5, 1000);
  CONFERE(n == 2 && v[1].episodio == 3, "sem empate: T%dE%d", v[1].temporada, v[1].episodio);

  // Lista cheia nao estoura.
  tk_ult_anotar(v, &n, 4, "tt3", 1, 1, 1);
  tk_ult_anotar(v, &n, 4, "tt4", 1, 1, 1);
  CONFERE(!tk_ult_anotar(v, &n, 4, "tt5", 1, 1, 1) && n == 4, "cheia: n=%d", n);

  // next_episode do Trakt.
  CONFERE(tk_prog_proximo("{\"aired\":50,\"completed\":50,\"seasons\":[],\"next_episode\":null,"
                          "\"last_episode\":{\"season\":4,\"number\":1}}", &t, &e) == 0,
          "next_episode null = acabou");
  CONFERE(tk_prog_proximo("{\"aired\":12,\"completed\":3,\"next_episode\":{\"season\":2,"
                          "\"number\":4,\"title\":\"X\",\"ids\":{\"trakt\":9,\"tvdb\":1}}}",
                          &t, &e) == 1 && t == 2 && e == 4, "proximo T2E4, veio T%dE%d", t, e);
  CONFERE(tk_prog_proximo("{\"next_episode\" : { \"season\" : 1 , \"number\" : 7 }}", &t, &e) == 1 &&
          t == 1 && e == 7, "com espacos: T%dE%d", t, e);
  CONFERE(tk_prog_proximo(NULL, &t, &e) == -1, "sem corpo");
  CONFERE(tk_prog_proximo("{\"error\":\"not found\"}", &t, &e) == -1, "corpo sem next_episode");
  CONFERE(tk_prog_proximo("{\"next_episode\":{\"title\":\"x\"},\"season\":3,\"number\":2}", &t, &e) == -1,
          "season/number fora do objeto nao vale");

  if (falhas) { printf("traktult: %d falha(s)\n", falhas); return 1; }
  printf("traktult: ok\n");
  return 0;
}
