// Simulacao da fila de rede com a ordem REAL de tirarFila: quanto o logo de um
// titulo espera para comecar a baixar quando chega atras de uma rajada de
// cartazes (mesmo cenario do logcat da C9: 4 fios, ~740 ms por arquivo).
#include "../src/sdlcompat.h"
#include "../src/tex_cache.c"
#include <assert.h>
#include <stdio.h>

// Devolve a espera (ms) do logo, que chega no instante `chegada` com `n` itens
// ja na fila (o ultimo e o fundo de tela cheia, urgente=1).
static int simular(int nivelLogo, int n, int chegada, int servico, int fios) {
  int f[MAX_FILA], ini = 0, fim = 0, k, livre[8], t, logoIdx = n + 1;
  for (k = 0; k < MAX_ITENS_ABS; k++) itens[k].urgente = 0;
  for (k = 0; k < n; k++) { f[fim] = k; fim = (fim + 1) % MAX_FILA; }
  itens[n - 1].urgente = 1;
  // Os fios ja estao ocupados com a rajada anterior, liberando em escada.
  for (k = 0; k < fios; k++) livre[k] = (servico * (k + 1)) / fios;
  { int logoNaFila = 0;
    for (t = 0;; t++) {
      int w = 0;
      for (k = 1; k < fios; k++) if (livre[k] < livre[w]) w = k;
      t = livre[w];
      if (!logoNaFila && t >= chegada) {
        itens[logoIdx].urgente = nivelLogo;
        f[fim] = logoIdx; fim = (fim + 1) % MAX_FILA; logoNaFila = 1;
      }
      if (ini == fim) break;
      { int i = tirarFila(f, &ini, fim);
        if (i == logoIdx) return t - chegada;
        livre[w] = t + servico; }
    }
  }
  return -1;
}

int main(void) {
  int n, antes, depois;
  for (n = 6; n <= 24; n += 6) {
    antes = simular(0, n, 100, 740, 4);
    depois = simular(2, n, 100, 740, 4);
    printf("fila=%2d  espera do logo ate comecar a baixar: antes=%4d ms  depois=%4d ms\n",
           n, antes, depois);
    assert(depois <= antes);
  }
  puts("logofila: ok");
  return 0;
}
