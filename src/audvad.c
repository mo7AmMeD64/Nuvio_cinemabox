// Ver audvad.h. Matematica pura: nenhum fio, nenhum malloc por quadro.
#include "audvad.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define VAD_ACIMA_DB    12.0f   // band energy above the noise floor
#define VAD_ABS_DB     -60.0f   // absolute gate (dBFS of the band)
#define VAD_FRACAO       0.30f  // band share of the frame energy
#define VAD_FLUXO        0.12f  // smoothed energy-weighted spectral flux
#define VAD_ONSET        3
#define VAD_SOLTA        15
#define VAD_JUNTA_S      0.30
#define VAD_MIN_S        0.20
#define BIN_LO           10     // 312 Hz (31.25 Hz/bin)
#define BIN_HI           128    // 4000 Hz

// RBJ biquad, Q = 1/sqrt(2).
static void biquad(double f0, int passaAlta, double b[3], double a[2]) {
  double w = 2.0 * M_PI * f0 / AUDVAD_HZ, c = cos(w), al = sin(w) / (2.0 * 0.70710678);
  double a0 = 1.0 + al;
  if (passaAlta) { b[0] = (1.0 + c) / 2.0; b[1] = -(1.0 + c); b[2] = (1.0 + c) / 2.0; }
  else           { b[0] = (1.0 - c) / 2.0; b[1] = 1.0 - c;    b[2] = (1.0 - c) / 2.0; }
  b[0] /= a0; b[1] /= a0; b[2] /= a0;
  a[0] = -2.0 * c / a0; a[1] = (1.0 - al) / a0;
}

void audvad_iniciar(AudVad *v) {
  int i;
  memset(v, 0, sizeof *v);
  biquad(300.0, 1, v->hb, v->ha);
  biquad(3400.0, 0, v->lb, v->la);
  for (i = 0; i < AUDVAD_FFT / 2; i++) {
    v->cosT[i] = (float)cos(2.0 * M_PI * i / AUDVAD_FFT);
    v->sinT[i] = (float)-sin(2.0 * M_PI * i / AUDVAD_FFT);
  }
  for (i = 0; i < AUDVAD_QUADRO; i++)
    v->hann[i] = (float)(0.5 - 0.5 * cos(2.0 * M_PI * i / (AUDVAD_QUADRO - 1)));
}

// In-place iterative radix-2 FFT, AUDVAD_FFT points.
static void fft(const AudVad *v, float *re, float *im) {
  int n = AUDVAD_FFT, i, j = 0, k, len;
  for (i = 1; i < n; i++) {
    int bit = n >> 1;
    for (; j & bit; bit >>= 1) j ^= bit;
    j ^= bit;
    if (i < j) { float t = re[i]; re[i] = re[j]; re[j] = t; t = im[i]; im[i] = im[j]; im[j] = t; }
  }
  for (len = 2; len <= n; len <<= 1) {
    int passo = n / len;
    for (i = 0; i < n; i += len)
      for (k = 0; k < len / 2; k++) {
        float wr = v->cosT[k * passo], wi = v->sinT[k * passo];
        float xr = re[i + k + len / 2] * wr - im[i + k + len / 2] * wi;
        float xi = re[i + k + len / 2] * wi + im[i + k + len / 2] * wr;
        re[i + k + len / 2] = re[i + k] - xr; im[i + k + len / 2] = im[i + k] - xi;
        re[i + k] += xr; im[i + k] += xi;
      }
  }
}

static void empurrar(AudVad *v, double ini, double fim) {
  if (fim - ini <= 0) return;
  if (v->nseg && ini - v->seg[v->nseg - 1].fim < VAD_JUNTA_S) {
    if (fim > v->seg[v->nseg - 1].fim) v->seg[v->nseg - 1].fim = fim;
    return;
  }
  // A short isolated burst (a door, a cough) is dropped only when the next
  // one cannot merge with it: decide when the following segment arrives.
  if (v->nseg && v->seg[v->nseg - 1].fim - v->seg[v->nseg - 1].inicio < VAD_MIN_S) v->nseg--;
  if (v->nseg >= AUDVAD_MAX_SEG) { v->transbordou = 1; return; }
  v->seg[v->nseg].inicio = ini; v->seg[v->nseg].fim = fim; v->nseg++;
}

