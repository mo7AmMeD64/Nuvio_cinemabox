// AUDIO SYNC, PURE PART (F06, 1.8): speech detection on mono 16 kHz PCM and
// alignment of the detected speech with the cues of the external subtitle.
// No threads, no SDL, no allocation per frame: audsync.c owns the session and
// the PCM ring; tests feed synthetic PCM straight into this module.
//
// VAD: 20 ms frames. A frame is speech-like when the energy of the speech band
// (300-3400 Hz, two biquads) stands well above an adaptive noise floor (minimum
// over the last 5 s), the band holds a real share of the frame's energy and the
// smoothed spectral flux (512-point FFT, 300-4000 Hz) shows syllable-rate
// change. Onset needs 3 frames, release 15; the segment ends at the last
// speech-like frame (the hangover never stretches it). Gaps < 300 ms merge;
// segments < 200 ms are dropped. Sustained music/tones raise the floor and keep
// the flux low: they produce no speech.
//
// ALIGNMENT: speech and cue activity on a 50 ms grid, chance-adjusted overlap
// (the same family of score as autosync.c) for every offset in +-raio. The
// result is only a CANDIDATE: audsync.c turns it into a cropped window and the
// AutoSync engine decides (its thresholds are not relaxed for audio).
#ifndef NV_AUDVAD_H
#define NV_AUDVAD_H
#include <stdint.h>

#define AUDVAD_HZ        16000
#define AUDVAD_QUADRO    320            // 20 ms
#define AUDVAD_FFT       512
#define AUDVAD_PISO_N    250            // 5 s of frames for the noise floor
#define AUDVAD_MAX_SEG   4096

typedef struct { double inicio, fim; } AudSeg;   // media time, seconds

typedef struct {
  // filters (direct form I, two cascaded biquads)
  double hx1, hx2, hy1, hy2, lx1, lx2, ly1, ly2;
  double hb[3], ha[2], lb[3], la[2];
  float cosT[AUDVAD_FFT / 2], sinT[AUDVAD_FFT / 2], hann[AUDVAD_QUADRO];
  int16_t quadro[AUDVAD_QUADRO];
  int nq;
  int64_t quadroUs;            // media time of quadro[0]
  float magAnt[AUDVAD_FFT / 2];
  float piso[AUDVAD_PISO_N];
  int npiso, ipiso;
  float fluxo[10];
  int ifluxo;
  int ativos, inativos, falando;
  double inicioFala, ultimoAtivo;
  AudSeg seg[AUDVAD_MAX_SEG];
  int nseg, transbordou;
  long quadros, quadrosFala;
} AudVad;

void audvad_iniciar(AudVad *v);
// Continuous PCM: pts of samples[0]. A discontinuity is the caller's job
// (audvad_iniciar again, or audvad_fechar + start a new window).
void audvad_pcm(AudVad *v, const int16_t *amostras, int n, int64_t ptsUs);
// Closes an open speech segment (end of the window).
void audvad_fechar(AudVad *v);

typedef enum {
  AUDALIGN_OK = 0, AUDALIGN_SEM_FALA, AUDALIGN_CONTINUA, AUDALIGN_SEM_LEGENDA,
  AUDALIGN_CONFIANCA, AUDALIGN_AMBIGUO, AUDALIGN_JANELA
} AudAlignMotivo;
const char *audalign_motivo(AudAlignMotivo m);   // stable English reason for logs

typedef struct {
  AudAlignMotivo motivo;
  int offsetMs;          // positive advances the subtitle (autosync.h sign)
  double score, alternativo, falaFracao;
} AudAlign;

// fala: speech segments inside [ini, fim]; cues: subtitle activity (already
// filtered: dialogue only), media time. raioMs <= 120000.
AudAlign audalign_estimar(const AudSeg *fala, int nFala, const AudSeg *cues, int nCues,
                          double ini, double fim, int raioMs);

#endif
