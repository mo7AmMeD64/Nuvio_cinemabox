#include "app_id.h"
#include "dados.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
int main(int argc,char **argv) {
  assert(argc==3);
  char root[512],expected[600];
  snprintf(root,sizeof(root),"%s/%s",argv[1],argv[2]); assert(!mkdir(root,0700));
  if(!strcmp(argv[2],"env")) { setenv("NUVIO_DADOS",root,1); unsetenv("HOME"); }
  else { unsetenv("NUVIO_DADOS"); setenv("HOME",root,1); }
#ifdef NV_DTS_DEBUG
  snprintf(expected,sizeof(expected),!strcmp(argv[2],"env")?"%s/%s":"%s/.%s",root,NV_APP_ID);
#else
  if(!strcmp(argv[2],"env")) snprintf(expected,sizeof(expected),"%s",root);
  else snprintf(expected,sizeof(expected),"%s/.nuvio",root);
#endif
  char production[600]; snprintf(production,sizeof(production),"%s/sessao.txt",root);
  FILE *file=fopen(production,"w"); assert(file); fputs("production-sentinel",file); fclose(file);
  dados_iniciar(root); assert(!strcmp(dados_dir(),expected));
#ifdef NV_DTS_DEBUG
  assert(!dados_ler("sessao.txt"));
  assert(dados_gravar("sessao.txt","debug-only"));
  char sentinel[40]={0}; file=fopen(production,"r"); assert(file); assert(fread(sentinel,1,sizeof(sentinel)-1,file)); fclose(file);
  assert(!strcmp(sentinel,"production-sentinel"));
#endif
  return 0;
}