static void quadro(AudVad *v) {
  float re[AUDVAD_FFT], im[AUDVAD_FFT];
  double et = 0, eb = 0, somaMag = 0, fluxo = 0;
  float piso = 1e9f, lb, f = 0;
  int i, ativo;
  double t0 = v->quadroUs / 1e6, t1 = t0 + (double)AUDVAD_QUADRO / AUDVAD_HZ;
  for (i = 0; i < AUDVAD_QUADRO; i++) {
    double x = v->quadro[i] / 32768.0, y;
    et += x * x;
    y = v->hb[0] * x + v->hb[1] * v->hx1 + v->hb[2] * v->hx2 - v->ha[0] * v->hy1 - v->ha[1] * v->hy2;
    v->hx2 = v->hx1; v->hx1 = x; v->hy2 = v->hy1; v->hy1 = y;
    x = y;
    y = v->lb[0] * x + v->lb[1] * v->lx1 + v->lb[2] * v->lx2 - v->la[0] * v->ly1 - v->la[1] * v->ly2;
    v->lx2 = v->lx1; v->lx1 = x; v->ly2 = v->ly1; v->ly1 = y;
    eb += y * y;
    re[i] = (float)(v->quadro[i] / 32768.0) * v->hann[i]; im[i] = 0;
  }
  for (; i < AUDVAD_FFT; i++) re[i] = im[i] = 0;
  et /= AUDVAD_QUADRO; eb /= AUDVAD_QUADRO;
  fft(v, re, im);
  for (i = BIN_LO; i < BIN_HI; i++) {
    float m = sqrtf(re[i] * re[i] + im[i] * im[i]);
    if (m > v->magAnt[i]) fluxo += m - v->magAnt[i];
    somaMag += m;
    v->magAnt[i] = m;
  }
  v->fluxo[v->ifluxo] = somaMag > 1e-9 ? (float)(fluxo / somaMag) : 0.0f;
  v->ifluxo = (v->ifluxo + 1) % 10;
  for (i = 0; i < 10; i++) f += v->fluxo[i];
  f /= 10.0f;
  lb = (float)(10.0 * log10(eb + 1e-12));
  v->piso[v->ipiso] = lb; v->ipiso = (v->ipiso + 1) % AUDVAD_PISO_N;
  if (v->npiso < AUDVAD_PISO_N) v->npiso++;
  for (i = 0; i < v->npiso; i++) if (v->piso[i] < piso) piso = v->piso[i];
  ativo = lb > VAD_ABS_DB && lb > piso + VAD_ACIMA_DB && eb > VAD_FRACAO * et && f > VAD_FLUXO;
  v->quadros++;
  if (ativo) {
    v->quadrosFala++;
    if (!v->ativos) v->inicioFala = v->falando ? v->inicioFala : t0;
    v->ativos++; v->inativos = 0; v->ultimoAtivo = t1;
    if (!v->falando && v->ativos >= VAD_ONSET) v->falando = 1;
  } else {
    if (!v->falando) v->ativos = 0;
    else if (++v->inativos >= VAD_SOLTA) {
      empurrar(v, v->inicioFala, v->ultimoAtivo);
      v->falando = 0; v->ativos = 0; v->inativos = 0;
    }
  }
}

void audvad_pcm(AudVad *v, const int16_t *a, int n, int64_t pts) {
  int i;
  for (i = 0; i < n; i++) {
    if (!v->nq) v->quadroUs = pts + (int64_t)i * 1000000 / AUDVAD_HZ;
    v->quadro[v->nq++] = a[i];
    if (v->nq == AUDVAD_QUADRO) { quadro(v); v->nq = 0; }
  }
}

