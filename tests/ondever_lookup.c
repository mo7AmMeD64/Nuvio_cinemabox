// Offline request/parser regressions. The production worker is run against
// mocked TMDB transport so no credentials or network access are required.
#include "../src/ondever.c"
#include <assert.h>

static pthread_mutex_t gate=PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t wake=PTHREAD_COND_INITIALIZER;
static int firstWaiting, releaseFirst;
#if defined(NV_TPK)
static int launches, stores;
static char launched[96];
static void mockList(void) {}
static void mockLaunch(const char *id) { launches++;snprintf(launched,sizeof launched,"%s",id); }
static void mockStore(const char *id) { (void)id;stores++; }
static void actions(void) {
  nv_tpk_apps_registrar(mockList,mockLaunch,mockStore);
  ondever_apps_limpar();
  assert(ondever_estado("Netflix")==ONDE_LOJA);
  assert(ondever_abrir("Netflix")==ONDE_LOJA && stores==1);
  ondever_app_visto("installed.netflix","Netflix");
  assert(ondever_abrir("Netflix")==ONDE_ABRIR && launches==1);
  assert(!strcmp(launched,"installed.netflix"));
  assert(ondever_abrir("HBO Max Amazon Channel")==ONDE_INFO);
  assert(ondever_abrir("Paramount Plus Apple TV Channel")==ONDE_INFO);
  assert(launches==1 && stores==1);
  assert(ondever_abrir("Unknown Service")==ONDE_PROCURAR && stores==2);
}
#endif
const char *desc_chave_tmdb_reserva(void) { return "test-only"; }
const char *ajustes_tmdb_idioma(void) { return "pt-BR"; }
char *dados_ler(const char *name) { (void)name; return NULL; }
int dados_gravar_leve(const char *name,const char *body) { (void)name;(void)body; return 1; }
char *rede_baixar(const char *url,int timeout) {
  (void)timeout;
  if(strstr(url,"/movie/3/")) return NULL;
  if(strstr(url,"/movie/4/")) return strdup("{\"results\":{}}");
  if(strstr(url,"/movie/1/")) {
    pthread_mutex_lock(&gate);firstWaiting=1;pthread_cond_broadcast(&wake);
    while(!releaseFirst)pthread_cond_wait(&wake,&gate);
    pthread_mutex_unlock(&gate);
    return strdup("{\"results\":{\"BR\":{\"flatrate\":[{\"provider_name\":\"Netflix\"}]}}}");
  }
  return strdup("{\"results\":{\"BR\":{\"flatrate\":[{\"provider_name\":\"Disney Plus\"}]}}}");
}
static Pedido *request(const char *id,long tmdb,unsigned version) {
  Pedido *p=calloc(1,sizeof *p);assert(p);
  snprintf(p->base,sizeof p->base,"%s",id);snprintf(p->country,sizeof p->country,"BR");
  p->tmdb=tmdb;p->ger=version;return p;
}
static void parser(void) {
  OndeVer out[ONDEVER_MAX];
  const char *body="{\"metadata\":{\"BR\":{\"flatrate\":[{\"provider_name\":\"Bogus\"}]}},\"results\":{\"US\":{\"flatrate\":[{\"provider_name\":\"US only\"}]},\"BR\":{\"flatrate\":[{\"provider_name\":\"Netflix\",\"logo_path\":\"/netflix.png\"},{\"provider_name\":\"Netflix Standard with Ads\"},{\"provider_name\":\"HBO Max Amazon Channel\"}],\"free\":[{\"provider_name\":\"Pluto TV\"}],\"rent\":[{\"provider_name\":\"Rent only\"}],\"buy\":[{\"provider_name\":\"Buy only\"}]}}}";
  assert(ondever_extrair(body,"BR",out,ONDEVER_MAX)==3);
  assert(!strcmp(out[0].nome,"Netflix"));assert(!strcmp(out[0].logo,"https://image.tmdb.org/t/p/w92/netflix.png"));
  assert(!strcmp(out[1].nome,"HBO Max Amazon Channel"));assert(!ondever_casa(out[1].nome,"HBO Max"));
  assert(out[2].gratis && !strcmp(out[2].nome,"Pluto TV"));
  assert(ondever_extrair(body,"CA",out,ONDEVER_MAX)==0);
  assert(ondever_extrair(body,"US",out,ONDEVER_MAX)==1 && !strcmp(out[0].nome,"US only"));
  assert(ondever_extrair(body,"BR",out,1)==1);
  assert(ondever_extrair(NULL,"BR",out,ONDEVER_MAX)==0);
  assert(ondever_extrair("{\"results\":{\"BR\":{","BR",out,ONDEVER_MAX)==0);
  assert(!paisValido(""));assert(!paisValido("B"));assert(!paisValido("XX"));assert(!paisValido("T1"));assert(paisValido("BR"));
  char id[32];baseDe("tt123:2:3",id,sizeof id);assert(!strcmp(id,"tt123"));
  baseDe("tmdb:456",id,sizeof id);assert(!strcmp(id,"tmdb:456"));
  baseDe("tt123?api_key=bad",id,sizeof id);assert(!id[0]);
}
static void generation(void) {
  pthread_t first,second;
  geracao=1;snprintf(pedido,sizeof pedido,"tt1");consultando=1;
  assert(pthread_create(&first,NULL,fioLista,request("tt1",1,1))==0);
  pthread_mutex_lock(&gate);while(!firstWaiting)pthread_cond_wait(&wake,&gate);pthread_mutex_unlock(&gate);
  pthread_mutex_lock(&trava);geracao=2;snprintf(pedido,sizeof pedido,"tt2");pthread_mutex_unlock(&trava);
  assert(pthread_create(&second,NULL,fioLista,request("tt2",2,2))==0);pthread_join(second,NULL);
  assert(ondever_n("tt2")==1);assert(ondever_status("tt2")==ONDE_PRONTO);
  pthread_mutex_lock(&gate);releaseFirst=1;pthread_cond_broadcast(&wake);pthread_mutex_unlock(&gate);pthread_join(first,NULL);
  OndeVer result;assert(ondever_item("tt2",0,&result));assert(!strcmp(result.nome,"Disney Plus"));assert(ondever_n("tt1")==0);
  ondever_apps_limpar();ondever_app_visto("bad\"id","Netflix");assert(nApps==0);
  ondever_app_visto("com.netflix.ninja","Netflix");assert(nApps==1);
  assert(ondever_estado("HBO Max Amazon Channel")==ONDE_INFO);
  assert(ondever_estado("Paramount Plus Apple TV Channel")==ONDE_INFO);
  char app[96];assert(appDo("Netflix",app,sizeof app));assert(!strcmp(app,"com.netflix.ninja"));
  pthread_mutex_lock(&trava);geracao=3;snprintf(pedido,sizeof pedido,"tt3");consultando=1;pthread_mutex_unlock(&trava);
  fioLista(request("tt3",3,3));
  assert(ondever_status("tt3")==ONDE_FALHOU && ondever_n("tt3")==0);
  pthread_mutex_lock(&trava);geracao=4;snprintf(pedido,sizeof pedido,"tt4");consultando=1;pthread_mutex_unlock(&trava);
  fioLista(request("tt4",4,4));
  assert(ondever_status("tt4")==ONDE_PRONTO && ondever_n("tt4")==0);
}
int main(void) { parser();generation();
#if defined(NV_TPK)
  actions();
#endif
  puts("ondever lookup: parser, entitlement and stale-response tests passed");return 0; }
