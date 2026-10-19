// PORTAO DO TEXTO: a tela abre com o texto INTEIRO ou nao mostra texto ainda.
//
// #172 ("parece que o texto esta carregando") e o dono ("abrir com o texto
// correto, ou skeleton do que e um texto e depois trocar"): o rasterizador tem
// orcamento por quadro (text.c), e uma pagina com 20-40 linhas novas entrava em
// degraus — palavra a palavra, linha a linha. O portao esconde o bloco enquanto
// alguma linha do PRIMEIRO viewport ainda nao existe como textura e o revela
// junto, num esvanecimento so. Enquanto esconde, quem chama continua DESENHANDO
// o bloco com opacidade quase nula: e isso que rasteriza as linhas durante a
// animacao de abertura, para estarem prontas quando ela acaba.
//
// Funcao pura (sem GL, sem SDL alem do tipo), para poder ser testada sem
// janela: tests/textogate.c.
#ifndef NV_TEXTOGATE_H
#define NV_TEXTOGATE_H
#include "anim.h"
#include "revela.h"
#include <SDL2/SDL.h>

// Teto de espera: passado isso o texto aparece mesmo incompleto (as linhas que
// faltam entram como sempre entraram). Texto escondido para sempre e pior que
// texto em degraus.
#define NV_TXTGATE_TIMEOUT_MS 400u
// Esvanecimento unico do bloco todo.
#define NV_TXTGATE_FADE_MS    180.0f
// Opacidade com que o bloco e desenhado enquanto o portao esta fechado: acima do
// corte de desenho de heroWeb (0,005), abaixo do que 8 bits enxergam.
#define NV_TXTGATE_AQUECER    0.006f

typedef struct {
  Uint32 inicio;   // primeiro quadro em que o portao foi consultado (0 = ainda nao)
  Uint32 pronto;   // quando o bloco ficou completo, ou o teto estourou (0 = nao)
} TextoGate;

static inline void textogate_reiniciar(TextoGate *g) { g->inicio = g->pronto = 0; }

// Uma vez por quadro, DEPOIS de desenhar o bloco. `pendentes` = linhas que o
// desenho do bloco pediu e o orcamento recusou (diferenca de txt_pendentes).
// Devolve a opacidade 0..1 a aplicar no bloco NO PROXIMO quadro.
static inline float textogate_passo(TextoGate *g, int pendentes, Uint32 agora) {
  if (!g->inicio) g->inicio = agora ? agora : 1u;
  if (!g->pronto) {
    if (pendentes <= 0 || (Uint32)(agora - g->inicio) >= NV_TXTGATE_TIMEOUT_MS)
      g->pronto = agora ? agora : 1u;
    else return 0.0f;
  }
  if (anim_politica_reduzida) return 1.0f;
  return revela_saida((float)(Uint32)(agora - g->pronto) / NV_TXTGATE_FADE_MS);
}

// O bloco ja foi revelado (opacidade final)? Depois disso nada e escondido de
// novo, mesmo que uma linha seja despejada do cache.
static inline int textogate_aberto(const TextoGate *g) { return g->pronto != 0; }

#endif
