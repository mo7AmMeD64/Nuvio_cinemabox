// LOGO DO APP (2.0, N1) — o simbolo que o Nuvio desenha como marca propria.
//
// Duas escolhas, ambas em Ajustes > Aparencia > "Logo e abertura", para todo mundo
// (sem a trava de apoiador do icone) e LOCAIS desta TV (logoAppLocal):
//   Novo     = o play arredondado em degrade (art/marcas/logo-novo-*.png), padrao;
//   Classico = a TV retro de sempre (art/marcas/abertura.jpg na abertura,
//              logo-classico.png nas miniaturas, nuvio_wordmark.png no Sobre).
//
// O QUE ISTO NAO E: o icone do launcher/pacote (appinfo.json, config.xml, manifest
// do .tpk, aliases do Android) e a galeria de apoiadores (iconeapp.h). Esses seguem
// como estao; um icone alternativo de apoiador em vigor tem PRIORIDADE sobre o logo
// nas telas de entrada (logoapp_marca).
#ifndef NV_LOGOAPP_H
#define NV_LOGOAPP_H
#include "gfx.h"

enum { LOGO_NOVO = 0, LOGO_CLASSICO = 1, LOGO_N = 2 };
// Qual arquivo de cada logo.
enum {
  LOGO_F_SIMBOLO,     // so o simbolo (miniatura, cantos das telas de entrada)
  LOGO_F_MARCA,       // simbolo + nome empilhados (abertura)
  LOGO_F_HORIZONTAL   // simbolo + nome lado a lado (Sobre)
};

void logoapp_iniciar(const char *dirArte);
// O logo EM VIGOR (ajustes_logo_app()).
int  logoapp_atual(void);
// Nome na lista de valores ("Novo", "Clássico"); "" fora da faixa.
const char *logoapp_nome(int logo);
// Caminho absoluto da arte (buffer proprio por logo/forma).
const char *logoapp_caminho(int logo, int forma);
// Miniatura quadrada do logo em `r` (o seletor de 64 px). `ladrilho` = 1 pinta o
// mesmo fundo escuro arredondado da galeria de icones. 0 enquanto a textura nao chegou.
int  logoapp_miniatura(int logo, GfxRect r, int ladrilho, float alpha);
// A marca propria nas telas de entrada (login, perfis, "preparando"): o icone
// alternativo de apoiador, se houver; senao o simbolo do logo Novo; com o Classico
// nada (como era). Devolve 1 se desenhou.
int  logoapp_marca(GfxRect r, float alpha);
#endif
