// Mapa de filmes vistos do Trakt: paginação, falhas e last_activities.
// Sem conta, SDL ou rede. Usa o carregador real com respostas deterministicas.
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/trakt.c"

static int falhas, nMapa, nPedidos, nAtividades, erroPagina, statusAtividade;
static int totalServidor = 137, porPagina = 100;
static int repetirPagina, primeiraSemId, trocarNaPagina;
static unsigned long long geracaoMapa = 1;
static const char *atividade = "2026-10-02T10:00:00.000Z";
static const char *corpoPagina;
static struct { char id[24]; int visto; } mapa[2048];

static void verifica(int ok, const char *caso) {
  if (!ok) { fprintf(stderr, "FAIL: %s\n", caso); falhas++; }
}
unsigned long long cat_historico_geracao(void) { return geracaoMapa; }
int cat_historico_definir_se_geracao(const char *id, const char *tipo, int visto,
                                     unsigned long long geracao);
void cat_historico_definir_id(const char *id, const char *tipo, int visto) {
  int i;
  assert(!strcmp(tipo, "movie"));
  for (i = 0; i < nMapa; i++) if (!strcmp(mapa[i].id, id)) break;
  assert(i < (int)(sizeof mapa / sizeof mapa[0]));
  if (i == nMapa) { snprintf(mapa[i].id, sizeof mapa[i].id, "%s", id); nMapa++; }
  mapa[i].visto = visto;
}
int cat_historico_definir_se_geracao(const char *id, const char *tipo, int visto,
                                     unsigned long long geracao) {
  if (geracao != geracaoMapa) return 0;
  cat_historico_definir_id(id, tipo, visto);
  return 1;
}
static char *pagina(int inicio, int quantidade) {
  size_t cap = (size_t)quantidade * 100 + 3, usado = 0;
  char *s = malloc(cap);
  assert(s);
  s[usado++] = '[';
  for (int i = 0; i < quantidade; i++)
    usado += (size_t)snprintf(s + usado, cap - usado,
      "%s{\"plays\":1,\"movie\":{\"ids\":{\"imdb\":\"tt%07d\"}}}",
      i ? "," : "", inicio + i);
  snprintf(s + usado, cap - usado, "]");
  return s;
}
static char *resposta(const char *url, int *status) {
  int p = 1, lim = 0, inicio, qtd;
  if (strstr(url, "/sync/last_activities")) {
    char s[160];
    nAtividades++;
    *status = statusAtividade;
    if (statusAtividade != 200) return strdup("{\"error\":\"fixture\"}");
    snprintf(s, sizeof s, "{\"movies\":{\"watched_at\":\"%s\"}}", atividade);
    return strdup(s);
  }
  assert(strstr(url, "/sync/watched/movies"));
  nPedidos++;
  if (nPedidos == trocarNaPagina) { geracaoMapa++; nMapa = 0; }
  if (strchr(url, '?')) {
    assert(sscanf(strchr(url, '?'), "?page=%d&limit=%d", &p, &lim) == 2);
    assert(p > 0 && lim > 0 && lim <= 250);
  }
  *status = p == erroPagina ? 503 : 200;
  if (*status != 200) return strdup("[{\"movie\":{\"ids\":{\"imdb\":\"tt9999999\"}}}]");
  if (primeiraSemId && p == 1) return strdup("[{\"movie\":{\"ids\":{\"imdb\":null}}}]");
  if (repetirPagina) return pagina(0, porPagina);
  if (corpoPagina && p == 2) return strdup(corpoPagina);
  inicio = (p - 1) * porPagina;
  qtd = inicio < totalServidor ? totalServidor - inicio : 0;
  if (qtd > porPagina) qtd = porPagina;
  return pagina(inicio, qtd);
}
char *rede_baixar_com(const char *url, int segundos, const char *const *cab) {
  int status;
  char *s;
  (void)segundos; (void)cab;
  s = resposta(url, &status);
  if (status < 200 || status >= 300) { free(s); return NULL; }
  return s;
}
char *rede_baixar_st(const char *url, int segundos, const char *const *cab, int *status) {
  (void)segundos; (void)cab;
  return resposta(url, status);
}

