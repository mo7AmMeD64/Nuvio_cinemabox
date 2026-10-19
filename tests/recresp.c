// "Ja assisti" numa recomendacao recebida (recresp.c), a marca automatica no
// fim do player (atividade.c) e a resposta pelo cartao dos creditos (reacao.c).
//
//   bash tests/recresp.sh
//
// Sem janela e sem rede: com NV_REC_URL de mentira o fio de recomenda.c nunca
// e ligado aqui (ninguem chama recomenda_verificar), entao nada sai.
#include "recresp.h"
#include "reacao.h"
#include "atividade.h"
#include "recomenda.h"
#include "catalogo.h"
#include "dados.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int falhas;
#define CHECA(c, msg) do { if (!(c)) { printf("FALHOU: %s (linha %d)\n", msg, __LINE__); falhas++; } } while (0)

static void tecla(SDL_Keycode k) {
  SDL_Event e;
  memset(&e, 0, sizeof e);
  e.type = SDL_KEYDOWN;
  e.key.keysym.sym = k;
  reacao_evento(&e, 0);
}

static void transicoes(void) {
  RecResp r;
  char j[256];
  memset(&r, 0, sizeof r);
  r.rec = 5;
  CHECA(recresp_aplicar(&r, RECRESP_EV_ASSISTIU, 0, NULL, 100), "assistiu muda");
  CHECA(r.assistida && !r.respondida && r.reacao == RECRESP_SEM_REACAO, "assistida sem resposta");
  CHECA(!recresp_aplicar(&r, RECRESP_EV_ASSISTIU, 0, NULL, 101), "assistiu de novo nao muda nada");
  CHECA(r.versao == 1, "versao so sobe com mudanca");
  recresp_json(&r, j, sizeof j);
  CHECA(!strcmp(j, "{\"id\":5,\"reacao\":null,\"texto\":\"\"}"), "json sem reacao");
  CHECA(recresp_aplicar(&r, RECRESP_EV_RESPONDEU, 1, "Valeu, AMEI!!", 102), "respondeu");
  CHECA(r.respondida && r.reacao == 1 && !strcmp(r.texto, "valeu amei"), "reacao e texto limpo");
  recresp_aplicar(&r, RECRESP_EV_RESPONDEU, RECRESP_SEM_REACAO, "", 103);
  CHECA(r.reacao == 1 && !strcmp(r.texto, "valeu amei"), "resposta vazia nao apaga");
  recresp_json(&r, j, sizeof j);
  CHECA(!strcmp(j, "{\"id\":5,\"reacao\":1,\"texto\":\"valeu amei\"}"), "json completo");
  memset(&r, 0, sizeof r);
  r.rec = 6;
  recresp_aplicar(&r, RECRESP_EV_PULOU, 0, NULL, 100);
  CHECA(r.assistida && r.respondida && r.reacao == RECRESP_SEM_REACAO, "pular = respondida sem reacao");
}

