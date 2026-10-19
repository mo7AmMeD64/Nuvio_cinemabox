// Cartao de novidades da 1.6.0: a maior versao ate aqui, e a do app nativo
// tambem na Samsung. Previa viva a esquerda (sete cenas com os componentes de
// verdade), a lista agrupada a direita e tres pilulas no rodape.
#ifndef NV_NOVIDADES160_H
#define NV_NOVIDADES160_H
#include <SDL2/SDL.h>

// O numero da versao mora SO aqui: trocar o nome do lancamento e uma linha.
#define N160_VERSAO "1.6.0"

void novidades160_dir(const char *dirArte);   // pasta da arte do pacote
void novidades160_primeira_vez(void);
int  novidades160_aberto(void);
void novidades160_abrir(void);
void novidades160_evento(const SDL_Event *e);
void novidades160_atualizar(float dt, Uint32 agora);
void novidades160_desenhar(Uint32 agora);

#define N160_PEDIU_NADA   0
#define N160_PEDIU_VIDRO  1   // Ajustes › Aparência, na linha do vidro
#define N160_PEDIU_LAYOUT 2   // Ajustes › Layout, na linha do layout da home
int novidades160_pedido(void);

// Para a captura (tests/novidades160_shot.c): quantas cenas, qual esta na
// tela, e pular direto para uma delas com o relogio interno em `t` segundos.
int  novidades160_cenas(void);
int  novidades160_cena(void);
void novidades160_ir(int cena, float t);
// A lista: quantas linhas, e a largura da descricao da linha `i` no idioma
// atual contra o `limite` da coluna (a captura confere os 30 idiomas).
int  novidades160_itens(void);
int  novidades160_item_largura(int i, int *limite, const char **nome);
// 1 = a previa esta com o foco (cima), 0 = os botoes.
int  novidades160_foco_na_previa(void);

#endif
