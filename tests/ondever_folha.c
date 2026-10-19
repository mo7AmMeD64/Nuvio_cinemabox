// Production availability module with an offline published fixture. No
// transport request or application launch is made by this test.
#include "../src/ondever.c"
#include "streams.h"
#include <SDL2/SDL.h>
#include <assert.h>
static void key(SDL_Keycode k) {
  SDL_Event event={0};event.type=SDL_KEYDOWN;event.key.keysym.sym=k;
  stream_folha_evento(&event);
}
int main(void) {
  Stream sources[2]={{0}};Stream late={0};int choice=-1;
  snprintf(publicado,sizeof publicado,"tt123");
  nItens=2;snprintf(itens[0].nome,sizeof itens[0].nome,"Netflix");snprintf(itens[1].nome,sizeof itens[1].nome,"Disney Plus");
  for(int i=0;i<2;i++) {
    snprintf(sources[i].url,sizeof sources[i].url,"https://example.invalid/source-%d.mp4",i);
    snprintf(sources[i].provedor,sizeof sources[i].provedor,"Addon");
    snprintf(sources[i].rotulo,sizeof sources[i].rotulo,"Source %d",i);
  }
  stream_definir_alvo("tt123");stream_definir_lista(sources,2);stream_definir_atual(-1);
  stream_folha_abrir();assert(stream_folha_n()==2);
  key(SDLK_UP);key(SDLK_LEFT);key(SDLK_RETURN);assert(stream_folha_n()==2);
  snprintf(late.url,sizeof late.url,"https://example.invalid/late.mp4");
  snprintf(late.provedor,sizeof late.provedor,"Earlier addon");
  stream_lista_acrescentar(&late,1,0);
  assert(stream_n()==3);assert(stream_folha_n()==2);
  assert(!strcmp(stream_item(1)->url,sources[1].url));
  // A service row never becomes a playable source index. On desktop it is
  // information only, so OK stays in the sheet with no playback request.
  key(SDLK_RETURN);assert(!stream_folha_escolheu(&choice));assert(stream_folha_aberta());
  key(SDLK_UP);key(SDLK_RIGHT);key(SDLK_RETURN);assert(stream_folha_n()==3);
  key(SDLK_RETURN);assert(stream_folha_escolheu(&choice));assert(choice>=0 && choice<3);
  puts("ondever sheet: source indexes and partial-list focus preserved");
  return 0;
}