// O ESTADO DO SERVIDOR (GET /v1/rec) fundido na linha local.
static void servidor(void) {
  RecResp r;
  char corpo[1024];
  long long rec = 0;
  unsigned v = 0;
  memset(&r, 0, sizeof r);
  r.rec = 21;
  // Linha nova: assistida em outra TV, sem resposta.
  CHECA(recresp_mesclar(&r, 1, RECRESP_SEM_REACAO, "", 0, 500), "servidor traz assistida");
  CHECA(r.assistida && !r.respondida && r.reacao == RECRESP_SEM_REACAO, "so assistida");
  CHECA(r.versao == r.enviada, "fundida nao fica pendente");
  CHECA(!recresp_mesclar(&r, 1, RECRESP_SEM_REACAO, "", 0, 501), "o mesmo estado nao muda nada");
  // Resposta de outra TV, mais nova que a mudanca local.
  CHECA(recresp_mesclar(&r, 1, -1, "Nao curti", 900, 901), "resposta de outro aparelho");
  CHECA(r.respondida && r.reacao == -1 && !strcmp(r.texto, "nao curti") && r.quando == 900,
        "reacao e texto do servidor");
  // Servidor sem reacao nem texto nao apaga o que ha.
  CHECA(!recresp_mesclar(&r, 1, RECRESP_SEM_REACAO, "", 900, 902), "null nao apaga");
  CHECA(r.reacao == -1 && !strcmp(r.texto, "nao curti"), "reacao e texto intactos");
  // Resposta MAIS VELHA que a local confirmada: o local fica.
  r.quando = 1000;
  CHECA(!recresp_mesclar(&r, 1, 1, "amei", 950, 1001) && r.reacao == -1, "servidor velho nao vence");
  // Servidor mais novo vence.
  CHECA(recresp_mesclar(&r, 1, 1, "amei", 1100, 1101) && r.reacao == 1 &&
        !strcmp(r.texto, "amei"), "servidor mais novo vence");
  // Mudanca local NAO enviada fica ate o ack.
  memset(&r, 0, sizeof r);
  r.rec = 22;
  recresp_aplicar(&r, RECRESP_EV_RESPONDEU, 1, "valeu", 2000);   // versao 1, enviada 0
  CHECA(!recresp_mesclar(&r, 1, -1, "ruim", 3000, 3001), "pendente local nao e tocado");
  CHECA(r.reacao == 1 && !strcmp(r.texto, "valeu"), "local pendente intacto");
  r.enviada = r.versao;   // ack
  CHECA(recresp_mesclar(&r, 1, -1, "ruim", 3000, 3002) && r.reacao == -1, "depois do ack o servidor entra");

  // Pelo JSON da rede, no arquivo: id velho (atras do cursor) e campos nulos.
  recresp_esquecer();
  snprintf(corpo, sizeof corpo,
    "{\"cursor\":9,\"novas\":0,\"itens\":[],\"respostas\":["
    "{\"id\":31,\"terminou\":1,\"reacao\":null,\"resposta\":\"\",\"respondido\":0},"
    "{\"id\":32,\"terminou\":1,\"reacao\":0,\"resposta\":\"mais ou menos\",\"respondido\":777},"
    "{\"id\":0,\"terminou\":1}]}");
  recomenda_fundir_respostas(corpo);
  CHECA(recresp_assistida(31) && !recresp_respondida(31), "31 assistida em outra TV");
  { RecResp x;
    CHECA(recresp_ler(32, &x) && x.respondida && x.reacao == 0 &&
          !strcmp(x.texto, "mais ou menos") && x.quando == 777, "32 respondida em outra TV"); }
  CHECA(!recresp_pendente(corpo, sizeof corpo, &rec, &v), "o que veio do servidor nao e reenviado");
  { unsigned rv = recresp_revisao();
    recomenda_fundir_respostas(corpo);   // o mesmo corpo de novo
    CHECA(recresp_revisao() == rv, "repetir o corpo nao mexe na revisao"); }
  // Servidor antigo: sem os campos novos, nada acontece.
  recomenda_fundir_respostas("{\"cursor\":1,\"novas\":1,\"itens\":[{\"id\":40,\"imdb\":\"tt1\"}]}");
  CHECA(!recresp_assistida(40), "servidor antigo: sem estado, sem efeito");
  // Um item novo com os campos dentro do proprio item.
  recomenda_fundir_respostas("{\"itens\":[{\"id\":41,\"imdb\":\"tt2\",\"terminou\":1,"
                             "\"reacao\":1,\"resposta\":\"gostei\",\"respondido\":55}]}");
  CHECA(recresp_respondida(41), "campos no proprio item");
  // Gesto local depois de fundir sobe a versao e vai ao servidor.
  recresp_responder(31, 1, "agora sim");
  CHECA(recresp_pendente(corpo, sizeof corpo, &rec, &v) && rec == 31 && strstr(corpo, "agora sim"),
        "responder depois de fundir envia");
}

static void fila(void) {
  char corpo[256];
  long long rec = 0;
  unsigned v = 0;
  recresp_esquecer();
  CHECA(!recresp_pendente(corpo, sizeof corpo, &rec, &v), "vazio nao tem pendente");
  recresp_marcar_assistida(11);
  CHECA(recresp_assistida(11) && !recresp_respondida(11), "marcada");
  CHECA(recresp_pendente(corpo, sizeof corpo, &rec, &v) && rec == 11, "vai ao servidor");
  recresp_responder(11, -1, "nao curti");   // mudou DURANTE o envio
  recresp_confirmar(11, v);
  CHECA(recresp_pendente(corpo, sizeof corpo, &rec, &v) && strstr(corpo, "\"reacao\":-1"),
        "mudanca durante o envio continua pendente");
  recresp_confirmar(11, v);
  CHECA(!recresp_pendente(corpo, sizeof corpo, &rec, &v), "confirmada sai da fila");
  { char *b = dados_ler("recomendacoes-respostas.txt");
    CHECA(b && strstr(b, "11\t1\t1\t-1\t") && strstr(b, "nao curti"), "gravado no disco");
    free(b); }
}

