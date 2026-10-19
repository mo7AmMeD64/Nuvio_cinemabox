// Cartao de NOVIDADES DA 2.0.1 — no padrao aprovado da 1.4.8 (novidades148.c):
// a esquerda uma PREVIA VIVA do que mais mudou (Central de controle, Ajustes,
// velocidade), trocando a cada ~4 s; a direita as mudancas em grupos, uma
// linha apagada por item. A ultima pagina e "Apoie o projeto", com os QRs do
// Patreon e do Ko-fi (apoio.h): opcional, nunca no caminho de quem fecha.
//
// QUANDO ABRE: uma vez, na home, para quem JA viu o guia da 2.0. Quem ainda
// vai ver o guia (instalacao nova ou vindo da 1.x) recebe o guia e esta marca
// gravada: dois cartoes seguidos na primeira abertura e demais.
#ifndef NV_NOVIDADES201_H
#define NV_NOVIDADES201_H
#include <SDL2/SDL.h>

#define N201_VERSAO "2.0.1"
#define N201_ARQ    "novidades-201.txt"

// Pasta da arte do pacote (a mesma de app_iniciar): fundo da previa, sem rede.
void novidades201_dir(const char *dirArte);
// Chamar ANTES de novidades20_primeira_vez no mesmo quadro: a decisao olha a
// marca do guia da 2.0 antes de ele gravar a dele.
void novidades201_primeira_vez(void);
int  novidades201_aberto(void);
void novidades201_abrir(void);
void novidades201_evento(const SDL_Event *e);
void novidades201_atualizar(float dt, Uint32 agora);
void novidades201_desenhar(Uint32 agora);

#define N201_PEDIU_NADA    0
#define N201_PEDIU_CENTRAL 1   // abrir a Central de controle
int  novidades201_pedido(void);

// ---- Para a captura e os testes.
int  novidades201_pagina(void);        // 0 = novidades, 1 = apoie o projeto
int  novidades201_foco(void);
int  novidades201_previa_pronta(void); // a arte do fundo da previa carregou
// Pula o relogio da previa para `seg` segundos (cena e momento dela).
void novidades201_teste_relogio(float seg);
void novidades201_teste_esquecer(void);
float novidades201_teste_folga(void);   // rodape - fim da lista, ultimo quadro
int  novidades201_teste_cortadas(void);  // frases com reticencias, ultimo quadro   // testes: a decisao volta a valer
#endif
