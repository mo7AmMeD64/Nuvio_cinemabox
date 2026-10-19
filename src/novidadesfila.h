// A FILA DE PRIMEIRA VEZ A PARTIR DA 1.8.0: um cartao so.
//
// Ate a 1.7 cada cartao de novidades gravava a sua marca (dados_gravar) e
// app.c encadeava todos os *_primeira_vez(): quem instalava do zero recebia a
// fila inteira, da 1.1 a 1.7, um cartao atras do outro. Da 1.8.0 em diante o
// Guia de uso (Ajustes › Sobre e ajuda) explica o app inteiro, e o dono
// aprovou que ele substitua essa fila: instalacao nova ve SO o cartao da 1.8.0
// (com "Boas-vindas ao Nuvio"), e quem atualiza tambem ve so ele (com
// "Novidades da 1.8.0").
//
// COMO: antes de qualquer *_primeira_vez() da fila, novidadesfila_preparar()
// olha as marcas antigas. Nenhuma = instalacao nova; alguma = atualizacao.
// Nos dois casos grava TODAS as que faltam, com o mesmo conteudo que o cartao
// dono dela grava, e a fila antiga passa a nao ter nada a mostrar. Com a marca
// da 1.8.0 ja gravada nao faz nada (o cartao ja foi visto).
//
// O QUE ENTRA: os cartoes de versao (novidades*.c) e os explicadores cujo
// assunto o guia cobre — o da tecla de Salvos (salvosintro.c), o do registro
// (registro.c), o da tela social (recintro.c) e a apresentacao do diagnostico
// (diagnostico.c). O QUE NAO ENTRA: a pergunta da telemetria (consentimento,
// tem de ser feita) e a do PiP (aparece no player, quando o assunto acontece).
#ifndef NV_NOVIDADESFILA_H
#define NV_NOVIDADESFILA_H

enum {
  NF_NADA = 0,       // o cartao da 1.8.0 ja foi visto: nada a fazer
  NF_ATUALIZOU = 1,  // havia marca antiga: "Novidades da 1.8.0"
  NF_NOVA = 2        // nenhuma marca: instalacao nova, "Boas-vindas ao Nuvio"
};

// A marca do cartao da 1.8.0 (gravada por novidades180.c ao fechar).
#define NF_ARQ_180 "novidades-180-ui.txt"

// Decide e grava as marcas antigas. Chamar UMA vez, antes da fila.
int novidadesfila_preparar(void);
// Quantas marcas antigas a fila conhece, e o nome da i-esima (para o teste).
int novidadesfila_n(void);
const char *novidadesfila_arquivo(int i);
#endif
