// Simkl e Letterboxd ligados a pessoa (F08, linhas da aba Amigos): fila de UM
// pedido por vez, estado por servico, 501 desliga o Simkl sem nova tentativa,
// declarar/desligar o Letterboxd, e o token do Simkl nunca no log nem no estado.
//
//   bash tests/recomenda_identidade_servicos.sh
//
// Mesma receita de tests/recomenda_social.c: inclui o .c, intercepta a rede.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NV_REC_URL "http://127.0.0.1:8799"
#define rede_baixar_etag  teste_rede_etag
#define rede_postar_st    teste_rede_postar
#define rede_baixar_st    teste_rede_st
#define rede_baixar_com   teste_rede_com
#include "../src/recomenda.c"

static int  respStatus = 200;
static char ultimaUrl[600], ultimoCorpo[2400];
static int  nPost;
static char *dup(const char *s) { char *r = malloc(strlen(s) + 1); strcpy(r, s); return r; }
char *teste_rede_etag(const char *u, int seg, const char *const *cab, int *st, char *etag, unsigned te) {
  (void)u; (void)seg; (void)cab; (void)etag; (void)te; if (st) *st = 200; return dup("{}");
}
char *teste_rede_postar(const char *u, int seg, const char *const *cab, const char *c, int *st) {
  (void)seg; (void)cab; nPost++;
  snprintf(ultimaUrl, sizeof ultimaUrl, "%s", u);
  snprintf(ultimoCorpo, sizeof ultimoCorpo, "%s", c ? c : "");
  if (st) *st = respStatus;
  return dup("{}");
}
char *teste_rede_st(const char *u, int seg, const char *const *cab, int *st) {
  (void)u; (void)seg; (void)cab; if (st) *st = 200; return dup("{}");
}
char *teste_rede_com(const char *u, int seg, const char *const *cab) { (void)u; (void)seg; (void)cab; return dup("{}"); }

static int falhas;
#define CONFERE(c, ...) do { if (!(c)) { falhas++; printf("FALHA: " __VA_ARGS__); printf("\n"); } } while (0)

// Roda o fio uma vez (o que o laco da rede faz ao ver o pedido).
static void roda(const char **cab) { tratarIdentidade(cab); }

