// Release 1.7.4 highlights; preview API also used by capture tests.
#ifndef NV_NOVIDADES174_H
#define NV_NOVIDADES174_H
#include <SDL2/SDL.h>

// O numero da versao mora SO aqui.
#define N174_VERSAO "1.7.4"

void novidades174_dir(const char *dirArte);   // pasta da arte do pacote
void novidades174_primeira_vez(void);
int  novidades174_aberto(void);
void novidades174_abrir(void);
void novidades174_evento(const SDL_Event *e);
void novidades174_atualizar(float dt, Uint32 agora);
void novidades174_desenhar(Uint32 agora);

// Para a captura (tests/novidades174_shot.c).
int  novidades174_cenas(void);
void novidades174_ir(int cena, float t);
int  novidades174_itens(void);
int  novidades174_item_largura(int i, int *limite, const char **nome);

#endif
