// PESSOAS DO TMDB PARA O SPOTLIGHT (spotlight.c).
//
// O Spotlight so achava pessoa no ELENCO dos titulos ja carregados: quem
// nunca tinha aberto um filme com o ator nao o achava. Isto pergunta ao TMDB
// (/search/person, com a chave e o idioma que o app ja usa, ver
// desc_chave_tmdb e desc_tmdb_idioma) e devolve nome, foto de perfil e os
// titulos pelos quais a pessoa e conhecida.
//
// DEBOUNCE: o pedido so sai quando o texto fica parado SPP_ESPERA_MS. Digitar
// "christian" no teclado de tela sao nove teclas em poucos segundos, e uma
// viagem por letra seriam nove pedidos que ninguem le. Um pedido por vez; se o
// texto mudou enquanto um estava na rede, o mais novo sai quando ele volta.
// As ultimas respostas ficam num cache curto: apagar uma letra e redigitar nao
// volta a rede.
//
// Nao bloqueia. pedir/atualizar/ler sao do fio de desenho; a rede e de um fio
// proprio.
#ifndef NV_SPOTPESSOA_H
#define NV_SPOTPESSOA_H

#define SPP_MAX       5
#define SPP_ESPERA_MS 350u
#define SPP_MIN_CP    3       // codepoints antes de perguntar

typedef struct {
  long tmdb;
  char nome[96];
  char foto[200];          // w185 do TMDB, ou ""
  char conhecido[200];     // "Duna · Wolverine · O Grande Truque"
  long tituloTmdb;         // o primeiro titulo conhecido: e por ele que a
  char tituloTipo[8];      // filmografia abre ("movie" / "tv")
} SpotPessoa;

// Le a resposta de /search/person. Pura (testes). So entra quem tem id e um
// titulo conhecido com id (sem ele nao ha pagina onde abrir a filmografia).
int  spotpessoa_extrair(const char *json, SpotPessoa *out, int max);

// O texto do campo mudou (ou nao): `agora` em ms. Barato, chamar a cada
// remontagem.
void spotpessoa_pedir(const char *termo, unsigned agora);
// Dispara o pedido quando o texto assentou. Chamar a cada quadro.
void spotpessoa_atualizar(unsigned agora);
// Quantas pessoas ha para `termo`; -1 quando ainda nao ha resposta.
int  spotpessoa_n(const char *termo);
int  spotpessoa_item(const char *termo, int i, SpotPessoa *out);
// Muda quando chega resposta nova: o Spotlight remonta.
unsigned spotpessoa_geracao(void);

// Testes: troca o download (recebe a URL pronta) e conta os pedidos que
// sairam. Com `baixar` definido a chave vazia vira "TESTE".
void spotpessoa_teste(char *(*baixar)(const char *url));
int  spotpessoa_disparos(void);
#endif
