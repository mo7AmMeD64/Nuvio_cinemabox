// Onde assistir: o nome que o TMDB da ao servico casa com o nome do app na TV.
//
// Os pares sao REAIS: o lado do servico saiu de /watch/providers do TMDB
// (regiao BR, 02/10/2026) e o lado do app do listApps da OLED65C9 do dono.
// Sem nenhum id fixo de app, este casamento e a unica coisa que decide se a
// linha "Netflix" da folha abre a Netflix.
#include "ondever.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void casa(const char *servico, const char *app) {
  if (!ondever_casa(servico, app)) {
    char a[96], b[96];
    ondever_chave(servico, a, sizeof a); ondever_chave(app, b, sizeof b);
    printf("ondever: \"%s\" (%s) deveria casar com \"%s\" (%s)\n", servico, a, app, b);
    assert(0);
  }
}
static void naoCasa(const char *servico, const char *app) {
  if (ondever_casa(servico, app)) {
    printf("ondever: \"%s\" NAO deveria casar com \"%s\"\n", servico, app);
    assert(0);
  }
}

int main(void) {
  casa("Netflix", "Netflix");
  casa("Netflix Standard with Ads", "Netflix");
  casa("Netflix basic with Ads", "Netflix");
  casa("Amazon Prime Video", "Prime Video");
  casa("Amazon Prime Video with Ads", "Prime Video");
  casa("HBO Max", "HBO Max");
  casa("Max", "HBO Max");
  naoCasa("HBO Max Amazon Channel", "HBO Max");
  naoCasa("HBO Max Apple TV Channel", "Apple TV");
  casa("Disney Plus", "Disney+");
  casa("Apple TV+", "Apple TV");
  casa("Apple TV Plus", "Apple TV");
  casa("Globoplay", "Globoplay");
  casa("Claro tv+", "Claro tv+");
  casa("Paramount Plus", "Paramount+");
  naoCasa("Netflix", "Prime Video");
  naoCasa("Max", "Apple TV");
  naoCasa("Globoplay", "Google Play Filmes e TV");
  naoCasa("", "Netflix");
  OndeVer out[ONDEVER_MAX];
  assert(ondever_extrair("{\"results\":{\"BR\":{\"flatrate\":[{\"provider_name\":\"Netflix\"}],\"rent\":[{\"provider_name\":\"Rent only\"}]},\"US\":{\"free\":[{\"provider_name\":\"US only\"}]}}}","BR",out,ONDEVER_MAX)==1);
  assert(!strcmp(out[0].nome,"Netflix"));
  printf("ondever: ok\n");
  return 0;
}
