// StreamFit passive ingestion + runtime provenance parser (F03).
// Real streamfit.c / streamfitpassiva.c / redemarca.c / streamfitdur.c; no
// network, JVM or device. Threaded section runs under TSan (SANITIZE=thread).
#include "streamfit.h"
#include "streamfitpassiva.h"
#include "streamfitdur.h"
#include "redemarca.h"
#include <assert.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

static const uint64_t NOW = UINT64_C(1700000000000);
static int checks;
#define CHECK(x) do { assert(x); checks++; } while (0)
static int v8[8] = {8000,8000,8000,8000,8000,8000,8000,8000};
static uint64_t bytes(double kbps) { return (uint64_t)(kbps * 1000.0 * 3600.0 / 8.0); }

static StreamfitResultado classify(const char *url, double kbps) {
  StreamfitFoto f; StreamfitResultado r;
  streamfit_foto(&f, NOW + 60000);
  streamfit_classificar(&f, url, bytes(kbps), 3600, &r);
  return r;
}

static void ingestion(void) {
  streamfit_limpar();
  redemarca_observar(1, 1);                     // network epoch 1 known
  CHECK(redemarca_atual() == 1);
  // No generation allowed (LG/TPK/WGT, trailer, idle): every window is stale.
  CHECK(!streamfitpassiva_receber(1, 5, "https://cdn.invalid", v8, 8, NOW));
  streamfitpassiva_permitir(5);
  CHECK(!streamfitpassiva_receber(1, 4, "https://cdn.invalid", v8, 8, NOW)); // stale generation
  CHECK(!streamfitpassiva_receber(1, 0, "https://cdn.invalid", v8, 8, NOW));
  CHECK(!streamfitpassiva_receber(2, 5, "https://cdn.invalid", v8, 8, NOW)); // other epoch
  CHECK(!streamfitpassiva_receber(1, 5, "https://cdn.invalid/path?x=1", v8, 8, NOW)); // not an authority
  CHECK(!streamfitpassiva_receber(1, 5, "https://u:p@cdn.invalid", v8, 8, NOW));
  CHECK(!streamfitpassiva_receber(1, 5, "http://", v8, 8, NOW));
  CHECK(!streamfitpassiva_receber(1, 5, "https://cdn.invalid", v8, 4, NOW));  // < 5
  CHECK(!streamfitpassiva_receber(1, 5, "https://cdn.invalid", v8, 49, NOW)); // > 48
  CHECK(streamfitpassiva_receber(1, 5, "https://cdn.invalid", v8, 8, NOW) == 8);
  CHECK(!streamfitpassiva_receber(1, 5, "https://cdn.invalid", v8, 8, NOW));  // replay
  StreamfitResultado r = classify("https://cdn.invalid/movie.mkv?token=1", 5000);
  CHECK(r.razao == SF_BITRATE_ESTIMADO && r.origem == SF_ORIGEM_PASSIVA && r.classe == SF_ADEQUADA);
  CHECK(r.sustentadoKbps == 8000 && r.otimoKbps == 6000 && r.idadeMs == 60000);
  CHECK(classify("https://cdn.invalid/m.mkv", 7000).classe == SF_PESADA);
  // A newer diagnostic of the same host replaces the passive window, origin too.
  CHECK(streamfit_diagnostico(1, "https://cdn.invalid/probe", v8, 8, NOW + 1) == 8);
  CHECK(classify("https://cdn.invalid/m.mkv", 5000).origem == SF_ORIGEM_DIAGNOSTICO);
  CHECK(streamfitpassiva_receber(1, 5, "https://cdn.invalid", v8, 8, NOW + 2) == 8);
  CHECK(classify("https://cdn.invalid/m.mkv", 5000).origem == SF_ORIGEM_PASSIVA);
  // Resolver-only source: its URL host was never measured; it stays unknown.
  r = classify("https://resolver.invalid/resolve/realdebrid/abc/1", 1000);
  CHECK(r.classe == SF_DESCONHECIDA && r.razao == SF_SEM_MEDIDA && r.origem == SF_ORIGEM_NENHUMA);
  // Generation switch (new source / player closed) makes the old one stale.
  streamfitpassiva_permitir(6);
  CHECK(!streamfitpassiva_receber(1, 5, "https://cdn.invalid", v8, 8, NOW + 3));
  streamfitpassiva_permitir(0);
  CHECK(!streamfitpassiva_receber(1, 6, "https://cdn.invalid", v8, 8, NOW + 3));
  // Network change clears the history and refuses windows of the old epoch.
  streamfitpassiva_permitir(6);
  redemarca_observar(2, 1);
  CHECK(classify("https://cdn.invalid/m.mkv", 5000).classe == SF_DESCONHECIDA);
  CHECK(!streamfitpassiva_receber(1, 6, "https://cdn.invalid", v8, 8, NOW + 4));
  CHECK(streamfitpassiva_receber(2, 6, "https://cdn.invalid", v8, 8, NOW + 4) == 8);
  redemarca_observar(3, 0);                     // disconnected: unknown network
  CHECK(!streamfitpassiva_receber(3, 6, "https://cdn.invalid", v8, 8, NOW + 5));
  CHECK(classify("https://cdn.invalid/m.mkv", 10).razao == SF_SEM_REDE);
}

