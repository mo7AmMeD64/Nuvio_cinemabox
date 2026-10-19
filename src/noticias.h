// ULTIMAS NOTICIAS DE UM TITULO, para o menu de contexto da Agenda.
//
// Pedido do dono (21/09/2026): "vamos pegar as ultimas noticias de cada
// titulo". Fonte: o RSS de busca do Google News, que nao pede chave, aceita
// qualquer origem e devolve manchete, veiculo e data. So MANCHETES: a TV nao
// abre link, entao a lista e o que se le, nao um indice para clicar.
//
//   https://news.google.com/rss/search?q="<titulo>"&hl=pt-BR&gl=BR&ceid=BR:pt-419
//
// A lingua segue a da interface (ajustes_idioma_ingles). Uma passada de rede
// por titulo, em fio proprio, com cache em memoria e em disco por 6 h
// (noticias-<imdb>.txt, dados_gravar_leve). O laco de desenho so le.
#ifndef NV_NOTICIAS_H
#define NV_NOTICIAS_H

#define NOT_MAX 12

typedef struct {
  char titulo[240];   // manchete, ja sem " - Veiculo" no fim
  char fonte[80];     // veiculo
  char data[16];      // "20 set" / "20 Sep", pronta para desenhar
  long chave;         // AAAAMMDD, para ordenar da mais nova para a mais velha
  // O <link> do item (news.google.com/rss/articles/...) e o instante do
  // pubDate. Entraram com o modal de noticia (29/09/2026): o link e o que
  // noticia.c resolve ate a pagina do veiculo, e o instante da a data relativa
  // ("há 3 h") da linha em foco. Vazio/0 no cache de disco da versao anterior.
  char link[400];
  long long quando;   // epoch UTC do pubDate; 0 = nao veio
} Noticia;

// "há 3 h", "ontem", "há 5 dias"; passada uma semana, a data curta de sempre
// (Noticia.data). `agora` e o epoch de quem desenha (time(NULL)).
void noticias_quando(const Noticia *nt, long long agora, char *dst, int tam);

// Dispara a busca (uma vez por imdb; repetir e gratis). `serie` so muda a
// palavra de apoio na consulta ("serie"/"filme") para desambiguar titulos.
void noticias_pedir(const char *imdb, const char *titulo, const char *rede, int serie);
// 1 quando a rede ja respondeu (com ou sem manchetes).
int  noticias_respondeu(const char *imdb);
int  noticias_n(const char *imdb);
const Noticia *noticias_item(const char *imdb, int i);

#endif
