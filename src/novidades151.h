// Cartao de novidades da 1.5.1: fontes e ajustes com previa visual.
#ifndef NV_NOVIDADES151_H
#define NV_NOVIDADES151_H
#include <SDL2/SDL.h>

void novidades151_primeira_vez(void);
int  novidades151_aberto(void);
void novidades151_evento(const SDL_Event *e);
void novidades151_atualizar(float dt, Uint32 agora);
void novidades151_desenhar(Uint32 agora);
void novidades151_abrir(void);

#define N151_PEDIU_NADA    0
#define N151_PEDIU_AJUSTES 1
int novidades151_pedido(void);

#endif
