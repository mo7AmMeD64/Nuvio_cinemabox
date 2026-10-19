// #190 (Owlphibia29, LG 65UT73006LA): "depois de um tempo o titulo e a lista
// de episodios trocam por outro (Attack on Titan vira Knights of Guinevere na
// tela de pausa e no menu de episodios do player)".
//
// A causa medida no log dela: "[cat] continuar assistindo refeita" roda com o
// player aberto (logo depois do loadCompleted). cat_trocar_continuar reordena
// a fileira do topo e desliza o resto, e quem guardava o INDICE passava a ler
// o vizinho. Este teste republica o catalogo com o player aberto, das tres
// formas que a descoberta usa, e confere que tudo que o player mostra continua
// sendo do titulo que esta tocando.
#include "catalogo.h"
#include "player.h"
#include "pausao.h"
#include "posplay.h"
#include "episodios.h"
#include <SDL2/SDL.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define AOT "tt2560140"
#define KOG "tt35459966"

static CatItem item(const char *imdb, const char *titulo, const char *tipo) {
  CatItem c;
  memset(&c, 0, sizeof c);
  snprintf(c.imdb, sizeof c.imdb, "%s", imdb);
  snprintf(c.titulo, sizeof c.titulo, "%s", titulo);
  snprintf(c.tipo, sizeof c.tipo, "%s", tipo);
  return c;
}

static CatFileira fileira(const char *chave, int ini, int n) {
  CatFileira f;
  memset(&f, 0, sizeof f);
  snprintf(f.chave, sizeof f.chave, "%s", chave);
  snprintf(f.titulo, sizeof f.titulo, "%s", chave);
  f.ini = ini; f.n = n;
  return f;
}

static void episodios(int idx, const char *prefixo, int qtd) {
  CatEp v[8];
  int i;
  memset(v, 0, sizeof v);
  for (i = 0; i < qtd; i++) {
    v[i].temporada = 1; v[i].episodio = i + 1;
    snprintf(v[i].nome, sizeof v[i].nome, "%s %d", prefixo, i + 1);
  }
  cat_definir_episodios(idx, v, qtd);
}

static const char *titulo(int idx) {
  const CatItem *c = idx >= 0 ? cat_item(idx) : NULL;
  return c ? c->titulo : "(nenhum)";
}

// Tudo que o player mostra, pelo caminho que cada tela usa.
static void conferir(const char *quando) {
  int ip, ipa, ipp, iep;
  player_atualizar(0.016f, SDL_GetTicks());
  episodios_atualizar(0.016f);
  ip = player_indice(); ipa = pausao_indice(); ipp = posplay_indice(); iep = episodios_titulo();
  printf("  %-38s player=%d(%s) pausa=%d(%s) aseguir=%d(%s) folha=%d(%s)\n", quando,
         ip, titulo(ip), ipa, titulo(ipa), ipp, titulo(ipp), iep, titulo(iep));
  assert(!strcmp(titulo(ip), "Attack on Titan"));
  assert(!strcmp(titulo(ipa), "Attack on Titan"));
  assert(!strcmp(titulo(ipp), "Attack on Titan"));
  assert(!strcmp(titulo(iep), "Attack on Titan"));
  // A linha "T1E3 · nome" do OSD sai da lista do titulo que toca.
  assert(strstr(player_linha_episodio(), "Titan 3"));
  // E a folha de episodios lista os episodios DELE.
  assert(cat_n_episodios(iep) == 5);
  assert(!strncmp(cat_episodio(iep, 0)->nome, "Titan", 5));
}

