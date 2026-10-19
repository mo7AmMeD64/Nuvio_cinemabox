// The actual menu label must identify each external subtitle's provider.
#include "../src/faixas.c"
#include <assert.h>
static Legenda externo;
int video_n_legenda(void) { return 0; }
const VideoFaixa *video_legenda(int i) { (void)i; return NULL; }
int mkvass_estado(void) { return 0; }
int mkvass_varredura(void) { return 0; }
const Legenda *addons_legenda(int i) { return i == 0 ? &externo : NULL; }
const char *i18n(const char *s) { return s; }
int main(void) {
  const char *marca;
  const char *nomes[] = {"OpenSubtitles", "Subs.ro", "Community Subtitles"};
  snprintf(externo.rotulo, sizeof externo.rotulo, "Romeno");
  for (int i = 0; i < 3; i++) {
    snprintf(externo.provedor, sizeof externo.provedor, "%s", nomes[i]);
    assert(!strcmp(rotuloLegenda(0, &marca), "Romeno") && !strcmp(marca, nomes[i]));
  }
  externo.provedor[0] = 0;
  assert(!strcmp(rotuloLegenda(0, &marca), "Romeno") && !strcmp(marca, "Legenda externa"));
  puts("addon subtitles menu: actual provider label and generic fallback ok");
}
