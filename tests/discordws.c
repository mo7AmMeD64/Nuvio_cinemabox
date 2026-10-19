// Websocket proprio (discordws.c) contra o gateway REAL do Discord. PRECISA de
// rede. O gateway manda HELLO (op 10) sem login nenhum, entao isto prova TLS
// cru (rede_tls_*), aperto de mao, leitura de quadro do servidor e ESCRITA de
// quadro mascarado — o heartbeat so volta como ACK (op 11) se o quadro que
// mandamos estiver certo.
#include "discordws.h"
#include "js.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static unsigned ms(void) {
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return (unsigned)(t.tv_sec * 1000 + t.tv_nsec / 1000000);
}

int main(void) {
  DiscordWs *w = dws_abrir("wss://gateway.discord.gg/?v=10&encoding=json");
  unsigned t0 = ms();
  int hello = 0, ack = 0;
  while (ms() - t0 < 20000 && !(hello && ack)) {
    char *m;
    if (dws_estado(w) == DWS_CAIU) { printf("FALHOU: caiu (%d)\n", dws_codigo_fechamento(w)); return 1; }
    while ((m = dws_receber(w))) {
      int op = (int)js_num(m, NULL, "op", -1);
      printf("recebido op %d (%zu bytes)\n", op, strlen(m));
      if (op == 10 && !hello) {
        hello = 1;
        if (js_num(m, NULL, "heartbeat_interval", 0) <= 0) { printf("FALHOU: hello sem intervalo\n"); return 1; }
        if (dws_enviar(w, "{\"op\":1,\"d\":null}") != 0) { printf("FALHOU: envio\n"); return 1; }
      }
      if (op == 11) ack = 1;
      free(m);
    }
    usleep(20000);
  }
  dws_fechar(w);
  if (!hello) { printf("FALHOU: sem HELLO\n"); return 1; }
  if (!ack) { printf("FALHOU: heartbeat sem ACK (quadro mascarado errado?)\n"); return 1; }
  // Token falso: o gateway fecha com 4004, que e o gatilho do refresh em
  // discord.c. Prova que o codigo do quadro de fechamento e lido.
  { DiscordWs *x = dws_abrir("wss://gateway.discord.gg/?v=10&encoding=json");
    int mandou = 0;
    t0 = ms();
    while (ms() - t0 < 20000 && dws_estado(x) != DWS_CAIU) {
      char *m;
      while ((m = dws_receber(x))) {
        if ((int)js_num(m, NULL, "op", -1) == 10 && !mandou) {
          dws_enviar(x, "{\"op\":2,\"d\":{\"token\":\"Bearer falso\",\"intents\":0,"
                        "\"properties\":{\"os\":\"linux\",\"browser\":\"Nuvio\",\"device\":\"teste\"}}}");
          mandou = 1;
        }
        free(m);
      }
      usleep(20000);
    }
    printf("token falso: fechamento %d\n", dws_codigo_fechamento(x));
    if (dws_codigo_fechamento(x) != 4004) { printf("FALHOU: esperava 4004\n"); return 1; }
    dws_fechar(x); }
  // Fechar com a conexao ainda conectando nao pode vazar nem travar.
  dws_fechar(dws_abrir("wss://gateway.discord.gg/?v=10&encoding=json"));
  printf("discordws: ok\n");
  return 0;
}