int main(void) {
  CatItem v[6];
  CatFileira f[2];
  int aot;

  // Continuar assistindo = [Silo, AoT(card do CW), KoG]; fileira de baixo
  // = [Outro, AoT, Filme]. O player abre na copia de baixo (indice 4), a que
  // o detalhe abriu e que tem os episodios.
  v[0] = item("tt14688458", "Silo", "series");
  v[1] = item(AOT ":3:16", "Attack on Titan", "series");
  v[2] = item(KOG ":1:2", "Knights of Guinevere", "series");
  v[3] = item("tt0000003", "Outro", "series");
  v[4] = item(AOT, "Attack on Titan", "series");
  v[5] = item("tt0000005", "Filme", "movie");
  f[0] = fileira("continue_watching", 0, 3);
  f[1] = fileira("addon_series_top", 3, 3);
  cat_definir_tudo(v, 6, f, 2);
  episodios(4, "Titan", 5);
  episodios(2, "Guinevere", 2);

  player_abrir(4, NULL);
  player_definir_episodio(1, 3);
  episodios_abrir(player_indice(), 1, 3);   // o que o botao Episodios faz
  conferir("aberto");

  // 1) A REFACAO DE "CONTINUAR ASSISTINDO", o caso do log: a fileira ganha um
  //    card e reordena. Tudo desliza uma posicao — o indice 4 da abertura
  //    passa a ser o vizinho de cima (o AoT foi para 5).
  { CatItem cw[4];
    cw[0] = item("tt0000009", "Novo", "series");
    cw[1] = v[0]; cw[2] = v[1]; cw[3] = v[2];
    cat_trocar_continuar(cw, 4); }
  assert(strcmp(titulo(4), "Attack on Titan"));   // o indice cru ja e outro
  // A refacao zera TODAS as faixas de episodio; a lista volta pelo fio de
  // episodios (aqui, na mao, no indice que o player diz ser o dele).
  aot = player_indice();
  assert(!strcmp(titulo(aot), "Attack on Titan"));
  episodios(aot, "Titan", 5);
  episodios(3, "Guinevere", 2);                     // o KoG tambem recarrega
  conferir("apos refacao do Continuar");

  // 2) Reordenacao sem card novo: o KoG cai exatamente na posicao em que o
  //    player estava (o "Knights of Guinevere no lugar" do relato).
  { CatItem cw[4];
    cw[0] = item("tt0000009", "Novo", "series");
    cw[1] = v[0]; cw[2] = v[2]; cw[3] = v[1];
    cat_trocar_continuar(cw, 4); }
  assert(!strcmp(titulo(2), "Knights of Guinevere"));   // onde o player estava
  aot = player_indice();
  episodios(aot, "Titan", 5);
  conferir("apos reordenar o Continuar");

  // 3) CATALOGO INTEIRO REPUBLICADO SEM O TITULO (a fileira de onde ele veio
  //    nao voltou): a copia da abertura volta por cat_acrescentar.
  { CatItem w[3];
    CatFileira g[1];
    w[0] = v[2]; w[1] = v[3]; w[2] = v[5];
    g[0] = fileira("addon_series_top", 0, 3);
    cat_definir_tudo(w, 3, g, 1); }
  aot = player_indice();
  assert(aot >= 3);                                  // entrou no fim
  assert(!strcmp(titulo(aot), "Attack on Titan"));
  episodios(aot, "Titan", 5);
  episodios(0, "Guinevere", 2);
  conferir("apos republicar sem o titulo");

  // 4) Uma republicacao ENTRE a atualizacao e o desenho: a pausa confere pelo
  //    id, e nao so pelo indice que recebeu no quadro.
  player_atualizar(0.016f, SDL_GetTicks());
  { CatItem w[4];
    CatFileira g[1];
    w[0] = v[0]; w[1] = v[4]; w[2] = v[2]; w[3] = v[3];
    g[0] = fileira("addon_series_top", 0, 4);
    cat_definir_tudo(w, 4, g, 1); }
  assert(!strcmp(titulo(pausao_indice()), "Attack on Titan"));
  assert(!strcmp(titulo(posplay_indice()), "Attack on Titan"));
  assert(!strcmp(titulo(episodios_titulo()), "Attack on Titan"));

  puts("PASS: #190 titulo e episodios do player seguem o titulo que toca apos republicar o catalogo");
  return 0;
}
