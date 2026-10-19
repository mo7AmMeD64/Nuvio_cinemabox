// MODO CINEMA DO TRAILER — a parte que a pagina de titulo e a home dividem.
//
// Dono, 21/09/2026 (detalhe) e 29/09/2026 (home): "quando o trailer comecar no
// hero, deixar ele igual a quando ta no details do titulo: so a arte do titulo
// (logo) embaixo e passando o trailer, e voltar ao normal quando acabar".
//
// Com o trailer TOCANDO o bloco de texto desce e apaga e so o logo fica,
// pequeno, no canto inferior esquerdo. Qualquer tecla traz o bloco de volta (o
// trailer segue); quando o trailer para, o bloco volta sozinho.
//
// Antes cada tela tinha a sua copia da conta (borda de subida + mola + os
// numeros do logo), e a home ia divergir na primeira mexida. Aqui mora so o que
// e PURO — estado, mola e medidas —, sem GL nem trailer.c: da para testar sem
// abrir janela (tests/trailercinema.c).
#ifndef NV_TRAILERCINEMA_H
#define NV_TRAILERCINEMA_H

#include "anim.h"
#include "layout.h"

// O bloco desce isto enquanto apaga (detail.c: heroWeb(..., c * 220)).
#define NV_CINEMA_DESCE       220.0f
// Base do logo pequeno, medida do fundo da tela.
#define NV_CINEMA_LOGO_BASE    96.0f
// Altura do logo pequeno: 0,62 do logo do heroi do detalhe (200 -> 124) e, no
// maximo, metade da largura maxima dele.
#define NV_CINEMA_LOGO_ESC      0.62f

typedef struct {
  float v;            // a mola: 0 = bloco no lugar, 1 = so o logo embaixo
  int   oculta;       // a intencao; `v` a persegue
  int   tocavaAntes;  // para a borda de subida
} TrailerCinema;

// Um passo por quadro. `toca` = o trailer desta tela esta MESMO tocando (o
// sinal `playing`, nao "aberto": um trailer que ainda prepara nao esconde nada).
// Borda de subida esconde o bloco; parar de tocar o devolve. `reduzida` e o
// ajuste "Animacoes reduzidas": sem mola, o estado salta.
static inline void trailercinema_passo(TrailerCinema *c, int toca, float dt, int reduzida) {
  if (toca && !c->tocavaAntes) c->oculta = 1;
  if (!toca) c->oculta = 0;
  c->tocavaAntes = toca;
  { float alvo = c->oculta ? 1.0f : 0.0f;
    c->v = reduzida ? alvo : anim_mola(c->v, alvo, dt, NV_MOLA_SCROLL); }
}

// Uma tecla traz o bloco de volta SEM fechar o trailer (nao rearma: so uma
// nova borda de subida esconde de novo).
static inline void trailercinema_tecla(TrailerCinema *c) { c->oculta = 0; }

static inline void trailercinema_zerar(TrailerCinema *c) { c->v = 0.0f; c->oculta = 0; c->tocavaAntes = 0; }

// Progresso com a curva suave, 0..1.
static inline float trailercinema_t(const TrailerCinema *c) { return anim_suave(c->v); }

// Tamanho do logo pequeno para um logo de proporcao `asp` (largura/altura).
static inline void trailercinema_logo(float asp, float *w, float *h) {
  float hh, ww;
  if (asp <= 0.0f) asp = 2.5f;
  hh = NV_DETW_LOGO_H * NV_CINEMA_LOGO_ESC; ww = hh * asp;
  if (ww > NV_DETW_LOGO_MAXW * 0.5f) { ww = NV_DETW_LOGO_MAXW * 0.5f; hh = ww / asp; }
  *w = ww; *h = hh;
}

// Base (y) do logo pequeno.
static inline float trailercinema_base(void) { return NV_TELA_H - NV_CINEMA_LOGO_BASE; }

#endif
