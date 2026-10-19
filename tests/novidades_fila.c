// A FILA DE PRIMEIRA VEZ DA 1.8.0 (novidadesfila.h): instalacao nova ve SO o
// cartao da 1.8.0 com "Boas-vindas ao Nuvio"; quem atualiza da 1.7.x ve SO o
// cartao da 1.8.0 com "Novidades da 1.8.0"; a segunda abertura nao ve nada.
// Cada cenario e um processo (os cartoes guardam a decisao em estaticos):
//   novidades_fila nova|atualizou|segunda  (NUVIO_DADOS = pasta de dados)
// tests/novidades_fila.sh roda os tres na ordem certa.
#include "novidades180.h"
#include "novidadesfila.h"
#include "dados.h"
#include "diagnostico.h"
#include "novidades.h"
#include "novidades11.h"
#include "novidades12.h"
#include "novidades13.h"
#include "novidades131.h"
#include "novidades132.h"
#include "novidades133.h"
#include "novidades134.h"
#include "novidades139.h"
#include "novidades1312.h"
#include "novidades142.h"
#include "novidades148.h"
#include "novidades151.h"
#include "novidades160.h"
#include "novidades170.h"
#include "novidades172.h"
#include "novidades174.h"
#include "recintro.h"
#include "registro.h"
#include "salvosintro.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int existe(const char *a) { char *s = dados_ler(a); int ok = s != NULL; free(s); return ok; }

// A FILA COMO O app.c A RODA: a 1.8.0 primeiro, depois cada cartao antigo.
// Devolve quantos cartoes antigos abriram.
static int fila(void) {
  int abertos = 0;
  novidades180_primeira_vez();
  diagnostico_intro_primeira_vez(); abertos += diagnostico_intro_aberto();
  registro_aviso_primeira_vez();
  sintro_primeira_vez();            abertos += sintro_aberto();
  novidades_primeira_vez();         abertos += novidades_aberto();
  novidades11_primeira_vez();       abertos += novidades11_aberto();
  novidades12_primeira_vez();       abertos += novidades12_aberto();
  novidades13_primeira_vez();       abertos += novidades13_aberto();
  novidades131_primeira_vez();      abertos += novidades131_aberto();
  novidades132_primeira_vez();      abertos += novidades132_aberto();
  novidades133_primeira_vez();      abertos += novidades133_aberto();
  novidades134_primeira_vez();      abertos += novidades134_aberto();
  novidades139_primeira_vez();      abertos += novidades139_aberto();
  novidades142_primeira_vez();      abertos += novidades142_aberto();
  novidades148_primeira_vez();      abertos += novidades148_aberto();
  novidades151_primeira_vez();      abertos += novidades151_aberto();
  novidades160_primeira_vez();      abertos += novidades160_aberto();
  novidades170_primeira_vez();      abertos += novidades170_aberto();
  novidades172_primeira_vez();      abertos += novidades172_aberto();
  novidades174_primeira_vez();      abertos += novidades174_aberto();
  novidades1312_primeira_vez();     abertos += novidades1312_aberto();
  recintro_primeira_vez();          abertos += recintro_aberto();
  return abertos;
}

int main(int argc, char **argv) {
  const char *modo = argc > 1 ? argv[1] : "";
  const char *dir = getenv("NUVIO_DADOS");
  int i;
  assert(dir && dir[0]);
  dados_iniciar(dir);
  assert(!strcmp(dados_dir(), dir));
  if (!strcmp(modo, "nova")) {
    for (i = 0; i < novidadesfila_n(); i++) assert(!existe(novidadesfila_arquivo(i)));
    assert(fila() == 0);
    assert(novidades180_aberto());
    assert(novidades180_boas_vindas());            // "Boas-vindas ao Nuvio"
    assert(novidades180_foco() == 1);               // foco em "Abrir o guia"
    for (i = 0; i < novidadesfila_n(); i++) assert(existe(novidadesfila_arquivo(i)));
    puts("PASS: instalacao nova -> so o cartao da 1.8.0, com Boas-vindas");
  } else if (!strcmp(modo, "atualizou")) {
    // Quem vinha da 1.7.x: viu o cartao da 1.7 e nunca abriu outros (o pior caso).
    assert(dados_gravar("novidades-170-ui.txt", "1\n"));
    assert(fila() == 0);
    assert(novidades180_aberto());
    assert(!novidades180_boas_vindas());            // "Novidades da 1.8.0"
    assert(novidades180_foco() == 2);               // foco em "Vidro ou sólido"
    for (i = 0; i < novidadesfila_n(); i++) assert(existe(novidadesfila_arquivo(i)));
    puts("PASS: atualizacao da 1.7.x -> so o cartao da 1.8.0, com Novidades");
  } else if (!strcmp(modo, "fechar")) {
    // Abre e fecha (Voltar): grava a marca da 1.8.0.
    SDL_Event e;
    novidades180_primeira_vez();
    assert(novidades180_aberto());
    memset(&e, 0, sizeof e);
    e.type = SDL_KEYDOWN; e.key.keysym.sym = SDLK_ESCAPE;
    novidades180_evento(&e);
    assert(!novidades180_aberto());
    assert(existe(NF_ARQ_180));
    puts("PASS: Voltar fecha e grava a marca da 1.8.0");
  } else if (!strcmp(modo, "segunda")) {
    assert(existe(NF_ARQ_180));
    assert(novidadesfila_preparar() == NF_NADA);
    assert(fila() == 0);
    assert(!novidades180_aberto());
    puts("PASS: segunda abertura -> nenhum cartao");
  } else return 2;
  return 0;
}
