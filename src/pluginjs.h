// Execucao de UM scraper de plugin Nuvio no QuickJS — o motor; quem decide
// quais scrapers rodam e o que fazer com as fontes e plugins.c.
//
// Um runtime NOVO por execucao (como o Android oficial): nada de um scraper
// vaza para o seguinte, e soltar o runtime devolve toda a memoria de uma vez.
// O laco de eventos (promessas, setTimeout, fetch) roda NO FIO DE QUEM CHAMA;
// cada fetch vai para um fio do pool de rede (plugrede_pedir = N01 rede_pedir
// no nativo) e volta ao laco por uma caixa com trava. O JS so e tocado pelo fio
// do laco; nenhuma trava e segurada durante rede ou disco.
//
// LIMITES (F09), todos conferidos ANTES de alocar:
//   * heap JS por runtime (JS_SetMemoryLimit) e um ORCAMENTO GLOBAL somado de
//     todos os runtimes vivos + documentos HTML (pj_orcamento_definir): o
//     alocador recusa quando a soma passaria do teto;
//   * documento HTML: teto por runtime e reserva global antes de montar;
//   * rede: teto de corpo por resposta e um orcamento global de bytes de
//     resposta em voo, reservado antes do pedido sair; fila por runtime
//     limitada (em voo, esperando e total por execucao);
//   * timers vivos limitados.
// Prazo da execucao inteira, cancelamento por flag/callback e GERACAO (RedeJob
// do grupo do dono) interrompem o JS (interrupt handler) e os fetches em voo.
#ifndef NV_PLUGINJS_H
#define NV_PLUGINJS_H
#include <stddef.h>
#include "rede.h"

typedef struct {
  const char *codigo;          // fonte do scraper (JS)
  size_t nCodigo;
  const char *arquivo;         // nome para mensagens de erro ("providers/x.js")
  const char *idScraper, *nomeScraper;
  const char *tmdbId;          // "27205"
  const char *tipo;            // "movie" | "tv"
  int temporada, episodio;     // <= 0 = null
  const char *tmdbChave;       // TMDB_API_KEY do scraper ("" = nenhuma)
  const char *ajustesJson;     // SCRAPER_SETTINGS ("{}")
  int prazoMs;                 // o scraper inteiro (0 = 20 s)
  int redeSegundos;            // cada fetch (0 = 15 s)
  long redeMaxBytes;           // corpo de cada resposta (0 = 5 MB)
  long memoriaMax;             // teto do runtime (0 = 48 MB)
  long domMax;                 // teto dos documentos HTML (0 = 16 MB)
  int maxResultados;           // 0 = 150
  int verLog;                  // 1 = console.log do scraper vai para o stdout
  int *cancelado;              // opcional, lido atomico: 1 para e descarta; 2 corta
  int (*parar)(void *);        // opcional: != 0 para (sem trabalho de UI)
  void *pararU;
  RedeJob *job;                // opcional: geracao da execucao (retido aqui)
  RedeGrupo *grupo;            // opcional: cada fetch ganha um job deste grupo
} PjPedido;

typedef struct {
  char *json;                  // {"streams":[...]} no formato Stremio (malloc)
  int n;                       // fontes no json
  unsigned long ms;            // tempo total
  unsigned long msCompilar;    // ambiente + codigo do scraper
  size_t memPico;              // pico do runtime QuickJS + DOM
  size_t domPico;              // pico dos documentos HTML (htmlq)
  int fetches, fetchesFalhos, fetchesRecusados;
  long bytesRede;
  int estourou;                // 1 = prazo, memoria ou cancelamento
  int cancelado;               // 1 = cancelamento/geracao (resultado descartado)
  int semMemoria;              // 1 = teto do runtime ou orcamento global
  int criptoUsado;             // 1 = o scraper carregou o CryptoJS
  char erro[240];              // "" quando terminou normalmente
} PjResultado;

// BLOQUEIA ate o scraper terminar, estourar o prazo ou ser cancelado. Chamar de
// um fio com pilha de pj_pilha() bytes. Devolve o numero de fontes (0 quando
// cancelado: o json e descartado).
int  pj_executar(const PjPedido *p, PjResultado *r);
void pj_resultado_soltar(PjResultado *r);
size_t pj_pilha(void);

// Orcamento GLOBAL (todos os runtimes juntos). heap = JS + DOM; rede = corpos
// de resposta em voo. Zero mantem o atual. Padrao por plataforma no codigo.
void   pj_orcamento_definir(size_t heap, size_t rede);
size_t pj_orcamento_heap_uso(void);
size_t pj_orcamento_rede_uso(void);
// Para os testes: maior uso global ja visto (heap) desde o ultimo zerar.
size_t pj_orcamento_heap_pico(void);
void   pj_orcamento_zerar_pico(void);

#endif
