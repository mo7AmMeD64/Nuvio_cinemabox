// Cliente websocket minimo, so o que o gateway do Discord precisa: texto,
// ping/pong e fechamento. Nada de compressao nem de extensao.
//
// POR QUE A MAO: a libcurl das TVs (7.53.1) e anterior ao websocket da libcurl
// (7.86), e o SDK do aparelho nao traz biblioteca de websocket. O que a TV TEM
// e TLS via libcurl (rede_tls_*), e o protocolo por cima e pequeno (RFC 6455).
// No navegador (Tizen .wgt) usa o WebSocket do proprio navegador.
//
// Tudo e chamado do fio principal e NUNCA bloqueia: a conexao e o aperto de
// mao rodam num fio proprio, e dws_receber so le o que ja chegou.
#ifndef NV_DISCORDWS_H
#define NV_DISCORDWS_H

typedef struct DiscordWs DiscordWs;

enum { DWS_CAIU = -1, DWS_CONECTANDO = 0, DWS_ABERTO = 1 };

// "wss://host/caminho?query". NULL so se faltar memoria.
DiscordWs *dws_abrir(const char *url);
int dws_estado(DiscordWs *w);
// 0 = queued/sent, -1 = closed or bounded queue full. Partial writes drain
// through dws_receber; sends never wait for socket writability.
int dws_enviar(DiscordWs *w, const char *texto);
// Proxima mensagem de texto completa (malloc, o chamador libera) ou NULL.
char *dws_receber(DiscordWs *w);
// Codigo do quadro de fechamento recebido (4004 etc), 0 se nao veio.
int dws_codigo_fechamento(DiscordWs *w);
// Fecha e libera. Pode ser chamado em qualquer estado, inclusive conectando.
void dws_fechar(DiscordWs *w);

#endif
