// Run the real discovery worker synchronously, replacing HTTP only.
#define trakt_lista_cresc bibfonte_remote_list
#include "../src/descoberta.c"
#undef trakt_lista_cresc
#include <assert.h>
static int watchlists;
int bibfonte_remote_list(const char *which, CatItem **out, int max) {
  (void)max;
  if (strcmp(which, "watchlist")) return 0;
  watchlists++;
  *out = calloc(1, sizeof **out); assert(*out);
  strcpy((*out)->imdb, "tt9999999"); strcpy((*out)->tipo, "series");
  (*out)->naLista = 1;
#ifndef BIBFONTE_BASELINE
  (*out)->listaFontes = 1u << AJ_SALVOS_TRAKT;
#endif
  return 1;
}
int bibfonte_discovery_watchlist(void) {
  watchlists = 0;
  ListasTrakt job = {0}; job.sessao = nv_perfil_sessao_ler();
  fioListas(&job);
  if (job.nWl) assert(cat_acrescentar_lote(job.wl, job.nWl, NULL) == 1);
  free(job.wl); free(job.col);
  return watchlists;
}
