// No network or account: exercise actual native framing and bounded sends.
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../src/discordws.c"
struct RedeTls { int dummy; };
static int blocked, writes, closed;
static size_t prefix = 3, emittedN;
static unsigned char emitted[100000];
static unsigned char header[4096];
static size_t headerN, headerOff;
RedeTls *rede_tls_abrir(const char *url, int s) { (void)url; (void)s; return NULL; }
void rede_tls_fechar(RedeTls *t) { free(t); closed++; }
int rede_tls_enviar(RedeTls *t, const void *b, size_t n) { (void)t; (void)b; (void)n; return 0; }
int rede_tls_tentar_enviar(RedeTls *t, const void *b, size_t n, size_t *foi) {
  (void)t; writes++;
  *foi = blocked ? 0 : (n < prefix ? n : prefix);
  assert(emittedN + *foi <= sizeof emitted);
  memcpy(emitted + emittedN, b, *foi); emittedN += *foi;
  return 0;
}
int rede_tls_receber(RedeTls *t, void *b, size_t n, int espera) {
  (void)t; (void)espera;
  if (headerOff == headerN) return 0;
  if (n > headerN - headerOff) n = headerN - headerOff;
  memcpy(b, header + headerOff, n); headerOff += n; return (int)n;
}
static DiscordWs *opened(void) {
  DiscordWs *w = calloc(1, sizeof *w);
  pthread_mutex_init(&w->trava, NULL); atomic_init(&w->estado, DWS_ABERTO);
  w->tls = calloc(1, sizeof *w->tls);
  return w;
}
static void checkFrame(size_t *off, const char *text) {
  size_t n = strlen(text), i; unsigned char *p = emitted + *off;
  assert(p[0] == 0x81 && p[1] == (0x80 | n));
  for (i = 0; i < n; i++) assert((p[6+i] ^ p[2+(i&3)]) == (unsigned char)text[i]);
  *off += n + 6;
}
int main(void) {
  DiscordWs *w = opened(); unsigned t; size_t off = 0; int i;
  blocked = 1; t = agoraWs();
  // A full socket used to wait ten seconds on every main-thread send.
  for (i = 0; i < 1000; i++) assert(dws_enviar(w, "tiny") == 0);
  assert(agoraWs() - t < 250); assert(writes == 1000);
  assert(w->nTx == 10000 && emittedN == 0);
  w->txDesde -= 10001; assert(dws_receber(w) == NULL);
  assert(dws_estado(w) == DWS_CAIU); dws_fechar(w);
  // Partial writes must retain frame order and masking across later app ticks.
  blocked = 0; writes = 0; emittedN = 0; w = opened();
  assert(dws_enviar(w, "first") == 0); assert(dws_enviar(w, "second") == 0);
  for (i = 0; i < 20 && w->nTx; i++) dws_receber(w);
  assert(w->nTx == 0); checkFrame(&off, "first"); checkFrame(&off, "second");
  assert(off == emittedN); dws_fechar(w);
  // Queue growth is bounded; closing a full socket does not attempt to drain.
  blocked = 1; w = opened();
  for (i = 0; i < 7000 && dws_estado(w) == DWS_ABERTO; i++) dws_enviar(w, "tiny");
  assert(dws_estado(w) == DWS_CAIU && w->nTx <= 65536);
  i = writes; t = agoraWs(); dws_fechar(w); assert(writes == i && agoraWs()-t < 250);
  // Exact-sized handshake chunks need one extra byte for their terminator.
  w = opened(); memset(header, 'x', sizeof header);
  memcpy(header, "HTTP/1.1 101 Switching Protocols\r\nX: ", 36);
  memcpy(header + sizeof header - 4, "\r\n\r\n", 4);
  headerN = sizeof header; headerOff = 0;
  assert(apertarMao(w) == 0); assert(w->nRx == 0); dws_fechar(w);
  assert(closed == 4); puts("discordws_local: ok"); return 0;
}
