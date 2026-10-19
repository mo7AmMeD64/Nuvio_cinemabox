// StreamFit passive telemetry ingestion (Android playback only).
// The Kotlin meter (PassivoMedidor.kt) aggregates media bytes of the player's
// own transfers per wall-clock second and delivers windows for the FINAL host.
// This module is the native gate: it only accepts a window from the playback
// generation the UI thread currently allows, on the current network epoch.
// LG / Samsung TPK / WGT never allow a generation: they stay unknown.
#ifndef NV_STREAMFITPASSIVA_H
#define NV_STREAMFITPASSIVA_H
#include <stdint.h>

// UI thread, every frame: the backend session that is the player's own real
// source (not a trailer, channel, warning clip or idle pipeline), or 0.
// Changing it makes every window of another generation stale.
void streamfitpassiva_permitir(uint64_t geracao);
uint64_t streamfitpassiva_permitida(void);

// Any thread (JNI). `origem` is "scheme://authority" of the final host, never
// a path or query. Returns the intervals accepted (0 = rejected).
int streamfitpassiva_receber(uint64_t rede, uint64_t geracao, const char *origem,
                             const int *kbps, int n, uint64_t fimMs);

#ifdef NV_ANDROID
// Native session of the Android backend (video_android.c), the generation the
// Kotlin player receives with each open.
unsigned video_android_sessao(void);
#endif
#endif