static void runtime(void) {
  CHECK(streamfitdur_texto("142 min") == 142 * 60);
  CHECK(streamfitdur_texto(" 45min ") == 45 * 60);
  CHECK(streamfitdur_texto("1h 52min") == 112 * 60);
  CHECK(streamfitdur_texto("1h52m") == 112 * 60);
  CHECK(streamfitdur_texto("2h") == 7200);
  CHECK(streamfitdur_texto("2 hours") == 7200);
  CHECK(streamfitdur_texto("PT1H52M") == 112 * 60);
  CHECK(streamfitdur_texto("pt45m") == 45 * 60);
  CHECK(streamfitdur_texto("38") == 38 * 60);   // numeric runtime = minutes
  CHECK(streamfitdur_texto("") == 0 && streamfitdur_texto(NULL) == 0);
  CHECK(streamfitdur_texto("0") == 0 && streamfitdur_texto("0 min") == 0);
  CHECK(streamfitdur_texto("45-60 min") == 0);  // a range is not a runtime
  CHECK(streamfitdur_texto("~45 min") == 0);
  CHECK(streamfitdur_texto("45.5 min") == 0);
  CHECK(streamfitdur_texto("1500 min") == 0);   // > 24 h
  CHECK(streamfitdur_texto("2024") == 0);       // 33 h: a year, not minutes
  CHECK(streamfitdur_texto("3 seasons") == 0);
  CHECK(streamfitdur_texto("45 min per episode") == 0);
  CHECK(streamfitdur_texto("PT") == 0 && streamfitdur_texto("PT1M2H") == 0);
  CHECK(streamfitdur_meta_filme("2024  \xc2\xb7  142 min") == 142 * 60);
  CHECK(streamfitdur_meta_filme("142 min") == 142 * 60);
  CHECK(streamfitdur_meta_filme("2024") == 0);  // year only
  CHECK(streamfitdur_meta_filme("1999 \xc2\xb7 3 temporadas") == 0);
  CHECK(streamfitdur_meta_filme("") == 0 && streamfitdur_meta_filme(NULL) == 0);
}

static void *producer(void *arg) {
  (void)arg;
  for (int i = 0; i < 3000; i++)
    streamfitpassiva_receber(9, 11, "https://tsan.invalid", v8, 8, NOW + 100 + (unsigned)i);
  return NULL;
}
static void *gate(void *arg) {
  (void)arg;
  for (int i = 0; i < 3000; i++) streamfitpassiva_permitir(i % 3 ? 11 : 12);
  streamfitpassiva_permitir(11);
  return NULL;
}
static void concurrent(void) {
  pthread_t a, b;
  streamfit_limpar(); redemarca_observar(9, 1);
  CHECK(!pthread_create(&a, NULL, producer, NULL));
  CHECK(!pthread_create(&b, NULL, gate, NULL));
  for (int i = 0; i < 2000; i++) { StreamfitFoto f; streamfit_foto(&f, NOW + 100000); CHECK(f.n <= 1); }
  CHECK(!pthread_join(a, NULL) && !pthread_join(b, NULL));
  CHECK(streamfitpassiva_receber(9, 11, "https://tsan.invalid", v8, 8, NOW + 5000) == 8);
}

int main(void) {
  setvbuf(stdout, NULL, _IOFBF, 1 << 16);
  ingestion(); runtime(); concurrent();
  fprintf(stderr, "streamfit_passiva: PASS (%d checks: generation/epoch/authority/5..48/replay, origin, resolver unknown, runtime parser, threaded gate)\n", checks);
  return 0;
}