static void fimNoPlayer(void) {
  CatItem ci;
  int i;
  recresp_esquecer();
  memset(&ci, 0, sizeof ci);
  snprintf(ci.imdb, sizeof ci.imdb, "tt0100");
  snprintf(ci.tipo, sizeof ci.tipo, "series");
  snprintf(ci.titulo, sizeof ci.titulo, "Serie");
  atividade_marcar_origem(42, "tt0100", "Ana");
  atividade_player_passo(&ci, 1, 3, 30.0, 3000.0, 1, 0, 0.1f);
  for (i = 0; i < 10; i++) atividade_player_passo(&ci, 1, 3, 1000.0 + i, 3000.0, 1, 0, 0.1f);
  CHECA(!recresp_assistida(42), "no meio do episodio ainda nao");
  atividade_player_passo(&ci, 1, 3, 2950.0, 3000.0, 1, 1, 0.1f);   // creditos
  CHECA(recresp_assistida(42), "fim de um EPISODIO da serie recomendada marca a rec");
  CHECA(!recresp_respondida(42), "marcar nao responde por ninguem");
  atividade_player_saiu(2950.0, 3000.0, 1);
  // Sem origem: nada.
  snprintf(ci.imdb, sizeof ci.imdb, "tt0200");
  snprintf(ci.tipo, sizeof ci.tipo, "movie");
  atividade_player_passo(&ci, 0, 0, 30.0, 6000.0, 1, 0, 0.1f);
  atividade_player_passo(&ci, 0, 0, 5900.0, 6000.0, 1, 0, 0.1f);   // 98 %
  atividade_player_saiu(5900.0, 6000.0, 0);
  CHECA(!recresp_assistida(43) && !recresp_assistida(0), "titulo sem origem nao marca nada");
}

static void cartao(void) {
  RecResp r;
  recresp_esquecer();
  // "Ja assisti" na aba: a aba marca, o cartao pergunta.
  recresp_marcar_assistida(7);
  CHECA(reacao_rec_abrir(7, "tt0300", "Filme", "movie", "", "Ana", 1500.0f), "abre");
  CHECA(reacao_painel_aberta() && reacao_passo() == 0, "passo da reacao");
  tecla(SDLK_RETURN);   // Gostei
  CHECA(recresp_ler(7, &r) && r.respondida && r.reacao == 1, "gostei vai a quem mandou");
  CHECA(reacao_passo() == 1, "com servico: oferece a mensagem no MESMO cartao");
  tecla(SDLK_RETURN);   // "Valeu pela dica!"
  CHECA(recresp_ler(7, &r) && !strcmp(r.texto, "valeu pela dica") && r.reacao == 1,
        "mensagem rapida sem perder a reacao");
  CHECA(!reacao_painel_aberta(), "fecha depois da mensagem");
  // Voltar no primeiro passo: assistida, sem resposta.
  recresp_marcar_assistida(8);
  reacao_rec_abrir(8, "tt0400", "Outro", "movie", "", "Bia", -1.0f);
  tecla(SDLK_ESCAPE);
  CHECA(recresp_assistida(8) && !recresp_respondida(8), "voltar nao responde");
  // "Agora nao".
  reacao_rec_abrir(8, "tt0400", "Outro", "movie", "", "Bia", -1.0f);
  tecla(SDLK_RIGHT); tecla(SDLK_RIGHT); tecla(SDLK_RETURN);   // Nao gostei
  tecla(SDLK_RIGHT); tecla(SDLK_RIGHT); tecla(SDLK_RETURN);   // Agora nao
  CHECA(recresp_ler(8, &r) && r.respondida && r.reacao == -1 && !r.texto[0], "agora nao");
}

int main(void) {
  const char *dir = getenv("NUVIO_DADOS");
  if (!dir || !*dir) { puts("recresp: NUVIO_DADOS ausente; recusando rodar"); return 1; }
  dados_iniciar(dir);
  if (strcmp(dados_dir(), dir)) { puts("recresp: dados_dir() nao e NUVIO_DADOS; recusando rodar"); return 1; }
  transicoes();
  servidor();
  fila();
  fimNoPlayer();
  cartao();
  if (falhas) { printf("recresp: %d falha(s)\n", falhas); return 1; }
  puts("recresp: ok");
  return 0;
}
