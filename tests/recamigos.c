// Entre amigos alem do Trakt, lado cliente, com a rede falsa. Ver
// tests/recamigos.sh. Inclui recomenda.c para alcançar as funcoes estaticas de
// rede e de parse.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef REC_TESTE_SEM_URL
#define NV_REC_URL ""
#else
#define NV_REC_URL "http://127.0.0.1:8799"
#endif

#define rede_baixar_etag  teste_rede_etag
#define rede_postar_st    teste_rede_postar
#define rede_baixar_st    teste_rede_st
#define rede_baixar_com   teste_rede_com

#include "../src/recomenda.c"

// --- rede falsa, ROTEADA por URL ------------------------------------------------
// Cada requisicao e registrada (url + corpo) para o teste conferir O QUE SAIU.
#define LOG_N 40
static struct { char url[200]; char corpo[900]; } sai[LOG_N];
static int nSai;
static struct { char sufixo[64]; int status; const char *corpo; } rotas[24];
static int nRotas;

static void rota(const char *sufixo, int status, const char *corpo) {
  int i;
  for (i = 0; i < nRotas; i++)
    if (!strcmp(rotas[i].sufixo, sufixo)) { rotas[i].status = status; rotas[i].corpo = corpo; return; }
  snprintf(rotas[nRotas].sufixo, sizeof rotas[0].sufixo, "%s", sufixo);
  rotas[nRotas].status = status; rotas[nRotas].corpo = corpo; nRotas++;
}
static char *dup(const char *s) {
  char *r; if (!s) return NULL;
  r = (char *)malloc(strlen(s) + 1); if (r) strcpy(r, s); return r;
}
static char *responde(const char *url, const char *corpo, int *status) {
  int i; size_t lu = strlen(url);
  if (nSai < LOG_N) {
    snprintf(sai[nSai].url, sizeof sai[0].url, "%s", url);
    snprintf(sai[nSai].corpo, sizeof sai[0].corpo, "%s", corpo ? corpo : "");
    nSai++;
  }
  for (i = 0; i < nRotas; i++) {
    size_t ls = strlen(rotas[i].sufixo);
    if (lu >= ls && !strcmp(url + lu - ls, rotas[i].sufixo)) {
      if (status) *status = rotas[i].status;
      return dup(rotas[i].corpo);
    }
  }
  if (status) *status = 200;
  return dup("{\"ok\":1}");
}
char *teste_rede_etag(const char *url, int seg, const char *const *cab, int *st, char *e, unsigned n) {
  (void)seg; (void)cab; (void)e; (void)n; return responde(url, NULL, st);
}
char *teste_rede_postar(const char *url, int seg, const char *const *cab, const char *corpo, int *st) {
  (void)seg; (void)cab; return responde(url, corpo, st);
}
char *teste_rede_st(const char *url, int seg, const char *const *cab, int *st) {
  (void)seg; (void)cab; return responde(url, NULL, st);
}
char *teste_rede_com(const char *url, int seg, const char *const *cab) {
  (void)seg; (void)cab; return responde(url, NULL, NULL);
}

static int falhas;
#define CONFERE(c, ...) do { if (!(c)) { falhas++; printf("FALHA: " __VA_ARGS__); printf("\n"); } } while (0)
// Quantas requisicoes cujo url termina em `suf` sairam.
static int saiu(const char *suf) {
  int i, n = 0; size_t ls = strlen(suf);
  for (i = 0; i < nSai; i++) { size_t lu = strlen(sai[i].url);
    if (lu >= ls && !strcmp(sai[i].url + lu - ls, suf)) n++; }
  return n;
}
static const char *corpoDe(const char *suf) {   // do ULTIMO pedido para `suf`
  int i; size_t ls = strlen(suf);
  for (i = nSai - 1; i >= 0; i--) { size_t lu = strlen(sai[i].url);
    if (lu >= ls && !strcmp(sai[i].url + lu - ls, suf)) return sai[i].corpo; }
  return "";
}
static void zerarSaida(void) { nSai = 0; }

