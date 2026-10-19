// Cartao de novidades da 1.7.2: a cara nova (Nuvio Legacy), entrar com e-mail
// e as fontes que chegam conforme cada addon responde. O molde da 1.4.8 e da
// 1.7 (previa viva a esquerda, lista curta a direita, duas pilulas), com a
// paleta do logo novo: ameixa, creme, laranja, amarelo e vermelho.
#ifndef NV_NOVIDADES172_H
#define NV_NOVIDADES172_H
#include <SDL2/SDL.h>

// O numero da versao mora SO aqui.
#define N172_VERSAO "1.7.2"

void novidades172_dir(const char *dirArte);   // pasta da arte do pacote
void novidades172_primeira_vez(void);
int  novidades172_aberto(void);
void novidades172_abrir(void);
void novidades172_evento(const SDL_Event *e);
void novidades172_atualizar(float dt, Uint32 agora);
void novidades172_desenhar(Uint32 agora);

// Para a captura (tests/novidades172_shot.c).
int  novidades172_cenas(void);
void novidades172_ir(int cena, float t);
int  novidades172_itens(void);
int  novidades172_item_largura(int i, int *limite, const char **nome);

#endif
