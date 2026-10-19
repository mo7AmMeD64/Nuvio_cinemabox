// O LOGO DO TITULO NO CABECALHO DE UMA MODAL, no lugar do nome escrito.
//
// Pedido do dono (01/10/2026): "nos modais que tiverem o nome do filme, trocar
// pela logo do filme". Menu de opcoes do cartaz, modal de recomendar, cartao de
// recomendacao recebida e o menu da temporada desenhavam o nome em texto, com
// a mesma arte de logo ja baixada pelo destaque e pelo detalhe.
//
// QUAL LOGO: a mesma do hero/detalhe — escolha a mao (#142) primeiro, depois a
// da sessao aberta, depois o `logo` do item (que ja respeita "Logo do addon",
// descoberta.c), no tamanho do TMDB que cobre `maxW` (artehero_logo_sessao_larg).
//
// A CAIXA E RESERVADA: o logo encosta a esquerda e centra na altura `maxH`, e
// sem logo (ausente, ainda carregando ou falhado) o nome em texto ocupa a
// MESMA caixa — o cabecalho nao pula quando o arquivo chega.
//
// LOGO ESCURO VIRA BRANCO pela luminancia medida (tex_marca_escura), a mesma
// regra do hero, do card aberto e do detalhe.
#ifndef NV_LOGOTITULO_H
#define NV_LOGOTITULO_H

#include "catalogo.h"
#include "text.h"

// Desenha logo ou nome na caixa (x, y, maxW, maxH). `estilo` e o estilo do
// texto de reserva, cortado em `largTexto` (0 = maxW): o nome pode usar a
// largura toda do cartao, o logo nao. 1 = desenhou o logo, 0 = o nome.
int logotitulo_desenhar(const CatItem *ci, const char *nome, TxtEstilo estilo,
                        float x, float y, float maxW, float maxH,
                        float largTexto, float a);

// So a url (ou NULL), para quem precisa saber se ha logo antes de desenhar.
const char *logotitulo_url(const CatItem *ci, float maxW);

#endif
