#include "imdbnota.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
  assert(imdbnota_obter("tt1",72,0)==72);
  imdbnota_publicar("tt1",81);
  assert(imdbnota_obter("tt1",72,0)==81);
  assert(imdbnota_obter("tt1:2:3",69,0)==81);
  assert(imdbnota_obter("tt2",72,0)==72);
  assert(imdbnota_obter("tmdb:1",84,0)==0);
  assert(imdbnota_obter("tmdb:1:2:3",84,0)==0);
  assert(imdbnota_obter("",84,0)==0);
  imdbnota_alias_tmdb("tt1",1,1);
  assert(imdbnota_obter("tmdb:1",84,1)==81);
  assert(imdbnota_obter("tmdb:1:2:3",84,1)==81);
  assert(imdbnota_obter("tmdb:1",84,0)==0);
  imdbnota_alias_tmdb("tt999",2,0);
  assert(imdbnota_obter("tmdb:2",84,0)==0);
  imdbnota_publicar("tt2",90);
  assert(imdbnota_obter("tt1",0,0)==81 && imdbnota_obter("tt2",0,0)==90);
  imdbnota_publicar("tt1",0);imdbnota_publicar("tt1",101);
  assert(imdbnota_obter("tt1",0,0)==81);
  for(int i=0;i<300;i++){char id[32];snprintf(id,sizeof id,"tt%d",100+i);imdbnota_publicar(id,75);}
  assert(imdbnota_obter("tt399",0,0)==75);
  assert(imdbnota_obter("tt1",72,0)==72); // bounded cache eviction retains honest fallback
  puts("IMDb rating: verified update, identity, aliases, TMDB exclusion and bounded eviction passed");
}
