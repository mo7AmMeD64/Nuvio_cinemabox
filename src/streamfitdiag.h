#ifndef NV_STREAMFITDIAG_H
#define NV_STREAMFITDIAG_H
#include "rede.h"
typedef struct {
  uint64_t rede;
  unsigned prazo_ms;
  const char *ca_arquivo;
  int (*cancelado)(void *);
  void *usuario;
} StreamfitDiagControle;
/* Existing active diagnostic only: no implicit retry/probe/cache. Discards
 * media, with the caller's byte/time budget. Feeds only validated final-host
 * media and >=5 real intervals on an unchanged known network. */
int streamfitdiag_medir(const char *url, const char *const *cabecalhos,
    int segundos, long inicio, uint64_t maxBytes,
    const StreamfitDiagControle *controle, int *kbps, int nMax,
    RedeVazao *res, char *final, unsigned tamFinal);
#endif
