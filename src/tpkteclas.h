// Tecla do controle Samsung pelo NOME (Key.KeyPressedName do NUI) -> evento SDL.
//
// Mora fora de tpk.c (que so compila com NV_TPK) para o simulador do Mac usar a
// MESMA tabela: "XF86Blue" escrito em /tmp/nuvio-key faz o caminho da TV, e
// nao um atalho de teclado parecido com ele. Foi assim que a AZUL/CH+ digitando
// "s" passou dois consertos sem ninguem ver (05/10/2026).
#ifndef NV_TPKTECLAS_H
#define NV_TPKTECLAS_H
#include <SDL2/SDL.h>

// Preenche `e` (KEYDOWN/KEYUP) e devolve 1; 0 se o nome nao tem mapa (ja
// registra "[tecla] tpk sem mapa: <nome>" no log).
int tpkteclas_evento(const char *nome, int apertou, SDL_Event *e);

#endif
