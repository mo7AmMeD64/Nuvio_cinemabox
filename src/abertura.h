// ABERTURA DO APP (#213): a marca do Nuvio por cima dos primeiros quadros.
//
// O relato da G5 (webOS 10.3): "abre uma tela cinza com bolinhas paradas, cara
// de app demo". O cinza e o splash PADRAO do sistema (splashColor "gray" na
// referencia do appinfo.json) e as bolinhas o spinner dele, parado enquanto o
// app compila os shaders — no registro, 1,3 s entre gfx_iniciar e as fontes.
//
// Duas pecas, uma imagem so:
//   1. ANTES do app: o sistema mostra deploy/app/splash.png (appinfo
//      splashBackground). Android: windowBackground (res/drawable/abertura.xml).
//      Tizen: o #abertura do tools/tizen-shell.html.
//   2. NO APP: o primeiro quadro ja nasce com a MESMA imagem (1.7.2: a arte
//      cheia, art/marcas/abertura.jpg; sem ela, a marca de 780 px no centro
//      sobre #0E0F12) — sem salto da imagem do sistema para o app —, respira
//      de leve e abre para a home com uma mola.
//      Um quad de cor e um de textura por quadro, e so ate a saida acabar.
//
// O app nao espera por ela: a home monta por baixo desde o primeiro quadro, e a
// saida comeca quando as artes pedidas chegam (ou no teto, ABERTURA_TETO_MS).
// Uma tecla durante a abertura antecipa a saida.
#ifndef NV_ABERTURA_H
#define NV_ABERTURA_H
#include <SDL2/SDL.h>
#include "gfx.h"

// Carrega art/marcas/abertura.jpg, ou a marca (nuvio_wordmark.png). Sem os dois, a abertura
// vira so o fundo esvanecendo. Depois de gfx_iniciar (precisa de contexto GL).
void abertura_iniciar(const char *dirArte);
// Desenha por cima do quadro; 0 quando ja acabou (e a textura foi liberada).
// `pendentes` = artes em voo no cache (tex_estatisticas).
int  abertura_desenhar(Uint32 agora, float dt, int pendentes);
// 1 = a tela de baixo e o login, cujo fundo e da mesma familia do splash: a
// saida nao cresce, so dissolve, e o que some e o logo — as listras ficam.
void abertura_fundo_fica(int sim);
// Uma tecla chegou: a saida comeca ja.
void abertura_tecla(void);
// 1 enquanto ainda cobre a tela.
int  abertura_ativa(void);

// Previa viva para os Ajustes: uma volta da abertura em laco em `r` (estilo 0..2 =
// Padrao/So esmaece/Direto; logo = LOGO_NOVO/LOGO_CLASSICO), `agora` em ms. Desenha
// so o veu e a marca: o que fica por baixo e de quem chama.
// `raio` = raio dos cantos em fracao da altura (como gfx_cor).
void abertura_previa(GfxRect r, float raio, int estilo, int logo, Uint32 agora, float alfa);

#endif
