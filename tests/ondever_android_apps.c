// Production worker with a gated slow JNI double; no Android device/network.
#define __ANDROID__ 1
#define NV_ANDROID 1
#include <pthread.h>
#include <errno.h>
static int failCreate;
static int createTest(pthread_t *t, const pthread_attr_t *a, void *(*fn)(void *), void *u) {
  if (failCreate) { failCreate=0;return EAGAIN; }
  return pthread_create(t,a,fn,u);
}
#define pthread_create createTest
#include "../src/ondever.c"
#undef main
#include <assert.h>
#include <unistd.h>
static pthread_mutex_t gate=PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond=PTHREAD_COND_INITIALIZER;
static int calls, entered, releaseQuery, fail;
static const char *response="installed.netflix\tNetflix\ninstalled.disney\tDisney+\n";
char *android_listar_apps(void) {
  pthread_mutex_lock(&gate);calls++;entered=1;pthread_cond_broadcast(&cond);
  while(!releaseQuery)pthread_cond_wait(&cond,&gate);
  char *r=fail?NULL:strdup(response);pthread_mutex_unlock(&gate);return r;
}
int android_abrir_app(const char *id) { (void)id;return 1; }
int android_abrir_loja(const char *id,const char *name) { (void)id;(void)name;return 1; }
const char *desc_chave_tmdb_reserva(void) { return ""; }
const char *ajustes_tmdb_idioma(void) { return "pt-BR"; }
char *dados_ler(const char *name) { (void)name;return NULL; }
int dados_gravar_leve(const char *name,const char *body) { (void)name;(void)body;return 1; }
char *rede_baixar(const char *url,int timeout) { (void)url;(void)timeout;return NULL; }
static void start(void) {
  pthread_mutex_lock(&gate);entered=releaseQuery=0;pthread_mutex_unlock(&gate);
  // Must return even though the callback cannot complete until finish().
  ondever_apps_atualizar();
  pthread_mutex_lock(&gate);while(!entered)pthread_cond_wait(&cond,&gate);pthread_mutex_unlock(&gate);
}
static void finish(void) {
  pthread_mutex_lock(&gate);releaseQuery=1;pthread_cond_broadcast(&cond);pthread_mutex_unlock(&gate);
  for(int i=0;i<2000;i++) {
    pthread_mutex_lock(&appsTrava);int busy=appsAndroidBuscando;pthread_mutex_unlock(&appsTrava);
    if(!busy)return;usleep(1000);
  }
  assert(!"worker completion timeout");
}
int main(void) {
  failCreate=1;ondever_apps_atualizar();
  assert(!appsAndroidBuscando && !appsAndroidPronto && calls==0);
  start();assert(ondever_estado("Netflix")==ONDE_INFO);
  for(int i=0;i<1000;i++)ondever_apps_atualizar();
  assert(calls==1);finish();
  assert(ondever_estado("Netflix")==ONDE_ABRIR);assert(ondever_estado("Disney+")==ONDE_ABRIR);
  char id[96];assert(appDo("Netflix",id,sizeof id)&&!strcmp(id,"installed.netflix"));
  fail=1;start();assert(ondever_estado("Netflix")==ONDE_ABRIR);finish();
  assert(ondever_estado("Netflix")==ONDE_ABRIR);assert(calls==2);
  fail=0;response="new.prime\tPrime Video\n";start();
  // Entire previous snapshot is preserved throughout refresh.
  assert(ondever_estado("Disney+")==ONDE_ABRIR);assert(ondever_estado("Prime Video")==ONDE_LOJA);
  finish();assert(ondever_estado("Netflix")==ONDE_LOJA);assert(ondever_estado("Prime Video")==ONDE_ABRIR);
  response="";start();finish();assert(ondever_estado("Prime Video")==ONDE_LOJA);
  puts("Android apps: nonblocking, coalescing, pending, snapshots, failure/retry and empty success ok");
}
