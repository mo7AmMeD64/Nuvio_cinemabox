// TELA DE DESCANSO (2.0): o que aparece quando a tela fica parada, por cima do
// preto do esmaecer (esmaecer.h decide QUANDO; este modulo decide O QUE).
//
//   VITRINE  um titulo do catalogo em tela cheia, com aproximacao lenta, e as
//            informacoes entrando uma de cada vez: titulo (logo quando ha),
//            ano/duracao/genero e uma linha da sinopse. A cada DESC_ITEM_S a
//            tela respira no escuro e troca. OK abre o titulo (so na Home).
//   RELOGIO  hora grande em numeral fino, data, a proxima estreia da Agenda e
//            uma linha que enche ao longo do minuto. O bloco anda devagar pela
//            tela (OLED: nada fica no mesmo pixel).
//
// O update pede as texturas e escolhe os titulos; o desenho so le GLuint.
// Catalogo vazio (ou so arte sem fundo) cai no relogio.
#ifndef NV_DESCANSO_H
#define NV_DESCANSO_H

#define DESC_ITEM_S      20.0f   // quanto cada titulo fica na vitrine
#define DESC_ENTRA_S      2.2f   // a arte sai do preto
#define DESC_SAI_S        1.6f   // e volta ao preto no fim
#define DESC_FILA_MAX      48    // titulos sorteados por sessao de descanso

// Fontes da vitrine (indices de V_DESCANSO_FONTE).
#define DESC_FONTE_CATALOGO 0
#define DESC_FONTE_LISTA    1    // na lista + em andamento ("Continuar")

// Uma vez por quadro, depois de esmaecer_quadro. `ativo` = esmaecer_descanso().
void descanso_quadro(unsigned agora, float dt, int ativo, int estilo, int fonte);
// Por cima do veu preto. Chamado por esmaecer_desenhar.
void descanso_desenhar(unsigned agora);
// Indice de catalogo do titulo na vitrine agora, ou -1 (relogio, entre dois
// titulos, ou titulo ainda saindo do preto).
int  descanso_indice_em_tela(void);
// OK na vitrine: guarda o pedido; app.c consome e abre o titulo.
void descanso_pedir_abrir(void);
int  descanso_pedido_abrir(void);   // indice ou -1; CONSOME

// PURAS (teste): o item entra no filtro da fonte?
int  descanso_item_serve(int temFundo, int temTitulo, int naLista, int progresso,
                         int fonte);
// Posicao na rota do relogio (0..1 nos dois eixos) para o instante `s`.
void descanso_rota(float s, float *px, float *py);

#endif
