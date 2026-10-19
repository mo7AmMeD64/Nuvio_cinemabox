// A FILA DA ILHA COM PRIORIDADE (ilha.h, mockup aprovado em 02/10): P1 fura a
// fila, FIFO dentro da mesma prioridade, "+N" com dois ou mais esperando, os
// avisos da central que esperam viram um "N avisos novos", mesma chave troca
// no lugar, retirar a central limpa so os dela.
//
// Sem janela e sem GL: so a fila. ilha.c e ligado com -undefined
// dynamic_lookup (o desenho nunca e chamado aqui) e i18n devolve o portugues.
#include "ilha.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

const char *i18n(const char *s) { return s; }
// Os dados que ilha.c cita (o codigo que os usa nao roda aqui).
float gfx_tex_aspect_atual;
float gfx_card_forcar_cover_atual;
int txt_pendentes;

static void av(const char *chave, int tipo, int grupo) {
  IlhaAvisoEx e;
  memset(&e, 0, sizeof e);
  e.chave = chave; e.tipo = tipo; e.texto = chave; e.ms = 4000; e.grupo = grupo;
  ilha_avisar_ex(&e);
}
#define VEZ(c) assert(!strcmp(ilha_aviso_vez(), (c)))
static void passa(void) { ilha_retirar(ilha_aviso_vez()); }

int main(void) {
  // 1. Sem nada: vazia.
  VEZ("");
  assert(ilha_esperando() == 0);

  // 2. FIFO dentro da mesma prioridade; P3 (ok/info) atras de P2 (acento).
  av("info-a", ILHA_INFO, 0);
  av("ok-b", ILHA_OK, 0);
  av("acento-c", ILHA_ACENTO, 0);
  VEZ("info-a");
  assert(ilha_esperando() == 2);
  passa(); VEZ("acento-c");     // P2 passou o P3 que chegou antes
  passa(); VEZ("ok-b");
  passa(); VEZ("");

  // 3. ERRO FURA A FILA e entra ja; o que estava (fora da central) volta para
  //    a frente da fila e reaparece depois.
  av("info-a", ILHA_INFO, 0);
  av("acento-b", ILHA_ACENTO, 0);
  av("erro", ILHA_ERRO, 0);
  VEZ("erro");
  assert(ilha_esperando() == 2);
  passa(); VEZ("acento-b");     // P2 na frente do P3 devolvido
  passa(); VEZ("info-a");
  passa(); VEZ("");

  // 4. ERRO NAO FURA ERRO: P1 atras de P1 espera a vez.
  av("erro1", ILHA_ERRO, 0);
  av("erro2", ILHA_ERRO, 0);
  VEZ("erro1");
  passa(); VEZ("erro2");
  passa(); VEZ("");

  // 5. A CENTRAL: o primeiro diz o assunto; chegando mais dois, os que esperam
  //    viram UM "N avisos novos" (chave "avisos") e contam 2 no "+N".
  av("av:rec", ILHA_ACENTO, 1);
  av("av:update:1.7.2", ILHA_ACENTO, 1);
  VEZ("av:rec");
  assert(ilha_esperando() == 1);      // um so esperando: continua com o assunto dele
  assert(ilha_tem("av:update:1.7.2"));
  av("av:canal:x", ILHA_INFO, 1);
  assert(!ilha_tem("av:update:1.7.2") && !ilha_tem("av:canal:x"));
  assert(ilha_tem("avisos"));
  assert(ilha_esperando() == 2);      // "+2"
  //    Um erro chega: fura, e o da central na tela NAO volta (ja foi visto).
  av("trakt", ILHA_ERRO, 0);
  VEZ("trakt");
  assert(ilha_esperando() == 2);
  assert(!ilha_tem("av:rec"));
  //    Mais um da central (a queda, ERRO) chega com o Trakt na tela: P1 nao
  //    fura P1, e como ja havia o resumo da central esperando, junta-se a ele
  //    ("3 avisos novos") em vez de passar sozinho.
  av("av:crash:1", ILHA_ERRO, 1);
  VEZ("trakt");
  passa();
  VEZ("avisos");
  assert(ilha_esperando() == 0);
  passa(); VEZ("");

  // 6. MESMA CHAVE troca no lugar (na tela e na fila), sem duplicar.
  av("rede", ILHA_ERRO, 0);
  av("x", ILHA_INFO, 0);
  av("rede", ILHA_OK, 0);            // "internet de volta" sobre "sem internet"
  VEZ("rede");
  assert(ilha_esperando() == 1);
  av("x", ILHA_INFO, 0);
  assert(ilha_esperando() == 1);
  passa(); passa(); VEZ("");

  // 7. A CENTRAL ABRIU: saem os dela (tela e fila), ficam os outros.
  av("av:rec", ILHA_ACENTO, 1);
  av("perfil", ILHA_INFO, 0);
  av("av:update:1.7.2", ILHA_ACENTO, 1);
  ilha_retirar_grupo();
  VEZ("perfil");
  assert(ilha_esperando() == 0);
  passa();

  // 8. FILA CHEIA (6): o de prioridade mais baixa cede; um P3 a mais nao entra.
  { int i; char k[16];
    av("vez", ILHA_ACENTO, 0);
    for (i = 0; i < 6; i++) { snprintf(k, sizeof k, "p2-%d", i); av(k, ILHA_ACENTO, 0); }
    assert(ilha_esperando() == 6);
    av("p3", ILHA_INFO, 0);
    assert(!ilha_tem("p3"));
    av("erro", ILHA_ERRO, 0);          // fura: "vez" volta, e o ultimo P2 sai
    VEZ("erro");
    assert(ilha_tem("vez") && !ilha_tem("p2-5") && ilha_tem("p2-4"));
    while (ilha_aviso_vez()[0]) passa(); }

  puts("PASS: fila da ilha (prioridade, +N, central, chave, cheia).");
  return 0;
}