static void iniciar(void) {
  nMapa = nPedidos = nAtividades = erroPagina = 0;
  corpoPagina = NULL; statusAtividade = 200; totalServidor = 137; porPagina = 100;
  repetirPagina = primeiraSemId = trocarNaPagina = 0;
  geracaoMapa++;
  atividade = "2026-10-02T10:00:00.000Z";
  filmesAtiv[0] = 0;
}
int main(void) {
  char *s;
  const char *cab[] = {NULL};
  iniciar();
  s = pagina(0, 301);
  verifica(trakt_ler_filmes_vistos(s) == 301 && nMapa == 301,
           "parser nao limita a 100 filmes");
  free(s);

  iniciar();
  carregarFilmesVistos(cab, historicoPedido());
  verifica(nMapa == 137 && nPedidos == 3 && nAtividades == 1,
           "pagina 137 filmes incluindo resposta curta 37 e ultima vazia");
  verifica(!strcmp(filmesAtiv, atividade), "atividade so aplicada com mapa completo");
  nPedidos = 0;
  carregarFilmesVistos(cab, historicoPedido());
  verifica(nPedidos == 0, "mesma atividade evita downloads redundantes");

  geracaoMapa++; nMapa = 0; nPedidos = 0;
  carregarFilmesVistos(cab, historicoPedido());
  verifica(nMapa == 137 && nPedidos == 3,
           "mesma atividade no outro perfil nao herda marcador do mapa antigo");

  atividade = "2026-10-02T11:00:00.000Z";
  erroPagina = 2; totalServidor = 205; nPedidos = 0;
  cat_historico_definir_id("tt0000000", "movie", 0);
  carregarFilmesVistos(cab, historicoPedido());
  verifica(nMapa == 137 && !mapa[0].visto && nPedidos == 2,
           "HTTP 503 pagina 2 preserva mapa inteiro inclusive estado local 0");
  verifica(strcmp(filmesAtiv, atividade), "pagina falha nao cacheia atividade nova");
  erroPagina = 0; nPedidos = 0;
  carregarFilmesVistos(cab, historicoPedido());
  verifica(nMapa == 205 && mapa[0].visto && nPedidos == 4,
           "retry da mesma atividade recupera todas as paginas");

  atividade = "2026-10-02T12:00:00.000Z";
  corpoPagina = "[{\"movie\":{\"ids\":{\"imdb\":\"tt7777777\"}}}, {";
  nPedidos = 0; cat_historico_definir_id("tt0000000", "movie", 0);
  carregarFilmesVistos(cab, historicoPedido());
  verifica(nMapa == 205 && !mapa[0].visto && strcmp(filmesAtiv, atividade),
           "pagina truncada nao publica parcialmente nem cacheia atividade");
  corpoPagina = "{\"error\":\"fixture\"}";
  carregarFilmesVistos(cab, historicoPedido());
  verifica(nMapa == 205 && !mapa[0].visto && strcmp(filmesAtiv, atividade),
           "corpo objeto em HTTP 200 nao significa mapa vazio");

  iniciar(); totalServidor = 0;
  carregarFilmesVistos(cab, historicoPedido());
  verifica(nMapa == 0 && nPedidos == 1 && !strcmp(filmesAtiv, atividade),
           "array vazio e resposta completa valida");

  iniciar(); statusAtividade = 503;
  carregarFilmesVistos(cab, historicoPedido()); carregarFilmesVistos(cab, historicoPedido());
  verifica(nMapa == 137 && nPedidos == 6 && !filmesAtiv[0],
           "atividade indisponivel baixa mapa sem cachear falha");

  iniciar(); primeiraSemId = 1;
  carregarFilmesVistos(cab, historicoPedido());
  verifica(nMapa == 37 && nPedidos == 3 && !strcmp(filmesAtiv, atividade),
           "pagina sem IMDb nao encerra antes das seguintes");

  iniciar(); repetirPagina = 1;
  carregarFilmesVistos(cab, historicoPedido());
  verifica(nMapa == 0 && nPedidos == 2 && !filmesAtiv[0],
           "servidor ignora pagina nao provoca loop nem mapa parcial");

  iniciar(); trocarNaPagina = 2;
  carregarFilmesVistos(cab, historicoPedido());
  verifica(nMapa == 0 && nPedidos == 2 && !filmesAtiv[0],
           "troca de perfil durante pagina 2 descarta snapshot antigo");
  trocarNaPagina = 0; nPedidos = 0;
  carregarFilmesVistos(cab, historicoPedido());
  verifica(nMapa == 137 && nPedidos == 3 && !strcmp(filmesAtiv, atividade),
           "perfil novo baixa mapa sem herdar atividade anterior");

  iniciar(); erroPagina = 1;
  carregarFilmesVistos(cab, historicoPedido());
  verifica(nMapa == 0 && nPedidos == 1 && !filmesAtiv[0],
           "erro primeira pagina preserva estado para retry");

  iniciar();
  verifica(trakt_ler_filmes_vistos("[{\"movie\":{\"ids\":{\"imdb\":\"tt1234567\"}}}, {") == -1 && nMapa == 0,
           "parser rejeita truncamento preservando mapa");
  verifica(trakt_ler_filmes_vistos("<html>fixture</html>") == -1 && nMapa == 0,
           "parser rejeita documento fora do contrato");
  verifica(trakt_ler_filmes_vistos("[] lixo") == -1 && nMapa == 0,
           "parser rejeita conteudo depois do array");
  verifica(trakt_ler_filmes_vistos(NULL) == -1, "corpo ausente falha");
  if (!falhas) puts("trakt filmes vistos: PASS (paginacao, integridade, falhas e atividades)");
  return falhas ? 1 : 0;
}
