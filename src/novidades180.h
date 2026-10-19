// Cartao de NOVIDADES DA 1.8.0 (Glass UI): o app inteiro no material da ilha
// do relogio. Mockup aprovado pelo dono (variante A, ilha modal no centro):
// previa viva a esquerda com tres cenas montadas nas pecas do Glass UI (a
// folha de Fontes, o pedido de amizade da ilha do relogio, a busca de Ajustes
// com o grafico de memoria), a lista agrupada a direita e tres pilulas: Agora
// nao, Abrir o guia e a principal, Vidro ou solido.
//
// E O UNICO CARTAO DE PRIMEIRA VEZ da 1.8.0 em diante (novidadesfila.h):
// quem instala do zero ve a variante "Boas-vindas ao Nuvio", quem atualiza ve
// "Novidades da 1.8.0".
#ifndef NV_NOVIDADES180_H
#define NV_NOVIDADES180_H
#include <SDL2/SDL.h>

// O numero da versao mora SO aqui.
#define N180_VERSAO "1.8.0"

void novidades180_dir(const char *dirArte);   // pasta da arte do pacote
// Uma vez, com a home de pe e antes da fila antiga: prepara a fila
// (novidadesfila_preparar) e abre o cartao se a 1.8.0 ainda nao foi vista.
void novidades180_primeira_vez(void);
int  novidades180_aberto(void);
// Abre na variante pedida: 0 = "Novidades da 1.8.0", 1 = "Boas-vindas".
void novidades180_abrir(int boasVindas);
// De volta do Guia ("Voltar às novidades"): a mesma variante, foco no guia.
void novidades180_reabrir(void);
void novidades180_evento(const SDL_Event *e);
void novidades180_atualizar(float dt, Uint32 agora);
void novidades180_desenhar(Uint32 agora);

#define N180_PEDIU_NADA  0
#define N180_PEDIU_GUIA  1   // Ajustes › Sobre e ajuda › Guia de uso (vindo daqui)
#define N180_PEDIU_VIDRO 2   // Ajustes › Aparência › Interface de vidro
int novidades180_pedido(void);

// Para a captura e os testes.
int  novidades180_boas_vindas(void);   // a variante aberta
int  novidades180_cenas(void);
int  novidades180_cena(void);
void novidades180_ir(int cena, float t);
int  novidades180_foco(void);           // 0 Agora nao, 1 Abrir o guia, 2 Vidro ou solido
int  novidades180_foco_na_previa(void);
int  novidades180_itens(void);
int  novidades180_item_largura(int i, int *limite, const char **nome);

// As cenas da previa, para quem quer mostrar uma delas (o "Ver exemplo" do
// Guia de uso): desenha a cena `c` no espaco virtual de N180_CENA_W x
// N180_CENA_H unidades a partir de (0, 0) — quem chama poe uma miniatura
// (gfx_mini_*) em volta. `t` em segundos.
#define N180_CENA_W 972.0f
#define N180_CENA_H 1278.0f
enum { N180_CENA_FONTES = 0, N180_CENA_ILHA, N180_CENA_AJUSTES, N180_NCENAS };
void novidades180_cena_desenhar(int c, float t);

#endif
