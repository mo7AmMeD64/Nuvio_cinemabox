// HTML tolerante + seletor CSS, em C — o lado nativo da ponte "cheerio" dos
// plugins Nuvio (plugins.h).
//
// POR QUE NAO RODAR O CHEERIO DE VERDADE DENTRO DO QUICKJS. Medido no Mac
// (tests/htmlq.sh imprime os numeros): o cheerio/slim empacotado sao 155 KB de
// JS que cada execucao de scraper teria de compilar, e a arvore dele vive como
// objetos JS — varias vezes o tamanho do HTML. Aqui o HTML vira um vetor de nos
// num bloco so, e o scraper so ve numeros (indices de no). E o mesmo desenho
// do app web (o DOM fica no hospedeiro, o JS recebe ids) e do Android oficial
// (Jsoup do lado Kotlin).
//
// NAO E O ALGORITMO DO HTML5. E o subconjunto que pagina real precisa: tags
// vazias, texto cru em script/style/textarea/title, fechamento implicito de
// p/li/dt/dd/tr/td/th/option e entidades. O que um scraper consulta (medido nos
// 29 provedores do repositorio publico tapframe/nuvio-providers) e tag, #id,
// .classe, [atributo op valor], combinadores, :nth-child e :contains.
#ifndef NV_HTMLQ_H
#define NV_HTMLQ_H
#include <stddef.h>

typedef struct HqDoc HqDoc;

// Copia `html` (n bytes) e monta a arvore. NULL so sem memoria.
HqDoc *hq_carregar(const char *html, size_t n);
// O mesmo com teto de bytes do documento (fonte + nos + textos), conferido
// ANTES de cada alocacao: NULL quando nao cabe (ou sem memoria). 0 = 64 MiB.
HqDoc *hq_carregar_max(const char *html, size_t n, size_t max);
void   hq_soltar(HqDoc *d);
// Bytes ocupados pelo documento (fonte + nos + textos), para a medida.
size_t hq_memoria(const HqDoc *d);
int    hq_nos(const HqDoc *d);

// Elementos que casam `seletor`, em ordem do documento, dentro de `ctx`
// (descendentes; -1 = documento inteiro). Vetor de malloc em *saida (NULL
// quando 0). Devolve a quantidade, ou -1 com seletor invalido — que a ponte
// trata como lista vazia, igual ao Android (Jsoup) e ao web.
int hq_selecionar(const HqDoc *d, int ctx, const char *seletor, int **saida);
// 1 quando o elemento `no` casa `seletor`; 0 nao; -1 seletor invalido.
int hq_casa(const HqDoc *d, int no, const char *seletor);

// Texto de varios elementos como o Jsoup: o de cada um com os brancos
// colapsados e aparado, unidos por espaco. malloc.
char *hq_texto(const HqDoc *d, const int *nos, int n);
// HTML interno (externo=0) ou externo (1) de `no`, como esta na fonte;
// no = -1 e o documento inteiro. malloc.
char *hq_html(const HqDoc *d, int no, int externo);
// Valor do atributo (entidades ja decodificadas) ou NULL quando ausente.
const char *hq_attr(const HqDoc *d, int no, const char *nome);
const char *hq_tag(const HqDoc *d, int no);
// Parentes ELEMENTO (texto e comentario sao pulados); -1 quando nao ha.
int hq_pai(const HqDoc *d, int no);
int hq_prox(const HqDoc *d, int no);
int hq_ant(const HqDoc *d, int no);
// Filhos elemento, em ordem. malloc em *saida; devolve a quantidade.
int hq_filhos(const HqDoc *d, int no, int **saida);

#endif
