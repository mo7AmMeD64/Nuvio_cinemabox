// Ver chsegura.h.
#include "chsegura.h"

int chs_ocupado(const ChSegura *s) { return s && s->estado != CHS_LIVRE; }

int chs_desce(ChSegura *s, unsigned t) {
  if (!s) return CHS_NADA;
  if (s->estado == CHS_LIVRE) {
    s->estado = CHS_APERTADO;
    s->t0 = s->tUlt = t;
    s->solto = 0;
    return CHS_NADA;
  }
  // Repeticao (ou o Down do par Up/Down): a mesma tecla continua embaixo.
  s->tUlt = t;
  s->solto = 0;
  if (s->estado == CHS_APERTADO && t - s->t0 >= CHS_SEGURAR_MS) {
    s->estado = CHS_SEGURADO;
    return CHS_LONGO;
  }
  return CHS_NADA;
}

int chs_sobe(ChSegura *s, unsigned t) {
  if (!s || s->estado == CHS_LIVRE) return CHS_NADA;
  // O par "quase junto" do LG, enquanto a sessao nao provou KEYUP de verdade.
  if (!s->viuSolta && t - s->tUlt < CHS_FANTASMA_MS) return CHS_NADA;
  s->viuSolta = 1;
  s->solto = 1;
  s->tSolta = t;
  return CHS_NADA;
}

int chs_quadro(ChSegura *s, unsigned t) {
  if (!s) return CHS_NADA;
  if (s->estado == CHS_APERTADO) {
    if (s->solto) {
      if (t - s->tSolta >= CHS_EMENDA_MS) { s->estado = CHS_LIVRE; return CHS_CURTO; }
      return CHS_NADA;
    }
    if (s->viuSolta && t - s->t0 >= CHS_SEGURAR_MS) { s->estado = CHS_SEGURADO; return CHS_LONGO; }
    if (!s->viuSolta && t - s->tUlt >= CHS_SILENCIO_MS) { s->estado = CHS_LIVRE; return CHS_CURTO; }
    return CHS_NADA;
  }
  if (s->estado == CHS_SEGURADO) {
    if (s->solto ? t - s->tSolta >= CHS_EMENDA_MS
                 : (!s->viuSolta ? t - s->tUlt >= CHS_SILENCIO_MS : t - s->tUlt >= CHS_ESQUECER_MS))
      s->estado = CHS_LIVRE;
  }
  return CHS_NADA;
}
