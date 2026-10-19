// Ver plugrede.h.
#include "plugrede.h"
#include <stdlib.h>
#include <string.h>

#ifndef __EMSCRIPTEN__

unsigned plugrede_capacidades(void) {
  return rede_pedido_capacidades() ? (PR_CAP_REDE | PR_CAP_PRAZO | PR_CAP_ABORTAR |
                                      PR_CAP_REDIRECT | PR_CAP_CAB_LIVRES) : 0;
}
int plugrede_pedir(const RedePedido *q, RedeResposta *r) { return rede_pedir(q, r); }

#else

#include <emscripten.h>
#include <emscripten/threading.h>
#include <time.h>

// XHR sincrono de worker com timeout. Cabecalhos de pedido em "Nome: valor\n".
// Devolve 1 com resposta; 0 sem resposta (*motivo: 1 transporte, 2 prazo,
// 3 corpo acima do teto, 4 cabecalhos acima do teto). O corpo so e copiado para
// o heap do wasm DEPOIS de conferido contra o teto.
EM_JS(int, nv_plug_xhr, (const char *metodo, const char *url, const char *cabs,
                         const char *corpo, int nCorpo, int prazoMs, int teto, int tetoCab,
                         int *status, int *motivo, char *urlFinal, int urlFinalTam,
                         char **saiCorpo, int *saiN, char **saiCab, int *saiNCab), {
  var xhr = new XMLHttpRequest();
  HEAP32[motivo >> 2] = 1;
  try { xhr.open(UTF8ToString(metodo), UTF8ToString(url), false); } catch (e) { return 0; }
  try { xhr.timeout = prazoMs; } catch (e) { return 0; }   // worker: permitido
  try { xhr.overrideMimeType("text/plain; charset=x-user-defined"); } catch (e) {}
  if (cabs) UTF8ToString(cabs).split("\n").forEach(function (l) {
    var i = l.indexOf(":"); if (i <= 0) return;
    try { xhr.setRequestHeader(l.slice(0, i).trim(), l.slice(i + 1).trim()); } catch (e) {}
  });
  try {
    xhr.send(corpo && nCorpo > 0 ? HEAPU8.slice(corpo, corpo + nCorpo) : null);
  } catch (e) {
    HEAP32[motivo >> 2] = (e && (e.name === "TimeoutError" || e.code === 23)) ? 2 : 1;
    return 0;
  }
  if (!xhr.status) return 0;
  HEAP32[status >> 2] = xhr.status;
  stringToUTF8(xhr.responseURL || "", urlFinal, urlFinalTam);
  var h = (xhr.getAllResponseHeaders() || "").split("\r").join("");
  if (lengthBytesUTF8(h) + 1 > tetoCab) { HEAP32[motivo >> 2] = 4; return 0; }
  var s = xhr.responseText || "", n = s.length;
  if (n > teto) { HEAP32[motivo >> 2] = 3; return 0; }
  var nh = lengthBytesUTF8(h) + 1, ph = _malloc(nh);
  if (!ph) return 0;
  stringToUTF8(h, ph, nh);
  var p = _malloc(n + 1);
  if (!p) { _free(ph); return 0; }
  for (var i = 0; i < n; i++) HEAPU8[p + i] = s.charCodeAt(i) & 0xff;
  HEAPU8[p + n] = 0;
  HEAP32[saiCorpo >> 2] = p; HEAP32[saiN >> 2] = n;
  HEAP32[saiCab >> 2] = ph; HEAP32[saiNCab >> 2] = nh - 1;
  HEAP32[motivo >> 2] = 0;
  return 1;
});

unsigned plugrede_capacidades(void) { return PR_CAP_REDE | PR_CAP_PRAZO; }

static unsigned agoraMs(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (unsigned)(ts.tv_sec * 1000u + (unsigned)(ts.tv_nsec / 1000000));
}

static RedeErro parou(const RedePedido *q) {
  RedeErro e = rede_job_estado(q->job);
  if (e != REDE_OK) return e;
  if (q->parar && q->parar(q->parar_usuario)) return REDE_CANCELADO;
  return REDE_OK;
}

int plugrede_pedir(const RedePedido *q, RedeResposta *r) {
  char *cabs = NULL, *corpo = NULL, *cab = NULL;
  int n = 0, nCab = 0, st = 0, motivo = 0, ok;
  size_t teto, tetoCab, tam = 1;
  unsigned t0 = agoraMs(), prazo;
  const char *m;
  int k;
  if (!r) return 0;
  memset(r, 0, sizeof *r);
  if (!q || !q->url || (strncmp(q->url, "http://", 7) && strncmp(q->url, "https://", 8))) {
    r->erro = REDE_ENTRADA; return 0;
  }
  // No fio principal o XHR sincrono travaria o desenho e recusa timeout.
  if (emscripten_is_main_runtime_thread()) { r->erro = REDE_INDISPONIVEL; return 0; }
  teto = q->max_bytes ? q->max_bytes : REDE_CORPO_PADRAO;
  tetoCab = q->max_cabecalhos ? q->max_cabecalhos : REDE_CAB_PADRAO;
  if (teto > REDE_CORPO_MAXIMO || tetoCab > REDE_CAB_MAXIMO || q->prazo_ms > 300000 ||
      (q->n_corpo && !q->corpo) || q->n_corpo > REDE_CORPO_PADRAO) {
    r->erro = REDE_ENTRADA; return 0;
  }
  m = q->metodo ? q->metodo : "GET";
  prazo = q->prazo_ms ? q->prazo_ms : 15000;
  rede_job_reter(q->job);
  if ((r->erro = parou(q)) != REDE_OK) goto fim;
  for (k = 0; q->cabecalhos && q->cabecalhos[k] && k < 64; k++) tam += strlen(q->cabecalhos[k]) + 1;
  if (tam > tetoCab) { r->erro = REDE_ENTRADA; goto fim; }
  if (q->cabecalhos && q->cabecalhos[0]) {
    cabs = malloc(tam);
    if (!cabs) { r->erro = REDE_MEMORIA; goto fim; }
    cabs[0] = 0;
    for (k = 0; q->cabecalhos[k] && k < 64; k++) { strcat(cabs, q->cabecalhos[k]); strcat(cabs, "\n"); }
  }
  ok = nv_plug_xhr(m, q->url, cabs, (const char *)q->corpo, (int)q->n_corpo, (int)prazo,
                   (int)teto, (int)tetoCab, &st, &motivo, r->final, (int)sizeof r->final,
                   &corpo, &n, &cab, &nCab);
  free(cabs);
  r->status = st;
  if (!ok) {
    r->erro = motivo == 2 ? REDE_PRAZO : motivo == 3 ? REDE_LIMITE_CORPO :
              motivo == 4 ? REDE_LIMITE_CABECALHOS : REDE_TRANSPORTE;
    goto fim;
  }
  // Tardio: o XHR nao e abortavel; geracao/cancelamento valem na entrega.
  if ((r->erro = parou(q)) != REDE_OK) { free(corpo); free(cab); goto fim; }
  if (agoraMs() - t0 > prazo) { r->erro = REDE_PRAZO; free(corpo); free(cab); goto fim; }
  r->corpo = corpo; r->n_corpo = (size_t)n;
  r->cabecalhos = cab; r->n_cabecalhos = (size_t)nCab;
  r->bytes_fio = (uint64_t)n;
  r->erro = REDE_OK;
fim:
  r->ms = agoraMs() - t0;
  rede_job_soltar(q->job);
  return r->erro == REDE_OK && r->status > 0;
}

#endif