#ifndef REC_TESTE_SEM_URL
static const char *CAB[3] = { "Authorization: Bearer tok-teste", "X-Nuvio-Auth: nuvio", NULL };

static char *lerArq(const char *nome) {
  char cam[600]; FILE *f; long n; char *s;
  snprintf(cam, sizeof cam, "%s/%s", dados_dir(), nome);
  f = fopen(cam, "rb"); if (!f) return NULL;
  fseek(f, 0, SEEK_END); n = ftell(f); fseek(f, 0, SEEK_SET);
  s = (char *)malloc((size_t)n + 1);
  if (fread(s, 1, (size_t)n, f) != (size_t)n) { free(s); fclose(f); return NULL; }
  s[n] = 0; fclose(f); return s;
}
#endif

int main(void) {
  const char *dir = getenv("NUVIO_DADOS");
  if (!dir || !dir[0]) { printf("recamigos: NUVIO_DADOS ausente; recusando rodar\n"); return 2; }
  dados_iniciar(dir);
  if (strcmp(dados_dir(), dir)) { printf("recamigos: dados_dir errado; recusando rodar\n"); return 2; }

#ifdef REC_TESTE_SEM_URL
  (void)rota; (void)saiu; (void)corpoDe; (void)zerarSaida;
  { RecPerfil p; RecPessoa x; memset(&p, 0, sizeof p);
    snprintf(p.apelido, sizeof p.apelido, "fabi");
    recomenda_iniciar();
    CONFERE(!recomenda_perfil_publicar(&p), "sem URL, publicar recusa");
    CONFERE(!recomenda_buscar("fabi"), "sem URL, buscar recusa");
    CONFERE(!recomenda_pedir_amizade("abcdefghij"), "sem URL, pedir recusa");
    CONFERE(!recomenda_pesquisavel(), "sem URL, nunca pesquisavel");
    CONFERE(recomenda_n_achados() == 0 && !recomenda_achado(0, &x), "sem URL, sem achados");
    recomenda_atividade_nivel(2);
    recomenda_atividade_fim(NULL, 1);
    { CatItem it[4]; memset(it, 0, sizeof it);
      CONFERE(recomenda_social_mesclar(it, 3, 4) == 3, "sem URL, a fileira fica so com o Trakt"); }
    CONFERE(nSai == 0, "sem URL nao pode sair requisicao (%d)", nSai); }
  printf(falhas ? "recamigos (sem URL): %d falhas\n" : "recamigos (sem URL): ok\n", falhas);
  return falhas ? 1 : 0;
#else
  if (!mtx) mtx = SDL_CreateMutex();
  recomenda_iniciar();

  // --- 1. TUDO NASCE DESLIGADO E NADA SAI -----------------------------------------
  { RecPerfil p; recomenda_perfil(&p);
    CONFERE(!p.publicado && !p.apelido[0] && p.ativ == 0 && !p.recentes && !p.foto,
            "perfil nasce vazio e desligado");
    CONFERE(!recomenda_pesquisavel(), "nasce nao pesquisavel"); }
  enviarPerfil(CAB); enviarAtividade(CAB); tratarSocial(CAB);
  CONFERE(nSai == 0, "sem ninguem ter ligado nada, NAO sai requisicao social (%d)", nSai);
  { CatItem ci; memset(&ci, 0, sizeof ci);
    snprintf(ci.imdb, sizeof ci.imdb, "tt0111161"); snprintf(ci.titulo, sizeof ci.titulo, "Um Sonho");
    recomenda_atividade_passo(&ci, 1);
    recomenda_atividade_fim(&ci, 1);
    CONFERE(nAtivFila == 0, "com tudo desligado o player nao enfileira nada (%d)", nAtivFila);
    enviarAtividade(CAB);
    CONFERE(saiu("/v1/atividade") == 0, "e nada sai"); }

  // --- 2. PUBLICAR: so o que ela escolheu, e limpo -----------------------------------
  { RecPerfil p; memset(&p, 0, sizeof p);
    CONFERE(!recomenda_perfil_publicar(&p), "sem apelido nao publica");
    snprintf(p.apelido, sizeof p.apelido, "Fabi CINE!! @x.com");
    snprintf(p.bio, sizeof p.bio, "Fã de terror. Escreva a@b.com ou http://x.io");
    p.generos = (1u << 13) | (1u << 6) | (1u << 20);   // terror, drama e um bit fora da faixa
    p.foto = 1; p.recentes = 0; p.ativ = -1;           // -1 = nao mexer no nivel
    CONFERE(recomenda_perfil_publicar(&p), "publica com apelido");
    zerarSaida();
    enviarPerfil(CAB);
    CONFERE(saiu("/v1/perfil") == 1, "POST /v1/perfil saiu");
    { const char *c = corpoDe("/v1/perfil");
      CONFERE(strstr(c, "\"apelido\":\"fabi cine x com\"") != NULL, "apelido limpo: %s", c);
      // O corpo JSON nao tem ponto, barra nem arroba: se algum aparecer, veio
      // do texto da pessoa (e-mail, link) e a limpeza falhou.
      CONFERE(strchr(c, '@') == NULL && strchr(c, '/') == NULL && strchr(c, '.') == NULL,
              "sem arroba, barra nem ponto no corpo: %s", c);
      CONFERE(strstr(c, "\"generos\":[\"drama\",\"terror\"]") != NULL, "generos por id: %s", c);
      CONFERE(strstr(c, "\"avatar\":1") != NULL && strstr(c, "\"recentes\":0") != NULL,
              "foto ligada, recentes desligado: %s", c);
      // SO ESTAS CINCO CHAVES. Se alguem acrescentar id, e-mail ou aparelho ao
      // corpo, este teste cai.
      CONFERE(strstr(c, "id") == NULL || strstr(c, "\"id\"") == NULL, "corpo nao leva id: %s", c);
      CONFERE(strstr(c, "email") == NULL && strstr(c, "addon") == NULL && strstr(c, "nuvio:") == NULL,
              "corpo nao leva e-mail/addon/id de conta: %s", c); } }
  CONFERE(recomenda_pesquisavel(), "agora pesquisavel");
  CONFERE(perfilPendente == 0, "pendente limpo com o servidor confirmando");
  CONFERE(recomenda_aparecer() == REC_APARECER_SIM, "publicar tambem liga o 'aparecer'");
  { char *arq = lerArq(REC_ARQ_PERFIL);
    CONFERE(arq && strstr(arq, "fabi cine x com") != NULL, "perfil foi para o disco");
    free(arq); }

  // FALHA DE REDE NAO PERDE A ESCOLHA: o pendente fica para o proximo ciclo.
  { RecPerfil p; recomenda_perfil(&p); snprintf(p.bio, sizeof p.bio, "so terror");
    p.ativ = -1;
    rota("/v1/perfil", 500, "{}");
    recomenda_perfil_publicar(&p);
    enviarPerfil(CAB);
    CONFERE(perfilPendente == 1, "500 mantem o pendente");
    rota("/v1/perfil", 200, "{\"ok\":1}");
    enviarPerfil(CAB);
    CONFERE(perfilPendente == 0, "e o proximo ciclo entrega"); }

  // --- 3. DESPUBLICAR APAGA LOCAL E MANDA APAGAR ---------------------------------------
  zerarSaida();
  recomenda_perfil_despublicar();
  { RecPerfil p; recomenda_perfil(&p);
    CONFERE(!p.publicado && !p.apelido[0] && !p.bio[0] && p.generos == 0 && !p.foto,
            "despublicar zera tudo que era publico"); }
  CONFERE(recomenda_aparecer() == REC_APARECER_NAO, "e o aparecer vira 'nao'");
  enviarPerfil(CAB);
  CONFERE(saiu("/v1/perfil/despublicar") == 1, "POST despublicar saiu");
  { char *arq = lerArq(REC_ARQ_PERFIL);
    CONFERE(arq && strstr(arq, "fabi") == NULL, "o apelido saiu do disco");
    free(arq); }

  // --- 4. ATIVIDADE: nivel a nivel ------------------------------------------------------
  { CatItem ci; memset(&ci, 0, sizeof ci);
    snprintf(ci.imdb, sizeof ci.imdb, "tt0111161"); snprintf(ci.titulo, sizeof ci.titulo, "Um Sonho de Liberdade");
    snprintf(ci.tipo, sizeof ci.tipo, "movie"); snprintf(ci.meta, sizeof ci.meta, "1994 · 2h22");
    ci.nota = 93;
    recomenda_atividade_nivel(1);
    zerarSaida(); enviarPerfil(CAB);
    CONFERE(strstr(corpoDe("/v1/perfil/atividade"), "\"nivel\":1") != NULL, "nivel 1 enviado");
    recomenda_atividade_passo(&ci, 1);
    CONFERE(nAtivFila == 0, "nivel 1 nao diz 'assistindo agora'");
    recomenda_atividade_fim(&ci, 0);
    CONFERE(nAtivFila == 0, "largar no meio nao e 'assistiu'");
    recomenda_atividade_fim(&ci, 1);
    CONFERE(nAtivFila == 1, "concluir e 'assistiu'");
    enviarAtividade(CAB);
    { const char *c = corpoDe("/v1/atividade");
      CONFERE(strstr(c, "\"imdb\":\"tt0111161\"") && strstr(c, "\"agora\":0") &&
              strstr(c, "\"ano\":\"1994\""), "corpo da atividade: %s", c);
      CONFERE(!strstr(c, "poster") && !strstr(c, "url") && !strstr(c, "fonte") && !strstr(c, "pos"),
              "SEM poster, fonte nem posicao: %s", c); }
    recomenda_atividade_nivel(2);
    recomenda_atividade_passo(&ci, 1);
    CONFERE(nAtivFila == 1, "nivel 2 diz 'agora'");
    recomenda_atividade_passo(&ci, 1);
    CONFERE(nAtivFila == 1, "e nao repete a cada quadro");
    recomenda_atividade_passo(&ci, 0);
    CONFERE(nAtivFila == 1, "pausado nao conta");
    // DESLIGAR DERRUBA O QUE AINDA NAO SAIU.
    recomenda_atividade_nivel(0);
    CONFERE(nAtivFila == 0, "desligar esvazia a fila");
    zerarSaida(); enviarAtividade(CAB);
    CONFERE(saiu("/v1/atividade") == 0, "e nada sai depois de desligar"); }

  // --- 5. BUSCA -----------------------------------------------------------------------------
  zerarSaida();
  CONFERE(!recomenda_buscar("fa"), "2 letras nao saem do aparelho");
  CONFERE(recomenda_soc_estado() == REC_SOC_CURTA, "e o estado diz 'curta'");
  recomenda_soc_limpar();
  CONFERE(!recomenda_buscar("  "), "vazio nao busca");
  recomenda_soc_limpar();
  rota("/v1/perfis/buscar", 200,
       "{\"resultados\":[{\"pub\":\"k9ptiwtmrb\",\"apelido\":\"fabi cine\",\"avatar\":\"https://walter.trakt.tv/a.jpg\","
       "\"bio\":\"fa de terror\",\"generos\":[\"terror\",\"drama\",\"inexistente\"],\"relacao\":\"\"},"
       "{\"pub\":\"m3n4p5q6r7\",\"apelido\":\"gui nerd\",\"avatar\":\"\",\"bio\":\"\",\"generos\":[],\"relacao\":\"recebido\"}]}");
  CONFERE(recomenda_buscar(" FaBi "), "busca de verdade entra na fila");
  CONFERE(!recomenda_buscar("outra"), "so uma operacao por vez");
  tratarSocial(CAB);
  CONFERE(strstr(corpoDe("/v1/perfis/buscar"), "{\"q\":\"fabi\"}") != NULL,
          "corpo normalizado: %s", corpoDe("/v1/perfis/buscar"));
  CONFERE(recomenda_soc_estado() == REC_SOC_OK, "busca ok");
  CONFERE(recomenda_n_achados() == 2 && recomenda_achados_origem() == 1, "2 achados de busca");
  { RecPessoa x; recomenda_achado(0, &x);
    CONFERE(!strcmp(x.pub, "k9ptiwtmrb") && !strcmp(x.apelido, "fabi cine"), "pessoa 0: %s/%s", x.pub, x.apelido);
    CONFERE(x.generos == ((1u << 13) | (1u << 6)), "generos em mascara, o desconhecido cai: %u", x.generos);
    recomenda_achado(1, &x);
    CONFERE(!strcmp(x.relacao, "recebido"), "relacao lida"); }
  recomenda_soc_limpar();
  rota("/v1/perfis/buscar", 429, "{\"erro\":\"limite\"}");
  recomenda_buscar("fabi"); tratarSocial(CAB);
  CONFERE(recomenda_soc_estado() == REC_SOC_LIMITE, "429 vira 'limite'");
  recomenda_soc_limpar();

  // --- 6. CARTAO, PEDIDO, ACEITE, BLOQUEIO ---------------------------------------------------
  CONFERE(!recomenda_ver_perfil("nuvio:aaa"), "so handle de 10 entra (nunca id de conta)");
  CONFERE(!recomenda_pedir_amizade("../../etc"), "handle malformado nao sai");
  rota("/v1/perfis/ver", 200,
       "{\"pub\":\"k9ptiwtmrb\",\"apelido\":\"fabi cine\",\"avatar\":\"\",\"bio\":\"fa\",\"generos\":[\"terror\"],"
       "\"relacao\":\"\",\"recentes\":[{\"imdb\":\"tt1\",\"tipo\":\"movie\",\"titulo\":\"Um Sonho\",\"ano\":\"1994\"},"
       "{\"imdb\":\"tt2\",\"tipo\":\"movie\",\"titulo\":\"Origem\",\"ano\":\"2010\"}]}");
  recomenda_ver_perfil("k9ptiwtmrb"); tratarSocial(CAB);
  { RecPessoa c; char t[80];
    CONFERE(recomenda_cartao(&c) && !strcmp(c.apelido, "fabi cine"), "cartao aberto");
    CONFERE(recomenda_cartao_n_recentes() == 2 && recomenda_cartao_recente(1, t, sizeof t) && !strcmp(t, "Origem"),
            "vistos recentes do cartao"); }
  recomenda_soc_limpar();
  rota("/v1/pedidos/enviar", 409, "{\"erro\":\"defina um apelido antes de pedir amizade\"}");
  recomenda_pedir_amizade("k9ptiwtmrb"); tratarSocial(CAB);
  CONFERE(recomenda_soc_estado() == REC_SOC_SEM_APELIDO, "409 vira 'sem apelido'");
  recomenda_soc_limpar();
  rota("/v1/pedidos/enviar", 200, "{\"ok\":1,\"estado\":\"enviado\"}");
  recomenda_pedir_amizade("k9ptiwtmrb"); tratarSocial(CAB);
  CONFERE(recomenda_soc_estado() == REC_SOC_OK, "pedido enviado");
  CONFERE(strstr(corpoDe("/v1/pedidos/enviar"), "{\"pub\":\"k9ptiwtmrb\"}") != NULL, "corpo do pedido so leva o handle");
  recomenda_soc_limpar();
  rota("/v1/pedidos", 200,
       "{\"recebidos\":[{\"pub\":\"aaaaaaaaaa\",\"apelido\":\"ana\",\"avatar\":\"\",\"bio\":\"\",\"generos\":[],\"relacao\":\"recebido\"},"
       "{\"pub\":\"bbbbbbbbbb\",\"apelido\":\"bia\",\"avatar\":\"\",\"bio\":\"\",\"generos\":[],\"relacao\":\"recebido\"}],\"enviados\":1}");
  recomenda_listar_pedidos(); tratarSocial(CAB);
  CONFERE(recomenda_n_pedidos() == 2, "2 pedidos recebidos");
  recomenda_soc_limpar();
  recomenda_aceitar("aaaaaaaaaa"); tratarSocial(CAB);
  CONFERE(recomenda_n_pedidos() == 1, "aceitar tira da caixa na hora");
  recomenda_soc_limpar();
  recomenda_recusar("bbbbbbbbbb"); tratarSocial(CAB);
  CONFERE(recomenda_n_pedidos() == 0, "recusar tambem");
  recomenda_soc_limpar();
  zerarSaida();
  CONFERE(recomenda_bloquear("k9ptiwtmrb"), "bloquear por handle");
  tratarSocial(CAB);
  CONFERE(strstr(corpoDe("/v1/bloquear"), "{\"pub\":\"k9ptiwtmrb\"}") != NULL, "bloqueio por handle: %s", corpoDe("/v1/bloquear"));
  recomenda_soc_limpar();
  CONFERE(recomenda_bloquear("nuvio:hhh"), "bloquear por id (lista de amigos)");
  tratarSocial(CAB);
  CONFERE(strstr(corpoDe("/v1/bloquear"), "{\"id\":\"nuvio:hhh\"}") != NULL, "bloqueio por id: %s", corpoDe("/v1/bloquear"));
  recomenda_soc_limpar();
  CONFERE(!recomenda_bloquear("lixo sem dois pontos"), "lixo nao vira bloqueio");
  rota("/v1/bloqueados", 200, "{\"bloqueados\":[{\"pub\":\"cccccccccc\",\"nome\":\"chato\"}]}");
  recomenda_listar_bloqueados(); tratarSocial(CAB);
  { char pub[16], nome[64];
    CONFERE(recomenda_n_bloqueados() == 1 && recomenda_bloqueado(0, pub, sizeof pub, nome, sizeof nome) &&
            !strcmp(pub, "cccccccccc") && !strcmp(nome, "chato"), "lista de bloqueados"); }
  recomenda_soc_limpar();

  // --- 7. A FILEIRA: o feed e a uniao --------------------------------------------------------
  { static const char *FEED =
      "{\"itens\":[{\"de\":\"nuvio:hhh\",\"deNome\":\"helena tv\",\"deAvatar\":\"https://walter.trakt.tv/h.jpg\","
      "\"imdb\":\"tt0111161\",\"tipo\":\"movie\",\"titulo\":\"Um Sonho de Liberdade\",\"ano\":\"1994\",\"nota\":93,\"agora\":1,\"criado\":1790000000},"
      "{\"de\":\"nuvio:iii\",\"deNome\":\"ivo\",\"deAvatar\":\"\",\"imdb\":\"tt0903747\",\"tipo\":\"series\",\"titulo\":\"Breaking Bad\","
      "\"ano\":\"2008\",\"nota\":95,\"agora\":0,\"criado\":1789999000},"
      "{\"de\":\"nuvio:jjj\",\"deNome\":\"jo\",\"imdb\":\"tt0468569\",\"tipo\":\"movie\",\"titulo\":\"Batman\",\"nota\":90,\"agora\":0,\"criado\":1789998000},"
      "{\"de\":\"nuvio:kkk\",\"deNome\":\"ka\",\"imdb\":\"semtt\",\"titulo\":\"Lixo\"}]}";
    CatItem nu[8], tk[8], un[8];
    int n, i;
    memset(nu, 0, sizeof nu); memset(tk, 0, sizeof tk); memset(un, 0, sizeof un);
    n = lerFeedCorpo(FEED, nu, 8);
    CONFERE(n == 3, "3 itens validos (o sem tt cai): %d", n);
    CONFERE(!strcmp(nu[0].socialNome, "helena tv") && !strcmp(nu[0].socialSlug, "nuvio:hhh") &&
            !strcmp(nu[0].socialAcao, "assistindo agora") && !strcmp(nu[0].imdb, "tt0111161"),
            "item 0: %s/%s/%s", nu[0].socialNome, nu[0].socialSlug, nu[0].socialAcao);
    CONFERE(!strcmp(nu[1].tipo, "series") && !strcmp(nu[1].direcao, "Série") &&
            !strcmp(nu[1].socialAcao, "assistiu"), "serie: tipo/direcao/acao");
    CONFERE(nu[0].nota == 93 && nu[0].retomadoMs == 1790000000000LL, "nota e instante");
    { RecAtivAmigo tit[3];
      CONFERE(recomenda_amigo_atividades("nuvio:hhh", tit, 3) == 1 && !strcmp(tit[0].titulo, "Um Sonho de Liberdade") &&
              tit[0].agora == 1 && !strcmp(tit[0].imdb, "tt0111161"),
              "o feed fica em memoria para 'Ver perfil' do amigo"); }
    // Trakt: 3 itens, um deles o MESMO titulo que o amigo Nuvio esta vendo.
    snprintf(tk[0].imdb, sizeof tk[0].imdb, "tt0111161"); snprintf(tk[0].socialNome, sizeof tk[0].socialNome, "TraktUser");
    snprintf(tk[0].socialAcao, sizeof tk[0].socialAcao, "assistiu");
    snprintf(tk[1].imdb, sizeof tk[1].imdb, "tt0133093"); snprintf(tk[1].socialNome, sizeof tk[1].socialNome, "T2");
    snprintf(tk[2].imdb, sizeof tk[2].imdb, "tt1375666"); snprintf(tk[2].socialNome, sizeof tk[2].socialNome, "T3");
    memcpy(un, tk, sizeof tk);
    n = rec_social_unir(un, 3, nu, 3, 8);
    CONFERE(n == 5, "uniao sem duplicata: %d", n);
    CONFERE(!strcmp(un[0].socialNome, "helena tv"), "'assistindo agora' vem primeiro: %s", un[0].socialNome);
    for (i = 0; i < n; i++) { int j;
      for (j = i + 1; j < n; j++) CONFERE(strcmp(un[i].imdb, un[j].imdb), "titulo repetido na fileira: %s", un[i].imdb); }
    { int achouT = 0, achouN = 0;
      for (i = 0; i < n; i++) { if (!strcmp(un[i].socialNome, "TraktUser")) achouT = 1;
                                 if (!strcmp(un[i].socialNome, "ivo")) achouN = 1; }
      CONFERE(!achouT, "o titulo repetido fica com o amigo que esta ASSISTINDO AGORA");
      CONFERE(achouN, "as duas fontes aparecem"); }
    memcpy(un, tk, sizeof tk);
    n = rec_social_unir(un, 3, nu, 3, 4);
    CONFERE(n == 4, "respeita o teto: %d", n);
    memcpy(un, tk, sizeof tk);
    CONFERE(rec_social_unir(un, 3, nu, 0, 8) == 3, "sem amigos Nuvio a fileira e a do Trakt"); }

  // --- 8. SAIR DA CONTA APAGA TUDO --------------------------------------------------------------
  { RecPerfil p; memset(&p, 0, sizeof p); snprintf(p.apelido, sizeof p.apelido, "ana");
    p.ativ = 2; recomenda_perfil_publicar(&p); recomenda_atividade_nivel(2); }
  recomenda_esquecer();
  { RecPerfil p; recomenda_perfil(&p);
    CONFERE(!p.publicado && !p.apelido[0] && p.ativ == 0, "sair zera o perfil");
    CONFERE(recomenda_n_achados() == 0 && recomenda_n_pedidos() == 0 && recomenda_n_bloqueados() == 0,
            "e a busca, os pedidos e os bloqueados");
    CONFERE(recomenda_cartao_n_recentes() == 0, "e os recentes do cartao");
    CONFERE(recomenda_soc_estado() == REC_SOC_NADA && nAtivFila == 0 && nFeed == 0, "e a fila e o feed");
    { char *arq = lerArq(REC_ARQ_PERFIL); CONFERE(arq == NULL, "e o arquivo do perfil some"); free(arq); } }

  printf(falhas ? "recamigos: %d falha(s)\n" : "recamigos: ok\n", falhas);
  return falhas ? 1 : 0;
#endif
}