int main(void) {
  const char *dir = getenv("NUVIO_DADOS");
  const char *cab[4] = { "Authorization: Bearer x", "X-Nuvio-Auth: nuvio", NULL, NULL };
  FILE *f;
  char arq[600];
  if (!dir || !dir[0]) { printf("identidade_servicos: sem NUVIO_DADOS; recusando\n"); return 2; }
  dados_iniciar(dir);
  if (strcmp(dados_dir(), dir)) { printf("identidade_servicos: dados_dir errado; recusando\n"); return 2; }
  mtx = SDL_CreateMutex();

  // Servidor antigo (sem identidade1): nada e oferecido e nada enfileira.
  CONFERE(recomenda_identidade_estado(REC_IDENT_SIMKL) == REC_IDENT_E_INDISPONIVEL, "old server: simkl unavailable");
  CONFERE(recomenda_identidade_estado(REC_IDENT_LETTERBOXD) == REC_IDENT_E_INDISPONIVEL, "old server: letterboxd unavailable");
  CONFERE(!recomenda_identidade_letterboxd_declarar("henrique"), "old server: declare refused");
  CONFERE(recomenda_identidade_estado(99) == REC_IDENT_E_INDISPONIVEL, "bad service id is safe");

  identRecurso = 1;
  // Letterboxd: sempre que o servidor sabe; Simkl: so com login ou ja ligado.
  CONFERE(recomenda_identidade_estado(REC_IDENT_LETTERBOXD) == REC_IDENT_E_PODE, "letterboxd offered");
  CONFERE(recomenda_identidade_estado(REC_IDENT_SIMKL) == REC_IDENT_E_INDISPONIVEL, "simkl hidden without a simkl login");
  identSimklLig = 1;
  CONFERE(recomenda_identidade_estado(REC_IDENT_SIMKL) == REC_IDENT_E_LIGADO, "linked simkl can be unlinked without a login");
  identSimklLig = 0;
  snprintf(arq, sizeof arq, "%s/simkl-p1.txt", dir);
  f = fopen(arq, "w"); fputs("SEGREDO-DO-SIMKL\n", f); fclose(f);
  simklauth_carregar_perfil(1);
  CONFERE(recomenda_identidade_estado(REC_IDENT_SIMKL) == REC_IDENT_E_PODE, "simkl offered with a login on this TV");

  // Usuario invalido / normalizacao.
  CONFERE(!recomenda_identidade_letterboxd_declarar("a"), "too short refused");
  CONFERE(!recomenda_identidade_letterboxd_declarar("!!"), "only junk refused");
  CONFERE(!recomenda_identidade_letterboxd_declarar(NULL), "null refused");
  CONFERE(recomenda_identidade_op_de(REC_IDENT_LETTERBOXD) == REC_IDENT_OP_NADA, "refusals leave no op");

  // Declarar: normaliza, enfileira, UM pedido por vez.
  CONFERE(recomenda_identidade_letterboxd_declarar("  Hen_Rique9 !"), "declare queued");
  CONFERE(recomenda_identidade_op_de(REC_IDENT_LETTERBOXD) == REC_IDENT_OP_INDO, "letterboxd op running");
  CONFERE(!recomenda_identidade_simkl_unir(), "second service waits while one is in flight");
  CONFERE(!recomenda_identidade_unir(), "trakt waits too");
  CONFERE(!recomenda_identidade_letterboxd_declarar("outro"), "same service waits too");
  CONFERE(recomenda_identidade_op_de(REC_IDENT_SIMKL) == REC_IDENT_OP_NADA, "rejected request leaves no op");
  respStatus = 200; nPost = 0;
  roda(cab);
  CONFERE(nPost == 1 && strstr(ultimaUrl, "/v1/identidades/vincular"), "vincular called (%s)", ultimaUrl);
  CONFERE(strstr(ultimoCorpo, "\"provedor\":\"letterboxd\"") && strstr(ultimoCorpo, "\"usuario\":\"hen_rique9\""),
          "body has normalized username (%s)", ultimoCorpo);
  CONFERE(recomenda_identidade_op_de(REC_IDENT_LETTERBOXD) == REC_IDENT_OP_OK, "declare ok");
  CONFERE(!strcmp(recomenda_identidade_usuario_letterboxd(), "hen_rique9"), "username reflected at once");
  CONFERE(recomenda_identidade_estado(REC_IDENT_LETTERBOXD) == REC_IDENT_E_LIGADO, "letterboxd linked");
  recomenda_identidade_op_limpar_de(REC_IDENT_LETTERBOXD);
  CONFERE(recomenda_identidade_op_de(REC_IDENT_LETTERBOXD) == REC_IDENT_OP_NADA, "op cleared");

  // Desligar o Letterboxd.
  CONFERE(recomenda_identidade_letterboxd_separar(), "unlink queued");
  roda(cab);
  CONFERE(strstr(ultimaUrl, "/v1/identidades/desvincular") && strstr(ultimoCorpo, "\"provedor\":\"letterboxd\""),
          "desvincular letterboxd (%s %s)", ultimaUrl, ultimoCorpo);
  CONFERE(recomenda_identidade_usuario_letterboxd()[0] == 0, "username cleared");
  CONFERE(recomenda_identidade_estado(REC_IDENT_LETTERBOXD) == REC_IDENT_E_PODE, "letterboxd offered again");
  recomenda_identidade_op_limpar_de(REC_IDENT_LETTERBOXD);

  // Simkl: o token do app vai no corpo, mas nunca fica no estado.
  CONFERE(recomenda_identidade_simkl_unir(), "simkl link queued");
  respStatus = 200; roda(cab);
  CONFERE(strstr(ultimoCorpo, "\"provedor\":\"simkl\"") && strstr(ultimoCorpo, "SEGREDO-DO-SIMKL"), "simkl body has the token");
  CONFERE(recomenda_identidade_op_de(REC_IDENT_SIMKL) == REC_IDENT_OP_OK && identSimklLig, "simkl linked");
  recomenda_identidade_op_limpar_de(REC_IDENT_SIMKL);

  // Desligar o Simkl; 409 = conta de outro perfil.
  CONFERE(recomenda_identidade_simkl_separar(), "simkl unlink queued");
  roda(cab);
  CONFERE(strstr(ultimaUrl, "desvincular") && !identSimklLig, "simkl unlinked");
  recomenda_identidade_op_limpar_de(REC_IDENT_SIMKL);
  recomenda_identidade_simkl_unir(); respStatus = 409; roda(cab);
  CONFERE(recomenda_identidade_op_de(REC_IDENT_SIMKL) == REC_IDENT_OP_CONFLITO && !identSimklLig, "409 = conflict, not linked");
  recomenda_identidade_op_limpar_de(REC_IDENT_SIMKL);
  recomenda_identidade_simkl_unir(); respStatus = 401; roda(cab);
  CONFERE(recomenda_identidade_op_de(REC_IDENT_SIMKL) == REC_IDENT_OP_RECUSADO, "401 = refused");
  recomenda_identidade_op_limpar_de(REC_IDENT_SIMKL);
  recomenda_identidade_simkl_unir(); respStatus = 500; roda(cab);
  CONFERE(recomenda_identidade_op_de(REC_IDENT_SIMKL) == REC_IDENT_OP_FALHA, "500 = failure, can retry");
  recomenda_identidade_op_limpar_de(REC_IDENT_SIMKL);

  // 501: o servidor nao tem SIMKL_CLIENT_ID. Aviso uma vez, depois some, sem
  // nova tentativa; o Letterboxd segue funcionando.
  recomenda_identidade_simkl_unir(); respStatus = 501; nPost = 0; roda(cab);
  CONFERE(recomenda_identidade_op_de(REC_IDENT_SIMKL) == REC_IDENT_OP_SEM_SERVICO, "501 reported");
  CONFERE(recomenda_identidade_estado(REC_IDENT_SIMKL) == REC_IDENT_E_SEM_SERVICO, "simkl off after 501");
  CONFERE(nPost == 1, "exactly one request");
  recomenda_identidade_op_limpar_de(REC_IDENT_SIMKL);
  CONFERE(recomenda_identidade_estado(REC_IDENT_SIMKL) == REC_IDENT_E_SEM_SERVICO, "stays off after the message is dismissed");
  CONFERE(recomenda_identidade_estado(REC_IDENT_LETTERBOXD) == REC_IDENT_E_PODE, "letterboxd unaffected");

  // Trocar de conta/perfil esquece tudo.
  identLbUsuario[0] = 'x'; identSimklLig = 1;
  recomenda_esquecer();
  CONFERE(!identSimklOff && !identSimklLig && !identLbUsuario[0] && !identOcupado(), "forget resets per-service state");
  CONFERE(recomenda_identidade_estado(REC_IDENT_SIMKL) == REC_IDENT_E_INDISPONIVEL, "after forget: unavailable");

  printf(falhas ? "identidade_servicos: %d falha(s)\n" : "identidade_servicos: OK\n", falhas);
  return falhas ? 1 : 0;
}
