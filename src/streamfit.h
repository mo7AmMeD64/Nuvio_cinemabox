// Stream-fit: arithmetic and a bounded, in-memory history of real measurements.
// No I/O, probes, resolution guesses or URL/credential persistence.
#ifndef NV_STREAMFIT_H
#define NV_STREAMFIT_H
#include <stdint.h>

#define STREAMFIT_HOSTS_MAX 32
#define STREAMFIT_HOST_MAX 160
#define STREAMFIT_AMOSTRAS_MAX 48
#define STREAMFIT_IDADE_MS UINT64_C(86400000)
#define STREAMFIT_BYTES_MAX UINT64_C(9007199254740991)

typedef enum { SF_DESCONHECIDA = 0, SF_ADEQUADA, SF_PESADA } StreamfitClasse;
typedef enum {
  SF_SEM_REDE = 0, SF_SEM_TAMANHO, SF_SEM_DURACAO, SF_SEM_HOST,
  SF_SEM_MEDIDA, SF_BITRATE_ESTIMADO
} StreamfitRazao;
typedef enum { SF_DUR_DESCONHECIDA = 0, SF_DUR_METADATA, SF_DUR_MEDIA } StreamfitDuracao;
// Provenance of a host's current window. A newer completed window replaces
// the older one whichever origin it has; the snapshot keeps the origin so the
// UI can say where the number came from.
typedef enum {
  SF_ORIGEM_NENHUMA = 0, SF_ORIGEM_DIAGNOSTICO, SF_ORIGEM_PASSIVA
} StreamfitOrigem;

typedef struct {
  char host[STREAMFIT_HOST_MAX]; // canonical public authority, never a URL
  int amostras, otimoKbps, maximoKbps, medianaKbps, sustentadoKbps; // sustained = p20
  int origem; // StreamfitOrigem
  uint64_t medidaMs;
} StreamfitHost;

// A value object. Captured once on sheet open, including the time/freshness
// decision. New sources use this same snapshot; updates cannot move focus.
typedef struct {
  uint64_t rede, agoraMs;
  int n;
  StreamfitHost hosts[STREAMFIT_HOSTS_MAX];
} StreamfitFoto;

typedef struct {
  StreamfitClasse classe;
  StreamfitRazao razao;
  double necessarioKbps;
  int otimoKbps, maximoKbps, amostras, sustentadoKbps;
  int origem; // StreamfitOrigem; NENHUMA unless a host window was used
  uint64_t idadeMs;
} StreamfitResultado;

// Opaque per-device/network epoch from the caller. 0 = unknown. A change
// clears history, including returning to an earlier network. No SSID needed.
void streamfit_rede(uint64_t chave);
void streamfit_limpar(void);

// Feed ONLY a completed diagnostic of real media from this final host, on
// this network. At least five valid one-second intervals (zeros included)
// are required. Failed/error clips, cache, pauses and synthetic speed-test
// endpoints must not be submitted. URL is used only to derive a public host.
// `fimMs` and snapshot time use the same wall-clock milliseconds. Clock
// rollback/future samples become unavailable. Returns intervals accepted.
// A newer completed test replaces the previous test of that host, so retest
// cannot extend the life of old measurements or mix different test windows.
int streamfit_diagnostico(uint64_t rede, const char *urlFinal,
                         const int *kbps, int n, uint64_t fimMs);
// Passive playback telemetry (Android ParaleloDataSource only). Same rules as
// the diagnostic: final host, unchanged network, 5..48 valid one-second
// intervals of media transfer (stalls included as zero; cache, pause and
// buffer-full idle excluded by the producer). The caller is responsible for
// rejecting stale playback generations (streamfitpassiva.c).
int streamfit_passiva(uint64_t rede, const char *urlFinal,
                      const int *kbps, int n, uint64_t fimMs);
void streamfit_foto(StreamfitFoto *saida, uint64_t agoraMs);
uint64_t streamfit_agora_ms(void);

// Exact per-file bytes (not season packs / textual GB), real item runtime.
// Missing or invalid provenance must be supplied as zero. Average demand is
// bytes*8 / seconds / 1000; the budget is existing p20*.75 (vazao.h).
// Passing this estimate does not guarantee that peaks won't buffer.
StreamfitClasse streamfit_classificar(const StreamfitFoto *foto,
                    const char *url, uint64_t bytes, double duracaoSeg,
                    StreamfitResultado *saida);

// Stable partition of an already sorted group, preserving both subsequences.
// Unknown/adequate first, heavy last. `tmp` has n entries and does not alias
// ordem. Invalid indexes behave as unknown. Never removes a source.
void streamfit_particionar(int *ordem, int n, const unsigned char *classes,
                          int total, int *tmp);
#endif
