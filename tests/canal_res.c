// Folha de Fontes de CANAL: resolucao e codec lidos do nome (UHD/FHD/HD/SD,
// H.265). Ver stream_canal_enriquecer em src/streams.c.
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "streams.h"
#include "badges.h"

static Stream mk(const char *rotulo, const char *desc) {
  Stream s; memset(&s, 0, sizeof s);
  snprintf(s.rotulo, sizeof s.rotulo, "%s", rotulo);
  snprintf(s.descricao, sizeof s.descricao, "%s", desc);
  return s;
}
static void quer(const char *rot, const char *desc, int altura, const char *bit, int hevc) {
  Stream s = mk(rot, desc);
  stream_canal_enriquecer(&s);
  if (s.altura != altura || (bit && !(s.badges & badges_bit(bit))) ||
      (!bit && (s.badges & (badges_bit("r-4k") | badges_bit("r-1080") | badges_bit("r-720") | badges_bit("r-sd")))) ||
      (!!(s.badges & badges_bit("co-x265"))) != hevc) {
    printf("FALHOU: \"%s\" / \"%s\" -> altura %d badges %llx\n", rot, desc, s.altura,
           (unsigned long long)s.badges);
    assert(0);
  }
}
int main(void) {
  quer("PT| GLOBO SP UHD 4K", "", 2160, "r-4k", 0);
  quer("BR: HBO 2160p H.265", "", 2160, "r-4k", 1);
  quer("RO| CINEMAX FHD", "", 1080, "r-1080", 0);
  quer("TV Cultura 1080i 50fps", "", 1080, "r-1080", 0);
  quer("ESPN HD", "", 720, "r-720", 0);
  quer("SporTV 720p HEVC", "", 720, "r-720", 1);
  quer("Record SD", "", 480, "r-sd", 0);
  quer("Band 480p", "", 480, "r-sd", 0);
  quer("Xtream (principal)", "Canal HD x265", 720, "r-720", 1);
  quer("Canal sem marca", "", 0, NULL, 0);
  quer("SHDOW News", "", 0, NULL, 0);          // "HD" dentro de palavra nao conta
  { Stream s = mk("Globo 1080p", ""); s.altura = 720;   // o que o parser leu manda
    stream_canal_enriquecer(&s); assert(s.altura == 720); }
  printf("canal_res: ok\n");
  return 0;
}
