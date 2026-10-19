// Exercises the real paged reader with a fake network, including stale responses.
#include <assert.h>
#include <unistd.h>
int ajustes_social(void) { return 1; }
int ajustes_hist_conta(void) { return 1; }
int ajustes_busca_cinemeta(void) { return 1; }
int ajustes_busca_nuvio(void) { return 0; }   // #311: Primeiro (padrao)
int ajustes_ocultar_nao_lancados(void) { return 0; }   // #369: descoberta.c le o ajuste
int ajustes_ocultar_filmes_vistos(void) { return 0; }    // #366: descoberta.c le o ajuste
unsigned cat_revisao_itens(void) { return 0; }               // #366: a projecao le a revisao
#define NV_DUBLE_SESSAO
#include "../src/descoberta.c"
// descoberta.c passou a traduzir os rotulos que monta ("Filme", "Serie", a
// data por extenso) e este teste nao linka idioma.c: linkar puxaria
// ajustes_idioma_ingles e, atras dele, ajustes.c e o resto do app — o oposto
// do que um teste do leitor paginado deve carregar. Devolver a entrada e o que
// i18n faz com o idioma em portugues, que e o padrao, entao os rotulos que as
// asserçoes comparam sao exatamente os de producao. Mesmo stub de
// tests/colfileiras.c.
// 0 = portugues, o padrao — e o idioma em que as asserçoes deste arquivo
// escreveram os rotulos esperados.
int ajustes_idioma_ingles(void) { return 0; }
int ajustes_idioma(void) { return 0; }
// Integracao TMDB ligada por padrao — ver a nota igual em tests/colfileiras.c.
int ajustes_tmdb_ligado(void) { return 1; }
const char *ajustes_tmdb_idioma(void) { return "pt-BR"; }
const char *i18n(const char *s) { return s; }
const char *idioma_mes_data(int mes, const char *nomePt) { (void)mes; return nomePt; }
// Ramo de fonte nao-addon (issue #44) e refazer da fileira CW (#38): nao sao
// o que este teste mede, mas fioVerTudo/fioContinuar referenciam os simbolos.
const char *nuvem_trakt_cliente(void) { return ""; }
char *rede_baixar_com(const char *u, int t, const char *const *c) {
  (void)c; return rede_baixar(u, t); }
