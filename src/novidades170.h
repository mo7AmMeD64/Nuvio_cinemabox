// Cartao de novidades da 1.7: a ilha do relogio (o filme pela metade, o modal
// e os Salvos que nascem dela), o Spotlight e o layout Apple TV. Previa viva a
// esquerda (tres cenas com as medidas dos componentes de verdade), a lista
// curta a direita e duas pilulas.
#ifndef NV_NOVIDADES170_H
#define NV_NOVIDADES170_H
#include <SDL2/SDL.h>

// O numero da versao mora SO aqui: trocar o nome do lancamento e uma linha.
#define N170_VERSAO "1.7"

void novidades170_dir(const char *dirArte);   // pasta da arte do pacote
void novidades170_primeira_vez(void);
int  novidades170_aberto(void);
void novidades170_abrir(void);
void novidades170_evento(const SDL_Event *e);
void novidades170_atualizar(float dt, Uint32 agora);
void novidades170_desenhar(Uint32 agora);

#define N170_PEDIU_NADA 0
#define N170_PEDIU_SPOT 1   // abrir o Spotlight por cima da home
int novidades170_pedido(void);

// Para a captura (tests/novidades170_shot.c): quantas cenas, qual esta na
// tela, e pular direto para uma delas com o relogio interno em `t` segundos.
int  novidades170_cenas(void);
int  novidades170_cena(void);
void novidades170_ir(int cena, float t);
// A lista: quantas linhas, e a largura da descricao da linha `i` no idioma
// atual contra o `limite` da coluna (a captura confere os 30 idiomas).
int  novidades170_itens(void);
int  novidades170_item_largura(int i, int *limite, const char **nome);
// 1 = a previa esta com o foco (cima), 0 = os botoes.
int  novidades170_foco_na_previa(void);
// Uma cena da previa fora do cartao (o "Ver exemplo" do Guia de uso): a cena
// `c` com o canto em (x, y), no tamanho da previa (N170_PREVIA_W x
// N170_PREVIA_H), no instante `t`.
#define N170_PREVIA_W 760.0f
#define N170_PREVIA_H novidades170_previa_altura()
float novidades170_previa_altura(void);
void  novidades170_cena_desenhar(int c, float x, float y, float t);

#endif
