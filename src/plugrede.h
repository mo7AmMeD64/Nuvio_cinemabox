// Transporte HTTP dos plugins Nuvio (F09). Mesmo contrato de rede_pedir (N01,
// rede.h) — RedePedido/RedeResposta, limites conferidos antes de alocar,
// RedeJob com geracao/cancelamento — com um adaptador a mais para o .wgt.
//
// NATIVO (Android, LG, TPK, Mac): e rede_pedir, sem nada por cima.
//
// WGT (Samsung, wasm): o N01 recusa (REDE_INDISPONIVEL) porque o XHR sincrono
// nao corta o corpo enquanto chega e nao aborta. Os plugins aceitam um
// contrato MENOR, que este adaptador entrega e nao exagera:
//   * so fora do fio principal (worker de pthread). No fio principal recusa,
//     sem XHR — la o XHR sincrono travaria o desenho e nao aceita timeout;
//   * prazo pelo xhr.timeout (permitido em XHR sincrono de worker): o
//     navegador corta a espera no prazo, inclusive DNS/conexao;
//   * teto de corpo/cabecalhos conferido ANTES de copiar para o heap do wasm
//     (o navegador ja baixou o corpo no heap DELE; o nosso nao cresce);
//   * cancelamento e geracao sao conferidos antes de enviar e ao voltar: um
//     XHR em curso NAO e interrompido, o resultado tardio e descartado;
//   * o navegador SEMPRE segue redirect (seguir=0 nao vale) e recusa os
//     cabecalhos proibidos (User-Agent, Referer, Origin, Cookie): limite real.
// rede_pedido_capacidades continua 0 no WGT; plugrede_capacidades diz o que
// este adaptador cumpre.
#ifndef NV_PLUGREDE_H
#define NV_PLUGREDE_H
#include "rede.h"

#define PR_CAP_REDE       1u   // ha transporte
#define PR_CAP_PRAZO      2u   // o prazo corta a espera em curso
#define PR_CAP_ABORTAR    4u   // cancelar interrompe a transferencia em curso
#define PR_CAP_REDIRECT   8u   // seguir=0 devolve o 3xx (redirect manual)
#define PR_CAP_CAB_LIVRES 16u  // User-Agent/Referer/Cookie passam

unsigned plugrede_capacidades(void);
// BLOQUEIA. Mesmo retorno de rede_pedir (1 = houve resposta HTTP).
int plugrede_pedir(const RedePedido *q, RedeResposta *r);

#endif