int trakt_enfeitar_lote(CatItem *s, int n) { (void)s; (void)n; return 0; }
int trakt_enfeitar_lote_sessao(CatItem *s, int n, uint64_t session) {
  (void)s; (void)n; (void)session; return 0;
}
Uint32 SDL_GetTicks(void) { return 0; }
void marco(const char *name) { (void)name; }
// Origem do item (CatItem.origem): sem addons neste teste, a origem fica vazia.
int addons_n(void) { return 0; }
int addons_base_desligada(const char *b) { (void)b; return 0; }   // 203-desligados
const char *addons_base(int i) { (void)i; return ""; }
const char *addons_id_manifesto(int i) { (void)i; return ""; }
void cat_trocar_continuar(const CatItem *l, int q) { (void)l; (void)q; }
const char *addons_base_por_id(const char *id) { (void)id; return ""; }
int cat_visto(const CatItem *item) { (void)item; return 0; }
int cat_copiar_por_id(const char *id, const char *type, CatItem *item) {
  (void)id; (void)type; (void)item; return 0;
}
static int calls;
static int fixtureStatus, fixtureReject;
static char fixtureUrl[NV_ADDON_PEDIDO_MAX];
static pthread_mutex_t fakeLock=PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t fakeCond=PTHREAD_COND_INITIALIZER;
static int slowStarted,releaseSlow;
char *rede_baixar(const char *url,int timeout) {
  (void)timeout;calls++;
  if (strstr(url, "https://collection.invalid/")) {
    snprintf(fixtureUrl, sizeof fixtureUrl, "%s", url);
    fixtureStatus = 404;
    const char *path = url + strlen("https://collection.invalid");
    const char *paths[] = {
      "/addon/catalog/movie/new%2Fmovies%20%231%3Ffresh%25/genre=New%20%26%20fresh%2FTV&skip=0.json?config=fixture",
      "/addon/catalog/movie/new%2Fmovies%20%231%3Ffresh%25/genre=New%20%26%20fresh%2FTV&skip=1.json?config=fixture",
      "/addon/catalog/series/new%2Fseries%20%231%3Ffresh%25.json?config=fixture",
      "/addon/catalog/series/new%2Fseries%20%231%3Ffresh%25/skip=1.json?config=fixture",
      "/addon/catalog/tv/new%2Fseries%20%231%3Ffresh%25.json?config=fixture",
      "/other/catalog/movie/new%2Fmovies%20%231%3Ffresh%25.json?config=fixture",
      "/addon/catalog/series/new%2Fseries%20%231%3Ffresh%25/search=new%20%26%20fresh.json?config=fixture"
    };
    for (unsigned i = 0; i < sizeof paths / sizeof *paths; i++) {
      if (strcmp(path, paths[i])) continue;
      fixtureStatus = fixtureReject ? 403 : 200;
      if (i == 1 || i == 3) return strdup("{\"metas\":[]}");
      return strdup("{\"metas\":[{\"id\":\"ttfixture\",\"name\":\"New release\",\"poster\":\"p.jpg\"}]}");
    }
    return NULL;
  }
  if(strstr(url,"/slow/")) {
    pthread_mutex_lock(&fakeLock);slowStarted=1;pthread_cond_broadcast(&fakeCond);
    while(!releaseSlow)pthread_cond_wait(&fakeCond,&fakeLock);
    pthread_mutex_unlock(&fakeLock);
    return strdup("{\"metas\":[{\"id\":\"ttold\",\"name\":\"OLD\",\"poster\":\"old.jpg\"}]}");
  }
  if(strstr(url,"/error/"))return NULL;
  if(strstr(url,"/empty/"))return strdup("{\"metas\":[]}");
  if(strstr(url,"skip=2"))return strdup("{\"metas\":[{\"id\":\"tt3\",\"name\":\"Third\",\"poster\":\"3.jpg\"}]}");
  if(strstr(url,"skip=3"))return strdup("{\"metas\":[]}");
  return strdup("{\"metas\":[{\"id\":\"tt1\",\"name\":\"First\",\"poster\":\"1.jpg\"},{\"id\":\"tt2\",\"name\":\"Second\",\"poster\":\"2.jpg\"}]}");
}
char *rede_baixar_st(const char *url, int timeout, const char *const *headers, int *status) {
  (void)headers;
  char *body = rede_baixar(url, timeout);
  *status = strstr(url, "https://collection.invalid/") ? fixtureStatus : body ? 200 : 0;
  return body;
}
static void waitDone(void) {for(int i=0;i<2000&&desc_vertudo_carregando();i++)usleep(1000);assert(!desc_vertudo_carregando());}
int main(void) {
  desc_vertudo_abrir("https://example.invalid","movie","rank");waitDone();
  assert(desc_vertudo_n()==2&&!desc_vertudo_fim());
  desc_vertudo_mais();waitDone();assert(desc_vertudo_n()==3);
  CatItem i;assert(desc_vertudo_item(2,&i)&&!strcmp(i.imdb,"tt3"));
  desc_vertudo_mais();waitDone();assert(desc_vertudo_fim());
  desc_vertudo_abrir("https://example.invalid/slow","movie","slow");
  pthread_mutex_lock(&fakeLock);while(!slowStarted)pthread_cond_wait(&fakeCond,&fakeLock);pthread_mutex_unlock(&fakeLock);
  desc_vertudo_abrir("https://example.invalid/new","series","new");
  pthread_mutex_lock(&fakeLock);releaseSlow=1;pthread_cond_broadcast(&fakeCond);pthread_mutex_unlock(&fakeLock);
  waitDone();assert(desc_vertudo_n()==2);assert(desc_vertudo_item(0,&i)&&!strcmp(i.tipo,"series")&&!strcmp(i.imdb,"tt1"));
  desc_vertudo_abrir("https://example.invalid/error","movie","error");waitDone();assert(desc_vertudo_erro()&&!desc_vertudo_fim());
  int before=calls;desc_vertudo_mais();waitDone();assert(calls>before);
  desc_vertudo_abrir("https://example.invalid/empty","movie","empty");waitDone();assert(!desc_vertudo_erro()&&desc_vertudo_fim()&&desc_vertudo_n()==0);
  assert(col_carregar("tests/fixtures/collections/new-tabs") == 1);
  const ColFolder *folder = col_folder(0);
  assert(folder && folder->nSources == 4);
  assert(!strcmp(folder->sources[0].title, "New movies"));
  assert(!strcmp(folder->sources[1].title, "New series"));
  for (int tab = 0; tab < folder->nSources; tab++) {
    const ColSource *s = &folder->sources[tab];
    desc_vertudo_filtro(s->base, s->type, s->catId, s->genre); waitDone();
    if (desc_vertudo_erro()) fprintf(stderr, "FAIL tab=%s: catalog path returned %d\n", s->title, fixtureStatus);
    assert(!desc_vertudo_erro() && desc_vertudo_n() == 1);
    assert(desc_vertudo_item(0, &i) && !strcmp(i.tipo, s->type));
    char tabUrl[NV_ADDON_PEDIDO_MAX];
    snprintf(tabUrl, sizeof tabUrl, "%s", fixtureUrl);
    CatItem initial[2]; DescColecaoResultado result;
    assert(desc_colecao_inicial(s, s->base, nv_perfil_sessao_ler(), initial, 1, &result));
    assert(!result.erro && result.n == 1 && result.pagina == 1);
    assert(!strcmp(tabUrl, fixtureUrl)); // Vitrine and folder tabs use the same request.
    if (tab < 2) {
      desc_vertudo_mais(); waitDone();
      assert(!desc_vertudo_erro() && desc_vertudo_fim() && desc_vertudo_n() == 1);
    }
  }
  fixtureReject = 1;
  desc_vertudo_abrir(folder->sources[1].base, "series", folder->sources[1].catId); waitDone();
  assert(desc_vertudo_erro() && !desc_vertudo_n() && !desc_vertudo_fim());
  fixtureReject = 0;
  desc_vertudo_mais(); waitDone();
  assert(!desc_vertudo_erro() && desc_vertudo_n() == 1);
  // Home and search must encode the same catalog identity too.
  int responded = 0;
  assert(lerCatalogo(folder->sources[3].base, "movie", folder->sources[3].catId,
                     &i, 1, 1, &responded) == 1 && responded);
  AlvoBusca target = {0};
  snprintf(target.base, sizeof target.base, "%s", folder->sources[1].base);
  snprintf(target.tipo, sizeof target.tipo, "series");
  snprintf(target.id, sizeof target.id, "%s", folder->sources[1].catId);
  assert(consultarAlvo(&target, "new & fresh", &i, 1) == 1);
  assert(!strcmp(fixtureUrl, "https://collection.invalid/addon/catalog/series/new%2Fseries%20%231%3Ffresh%25/search=new%20%26%20fresh.json?config=fixture"));
  puts("collections data: PASS (retry, empty, stale tabs, encoded folder/Home/search/Vitrine catalogs, raw skip, types, addon identity)");
}
