#ifndef NV_CONTINUAR_H
#define NV_CONTINUAR_H
#include "catalogo.h"
#include "gfx.h"

// Conteudo sobre a capa; a home continua responsavel por imagem e foco.
// `raio`: o MESMO raio da arte do cartao (fracao da altura, raioDe em home.c).
void continuar_desenhar(const CatItem *item, GfxRect card, float raio);
// O veu da base que continuar_desenhar pede (gfx_veu_base): a home o poe
// dentro da arte (gfx_veu_card_por) antes de desenha-la.
#define NV_CW_VEU_F 0.66f
#define NV_CW_VEU_A 0.88f
#endif
