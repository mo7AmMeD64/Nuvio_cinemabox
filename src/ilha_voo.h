// Geometria barata do voo do quadro ate a mini capa. Compartilhada com a
// regressao de limites/timing; nenhum estado de desenho ou textura aqui.
//
// CURVA: mola criticamente amortecida (resposta ~0,5 s, a padrao das
// transicoes da Apple), avaliada de forma fechada pelo relogio e nao somada
// por dt — um quadro atrasado nao vira salto, so pula para onde a mola estaria.
// Comeca parada (sem tranco), desacelera ate pousar sem atravessar o destino.
// Normalizada para valer exatamente 1 em NV_ILHA_VOO_MS (a sobra da mola ali
// e 1,5% do caminho; sem normalizar o fim seria um degrau de ~6 px).
//
// FORMA: o quadro mantem a PROPORCAO DE ORIGEM (a do video, 16:9 ou 2,39:1)
// encolhendo; so no ultimo terco o retangulo passa da proporcao do video para
// a da capa, e quem desenha usa cover forcado (recorta, nunca estica).
#ifndef NV_ILHA_VOO_H
#define NV_ILHA_VOO_H
#include "gfx.h"
#include <math.h>

#define NV_ILHA_VOO_MS 560u
#define NV_ILHA_VOO_W  11.0f    // rad/s; 2*pi/11 = resposta de ~0,57 s
#define NV_ILHA_PULSO_MS 320u   // a pilula "recebe" o quadro depois do pouso

static inline float ilha_voo_fracao(unsigned decorrido) {
  float t, T, x, xT;
  if (decorrido >= NV_ILHA_VOO_MS) return 1.0f;
  t = (float)decorrido / 1000.0f;
  T = (float)NV_ILHA_VOO_MS / 1000.0f;
  x = 1.0f - (1.0f + NV_ILHA_VOO_W * t) * expf(-NV_ILHA_VOO_W * t);
  xT = 1.0f - (1.0f + NV_ILHA_VOO_W * T) * expf(-NV_ILHA_VOO_W * T);
  x /= xT;
  return x < 0.0f ? 0.0f : x > 1.0f ? 1.0f : x;
}

static inline float ilha_voo_suave(float a, float b, float x) {
  float t = (x - a) / (b - a);
  t = t < 0.0f ? 0.0f : t > 1.0f ? 1.0f : t;
  return t * t * (3.0f - 2.0f * t);
}

// `de` = onde o quadro estava na tela (o retangulo do video; tela cheia no
// still). `asp` = proporcao da imagem (<= 0: a de `de`).
static inline GfxRect ilha_voo_rect_de(GfxRect de, float asp, GfxRect alvo, float t, float *f) {
  float u = t < 0.0f ? 0.0f : t > 1.0f ? 1.0f : t;
  float aDe = asp > 0.05f ? asp : (de.h > 1.0f ? de.w / de.h : 1.7777778f);
  float aAlvo = alvo.h > 1.0f ? alvo.w / alvo.h : 1.0f;
  float m = ilha_voo_suave(0.62f, 1.0f, u);
  // Proporcao em escala logaritmica: a passagem 16:9 -> 2:3 nao "corre" no fim.
  float a = expf(logf(aDe) + (logf(aAlvo) - logf(aDe)) * m);
  // A altura de partida e a do quadro na proporcao dele dentro de `de`.
  float h0 = de.w / de.h > aDe ? de.h : de.w / aDe;
  float h = h0 + (alvo.h - h0) * u, w = h * a;
  float cx = de.x + de.w * .5f + (alvo.x + alvo.w * .5f - de.x - de.w * .5f) * u;
  float cy = de.y + de.h * .5f + (alvo.y + alvo.h * .5f - de.y - de.h * .5f) * u;
  if (u >= 1.0f) { w = alvo.w; h = alvo.h; }
  *f = u;
  return (GfxRect){cx - w * .5f, cy - h * .5f, w, h};
}

static inline GfxRect ilha_voo_rect(GfxRect alvo, float t, float W0, float H0, float *f) {
  return ilha_voo_rect_de((GfxRect){0, 0, W0, H0}, W0 / H0, alvo, t, f);
}

// Escala da pilula depois do pouso: sobe ~6% e assenta, sem repique abaixo de 1.
static inline float ilha_voo_pulso(unsigned desdePouso) {
  float s;
  if (desdePouso >= NV_ILHA_PULSO_MS) return 1.0f;
  s = (float)desdePouso / (float)NV_ILHA_PULSO_MS;
  return 1.0f + 0.06f * sinf(3.14159265f * s) * (1.0f - s * 0.35f);
}

// SILENCIO DA CARGA DA HOME DURANTE O VOO (pedido do dono, 03/10: "quando
// fecha um filme quebra a animacao do filme indo pra ilha porque quando fecha
// a fileira de continue watching carrega"). A atividade tem prioridade sobre o
// cartao na pilula (ilha.c), entao "Carregando fileiras…" no meio do voo troca
// a pilula de forma e o quadro pousa numa capa que nao esta la
// (tests/ilha_voo_cw.sh: 428 px de pilula no pouso contra 543 do cartao). Quem
// alimenta a ilha com a carga (app.c) pergunta aqui: cala enquanto voa e por
// NV_ILHA_VOO_SILENCIO_MS depois do pouso (o pulso e o cartao assentarem).
#define NV_ILHA_VOO_SILENCIO_MS 1200u
typedef struct { int voava; unsigned pousou; } IlhaVooSilencio;
static inline int ilha_voo_silencio(IlhaVooSilencio *s, int voando, unsigned agora) {
  if (voando) { s->voava = 1; s->pousou = 0; return 1; }
  if (s->voava) { s->voava = 0; s->pousou = agora ? agora : 1u; }
  if (s->pousou && agora - s->pousou < NV_ILHA_VOO_SILENCIO_MS) return 1;
  s->pousou = 0;
  return 0;
}
#endif
