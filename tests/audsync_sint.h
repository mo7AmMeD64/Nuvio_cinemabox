// Synthetic material for the audio-sync tests (F06): a dialogue timeline, the
// matching SRT and mono 16 kHz PCM of "speech" (voiced syllables with gliding
// pitch and harmonics), steady music chords and background noise. Deterministic.
#ifndef NV_AUDSYNC_SINT_H
#define NV_AUDSYNC_SINT_H
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SINT_MAX 400
typedef struct { double ini, fim; } SintCue;
static SintCue sintCues[SINT_MAX];
static int sintN;
static uint32_t sintSemente = 12345;
static double sintRand(void) { sintSemente = sintSemente * 1664525u + 1013904223u; return (sintSemente >> 8) / 16777216.0; }

// Cues from `de` to `ate` seconds: 1.0-3.5 s lines, 0.7-3.0 s gaps.
static void sint_timeline(double de, double ate, uint32_t semente) {
  double t = de;
  sintSemente = semente; sintN = 0;
  while (t < ate && sintN < SINT_MAX) {
    double d = 1.0 + 2.5 * sintRand();
    sintCues[sintN].ini = t; sintCues[sintN].fim = t + d; sintN++;
    t += d + 0.7 + 2.3 * sintRand();
  }
}

static char *sint_srt(void) {
  size_t cap = (size_t)sintN * 120 + 16, o = 0;
  char *b = malloc(cap);
  int i;
  for (i = 0; i < sintN; i++) {
    int a = (int)lround(sintCues[i].ini * 1000), z = (int)lround(sintCues[i].fim * 1000);
    o += (size_t)snprintf(b + o, cap - o, "%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nLine %d says something new\n\n",
                          i + 1, a / 3600000, a / 60000 % 60, a / 1000 % 60, a % 1000,
                          z / 3600000, z / 60000 % 60, z / 1000 % 60, z % 1000, i + 1);
  }
  return b;
}

// Speech at cue time c happens in the audio at c - offset (positive offset =
// the subtitle must be advanced, autosync.h sign).
static int sint_falando(double t, double offsetS, double *desde) {
  int lo = 0, hi = sintN - 1;
  while (lo <= hi) {
    int m = (lo + hi) / 2;
    double a = sintCues[m].ini - offsetS, z = sintCues[m].fim - offsetS;
    if (t < a) hi = m - 1; else if (t >= z) lo = m + 1; else { *desde = t - a; return m + 1; }
  }
  return 0;
}

enum { SINT_FALA = 1, SINT_MUSICA = 2, SINT_RUIDO = 4 };
// Fills n samples starting at media time t0 (seconds).
static void sint_pcm(int16_t *o, int n, double t0, double offsetS, int tipo) {
  int i, k;
  for (i = 0; i < n; i++) {
    double t = t0 + i / 16000.0, x = 0, desde;
    int linha;
    if ((tipo & SINT_FALA) && (linha = sint_falando(t, offsetS, &desde)) != 0) {
      double ciclo = fmod(desde, 0.24);
      if (ciclo < 0.18) {
        int sil = (int)(desde / 0.24);
        double f0 = 110.0 + 15.0 * ((linha * 7 + sil * 3) % 9), env;
        double fase;
        env = ciclo < 0.01 ? ciclo / 0.01 : ciclo > 0.17 ? (0.18 - ciclo) / 0.01 : 1.0;
        // pitch glide +-20% across the syllable: harmonics move between FFT bins
        fase = 2 * M_PI * f0 * (ciclo + 0.2 * ciclo * ciclo / 0.18 * ((sil & 1) ? 1 : -1));
        for (k = 1; k <= 10; k++) {
          double hz = f0 * k, peso = 1.0 / k;
          if (hz > 500 && hz < 900) peso *= 3.0;     // first formant
          if (hz > 1200 && hz < 2000) peso *= 2.0;   // second formant
          x += peso * sin(k * fase);
        }
        x *= 0.06 * env;
      }
    }
    if (tipo & SINT_MUSICA) {
      static const double acordes[4][3] = { { 220, 277.2, 329.6 }, { 196, 246.9, 293.7 },
                                            { 174.6, 220, 261.6 }, { 246.9, 311.1, 370 } };
      int a = (int)(t / 2.0) % 4;
      for (k = 0; k < 3; k++) x += 0.05 * sin(2 * M_PI * acordes[a][k] * t);
    }
    if (tipo & SINT_RUIDO) x += 0.003 * (sintRand() * 2 - 1);
    if (x > 1) x = 1;
    if (x < -1) x = -1;
    o[i] = (int16_t)(x * 32767);
  }
}
#endif