void audvad_fechar(AudVad *v) {
  if (v->falando) empurrar(v, v->inicioFala, v->ultimoAtivo);
  v->falando = 0; v->ativos = v->inativos = 0;
  if (v->nseg && v->seg[v->nseg - 1].fim - v->seg[v->nseg - 1].inicio < VAD_MIN_S) v->nseg--;
}

// --- alignment ----------------------------------------------------------------------
const char *audalign_motivo(AudAlignMotivo m) {
  static const char *const n[] = { "accepted_candidate", "no_speech", "continuous_audio",
                                   "no_subtitle_activity", "low_confidence", "ambiguous_peak",
                                   "short_window" };
  return m >= 0 && m < (int)(sizeof n / sizeof *n) ? n[m] : "invalid";
}

#define PASSO_S 0.05

static void marcar(unsigned char *g, int n, double base, const AudSeg *s, int ns) {
  int i, k;
  for (i = 0; i < ns; i++) {
    int a = (int)ceil((s[i].inicio - base) / PASSO_S - 0.5), b = (int)floor((s[i].fim - base) / PASSO_S - 0.5);
    if (a < 0) a = 0;
    if (b >= n) b = n - 1;
    for (k = a; k <= b; k++) g[k] = 1;
  }
}

static double pontuar(const unsigned char *A, const unsigned char *C, int K, int s, int ta) {
  int k, inter = 0, tb = 0;
  for (k = 0; k < K; k++) { tb += C[k + s]; inter += A[k] & C[k + s]; }
  if (ta <= 0 || tb <= 0) return 0;
  {
    double den = ta + tb - inter, azar = (double)ta * tb / K / (ta + tb - (double)ta * tb / K);
    double j = inter / den;
    return azar < 1 ? fmax(0, (j - azar) / (1 - azar)) : 0;
  }
}

AudAlign audalign_estimar(const AudSeg *fala, int nFala, const AudSeg *cues, int nCues,
                          double ini, double fim, int raioMs) {
  AudAlign r = { .motivo = AUDALIGN_JANELA };
  int K = (int)((fim - ini) / PASSO_S), R = raioMs / 50, s, ta = 0, tc = 0, k, melhor = 0;
  unsigned char *A, *C;
  double *sc;
  if (K < 600 || raioMs < 1000 || raioMs > 120000) return r;   // < 30 s of audio
  A = calloc((size_t)K, 1); C = calloc((size_t)K + 2 * R + 1, 1); sc = calloc((size_t)(2 * R + 1), sizeof *sc);
  if (!A || !C || !sc) { free(A); free(C); free(sc); return r; }
  marcar(A, K, ini, fala, nFala);
  // Cue time t+o for audio time t: the cue grid starts raio earlier.
  marcar(C, K + 2 * R + 1, ini - R * PASSO_S, cues, nCues);
  for (k = 0; k < K; k++) { ta += A[k]; tc += C[k + R]; }
  r.falaFracao = (double)ta / K;
  if (ta * PASSO_S < 20.0 || r.falaFracao < 0.05) { r.motivo = AUDALIGN_SEM_FALA; goto fim; }
  if (r.falaFracao > 0.85) { r.motivo = AUDALIGN_CONTINUA; goto fim; }
  if (tc * PASSO_S < 20.0) { r.motivo = AUDALIGN_SEM_LEGENDA; goto fim; }
  for (s = 0; s <= 2 * R; s++) {
    sc[s] = pontuar(A, C, K, s, ta);
    if (sc[s] > sc[melhor]) melhor = s;
  }
  r.score = sc[melhor];
  r.offsetMs = (melhor - R) * 50;
  for (s = 0; s <= 2 * R; s++) if (abs(s - melhor) * 50 > 500 && sc[s] > r.alternativo) r.alternativo = sc[s];
  if (r.score < 0.30) r.motivo = AUDALIGN_CONFIANCA;
  else if (r.score - r.alternativo < 0.08) r.motivo = AUDALIGN_AMBIGUO;
  else r.motivo = AUDALIGN_OK;
fim:
  free(A); free(C); free(sc);
  return r;
}
