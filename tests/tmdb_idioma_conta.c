// #187: "DA INTERFACE" NAO SOBE PARA A CONTA.
//
// O relato: Guia em coreano numa conta com tmdb_language "ru". O registro da
// 1.5.3 (29/09) mostrava, no mesmo arranque, `tmdb_language="ru" nao
// reconhecido; mantido` e `1 ajuste(s) desta TV entram no blob da conta`: a TV
// ficava no indice 0 ("Da interface") e a costura escrevia o sentinela
// "interface" por cima do "ru". Este teste fixa as duas pontas:
//   1. valor 0 local nunca escreve "interface" no blob (a chave fica intacta);
//   2. um idioma escolhido de verdade continua subindo ("ko" para 한국어);
//   3. "ru" da conta continua chegando como Русский / ru-RU;
//   4. a linha de diagnostico do registro diz o que a TV pede e o que a conta tem.
#include "../src/ajustes.c"
#include <assert.h>

static const char *BLOB =
  "{\"tmdb_settings\":{\"tmdb_enabled\":{\"type\":\"boolean\",\"value\":true},"
  "\"tmdb_language\":{\"type\":\"string\",\"value\":\"ru\"}}}";

int main(void) {
  char *out = NULL;

  // 1. Local "Da interface", conta "ru": nada sobe.
  valor[AJ_TMDB_IDIOMA] = 0;
  assert(ajustes_mesclar_blob(BLOB, &out) == 0);
  assert(out == NULL);

  // 2. Escolha local de verdade sobe com o codigo do web.
  valor[AJ_TMDB_IDIOMA] = 9;   // 한국어
  assert(!strcmp(ajustes_tmdb_idioma(), "ko-KR"));
  assert(ajustes_mesclar_blob(BLOB, &out) == 1);
  assert(out && strstr(out, "\"value\":\"ko\"") && !strstr(out, "interface"));
  free(out); out = NULL;
  ajustes_tmdb_idioma_relatar(BLOB);

  // 3. A conta manda "ru": vira Русский e o TMDB recebe ru-RU.
  valor[AJ_TMDB_IDIOMA] = 0;
  ajustes_aplicar_blob(BLOB);
  assert(valor[AJ_TMDB_IDIOMA] == 13);
  assert(!strcmp(ajustes_tmdb_idioma(), "ru-RU"));
  assert(ajustes_mesclar_blob(BLOB, &out) == 0);   // igual a conta: nada sobe
  ajustes_tmdb_idioma_relatar(BLOB);

  // 4. Conta antiga com regiao ("pt-br", 22 logs no D1): vira Portugues
  // (Brasil), nao "nao reconhecido; mantido". "pt-pt" continua exato.
  valor[AJ_TMDB_IDIOMA] = 0;
  ajustes_aplicar_blob("{\"tmdb_settings\":{\"tmdb_language\":{\"type\":\"string\",\"value\":\"pt-br\"}}}");
  assert(valor[AJ_TMDB_IDIOMA] == 1);
  assert(!strcmp(ajustes_tmdb_idioma(), "pt-BR"));
  ajustes_aplicar_blob("{\"tmdb_settings\":{\"tmdb_language\":{\"type\":\"string\",\"value\":\"pt-pt\"}}}");
  assert(valor[AJ_TMDB_IDIOMA] == 7);

  printf("ok: tmdb_idioma_conta\n");
  return 0;
}
