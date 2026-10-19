// Ver audsync.h. Nucleo sem SDL/GL; a ponte Android fica em video_android.c.
#include "audsync.h"
#include <ctype.h>
#include <math.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DESC_N      512
#define SALTO_US    200000        // pts jump that breaks the window
#define CORTE_FOLGA 0.6           // a cut point keeps this much silence on both sides
#define GUARDA_S    1.0

typedef struct { int64_t pts; uint32_t pos, n; int quebra; } Desc;

static pthread_mutex_t M = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t C = PTHREAD_COND_INITIALIZER;
static void (*backendLigar)(int);
static atomic_int formato;
static int travadoTeste;   // M: tests stall the worker to fill the ring

static struct {
  // ring (M)
  int16_t ring[AUDSYNC_RING];
  uint32_t wIni, wUsado;
  Desc desc[DESC_N];
  int dIni, dN, quebraPendente;
  int armado;
  // session (M)
  int fio, parar;
  pthread_t thread;
  uint64_t pedido, sessao, geracao;
  int alvoMs, raioMs;
  LegendaDocumento *prim;
  int pausado, resetar, alinhar;
  AudSyncStatus st;
  LegendaDocumento *ref, *recorte;
  int tapAplicado;
  // worker only
  AudVad vad;
  int64_t spanIni, esperado;
  int temSpan;
  int16_t local[4096];
} S;

const char *audsync_motivo(AudSyncMotivo m) {
  static const char *const n[] = { "ok", "platform", "passthrough", "no_speech", "continuous_audio",
                                   "no_subtitle_activity", "low_confidence", "short_window",
                                   "out_of_memory" };
  return m >= 0 && m < (int)(sizeof n / sizeof *n) ? n[m] : "invalid";
}

void audsync_backend(void (*ligar)(int)) {
  pthread_mutex_lock(&M); backendLigar = ligar; S.tapAplicado = 0; pthread_mutex_unlock(&M);
}

AudSyncCap audsync_capacidade(void) {
  void (*b)(int);
  int f = atomic_load(&formato);
  pthread_mutex_lock(&M); b = backendLigar; pthread_mutex_unlock(&M);
  if (!b) return AUDSYNC_CAP_PLATAFORMA;
  if (f == AUDSYNC_FMT_BITSTREAM) return AUDSYNC_CAP_PASSTHROUGH;
  if (f != AUDSYNC_FMT_PCM) return AUDSYNC_CAP_SEM_AUDIO;
  return AUDSYNC_CAP_OK;
}

static void limparRing(void) { S.wIni = S.wUsado = 0; S.dIni = S.dN = 0; S.quebraPendente = 1; }

static void falhar(AudSyncMotivo m) {   // with M
  // No log here: stderr may block and the playback thread takes this lock.
  // legsync.c logs the outcome on the UI thread.
  S.st.fase = AUDSYNC_FALHOU; S.st.motivo = m; S.armado = 0; S.alinhar = 0;
}

void audsync_formato(int f) {
  int antes = atomic_exchange(&formato, f);
  pthread_mutex_lock(&M);
  if (S.st.fase == AUDSYNC_OUVINDO) {
    // Bitstream never reaches the tap: report it, never switch passthrough off.
    if (f == AUDSYNC_FMT_BITSTREAM) falhar(AUDSYNC_M_PASSTHROUGH);
    else if (f != antes) { S.resetar = 1; limparRing(); }
  }
  pthread_mutex_unlock(&M);
}

int audsync_pcm(const int16_t *a, int n, int64_t pts) {
  uint32_t fimW, primeiro;
  if (!a || n <= 0) return 0;
  pthread_mutex_lock(&M);
  if (!S.armado || S.pausado) { pthread_mutex_unlock(&M); return 0; }
  if ((uint32_t)n > AUDSYNC_RING - S.wUsado || S.dN >= DESC_N || n > (int)(sizeof S.local / sizeof *S.local)) {
    // Worker behind: drop and break the timeline; never wait here.
    S.st.descartados++; S.quebraPendente = 1;
    pthread_mutex_unlock(&M); return 0;
  }
  fimW = (S.wIni + S.wUsado) % AUDSYNC_RING;
  primeiro = AUDSYNC_RING - fimW < (uint32_t)n ? AUDSYNC_RING - fimW : (uint32_t)n;
  memcpy(S.ring + fimW, a, primeiro * sizeof *a);
  if (primeiro < (uint32_t)n) memcpy(S.ring, a + primeiro, ((uint32_t)n - primeiro) * sizeof *a);
  {
    Desc *d = &S.desc[(S.dIni + S.dN) % DESC_N];
    d->pts = pts; d->pos = fimW; d->n = (uint32_t)n; d->quebra = S.quebraPendente;
    S.quebraPendente = 0; S.dN++; S.wUsado += (uint32_t)n;
  }
  pthread_cond_signal(&C);
  pthread_mutex_unlock(&M);
  return 1;
}

void audsync_teste_travar(int t) {
  pthread_mutex_lock(&M); travadoTeste = t; pthread_cond_broadcast(&C); pthread_mutex_unlock(&M);
}

int audsync_teste_livre(void) {
  int l;
  pthread_mutex_lock(&M); l = (int)(AUDSYNC_RING - S.wUsado); pthread_mutex_unlock(&M);
  return l;
}

// --- documents ------------------------------------------------------------------------
// Dialogue only, like the engine: signs, music/SDH and absurd durations out.
static int dialogo(const LegendaCue *c) {
  const char *s = c->texto;
  size_t n;
  double d = c->fim - c->inicio;
  if (d < 0.20 || d > 15.0 || c->an >= 4) return 0;
  if (c->posY >= 0 && c->resY > 0 && c->posY < c->resY * 0.65f) return 0;
  while (isspace((unsigned char)*s)) s++;
  if (strstr(s, "\xe2\x99\xaa") || strstr(s, "\xe2\x99\xab")) return 0;
  n = strlen(s);
  if (n > 1 && ((s[0] == '[' && s[n - 1] == ']') || (s[0] == '(' && s[n - 1] == ')'))) return 0;
  return n > 0;
}

static int ocupado(const AudSeg *v, int n, double t, double desloc) {
  int i;
  for (i = 0; i < n; i++)
    if (v[i].inicio - desloc - CORTE_FOLGA < t && v[i].fim - desloc + CORTE_FOLGA > t) return 1;
  return 0;
}

// Cut point near `t` (searching in `dir`) with silence in BOTH timelines.
static int corte(const AudSeg *fala, int nf, const AudSeg *cues, int nc, double off,
                 double t, int dir, double *saida) {
  int i;
  for (i = 0; i < 400; i++) {
    double p = t + dir * i * 0.05;
    if (!ocupado(fala, nf, p, 0) && !ocupado(cues, nc, p, off)) { *saida = p; return 1; }
  }
  return 0;
}

static AudSyncMotivo deAlign(AudAlignMotivo m) {
  switch (m) {
    case AUDALIGN_OK: return AUDSYNC_M_OK;
    case AUDALIGN_SEM_FALA: return AUDSYNC_M_SEM_FALA;
    case AUDALIGN_CONTINUA: return AUDSYNC_M_CONTINUA;
    case AUDALIGN_SEM_LEGENDA: return AUDSYNC_M_SEM_LEGENDA;
    case AUDALIGN_JANELA: return AUDSYNC_M_JANELA;
    default: return AUDSYNC_M_CONFIANCA;
  }
}

// Worker, without M. Builds the reference + cropped documents.
static AudSyncMotivo montar(LegendaDocumento *prim, uint64_t pedido, double ini, double fim, int raio,
                            LegendaDocumento **ref, LegendaDocumento **recorte, AudAlign *al) {
  int n = 0, i, nc = 0, nf = 0, nr = 0, nd = 0;
  const LegendaCue *v = legenda_documento_dados(prim, &n);
  const LegendaDocumentoInfo *pi = legenda_documento_info(prim);
  AudSeg *cues = NULL, *fala = S.vad.seg;
  LegendaCue *rv = NULL, *dv = NULL;
  LegendaDocumentoInfo info;
  double w0, w1, off;
  AudSyncMotivo m = AUDSYNC_M_MEMORIA;
  *ref = *recorte = NULL;
  memset(al, 0, sizeof *al);
  if (!v || !pi || n <= 0) return AUDSYNC_M_SEM_LEGENDA;
  cues = malloc((size_t)n * sizeof *cues);
  if (!cues) return AUDSYNC_M_MEMORIA;
  for (i = 0; i < n; i++) {
    if (!dialogo(&v[i])) continue;
    if (nc && v[i].inicio <= cues[nc - 1].fim + 0.02) {
      if (v[i].fim > cues[nc - 1].fim) cues[nc - 1].fim = v[i].fim;
    } else { cues[nc].inicio = v[i].inicio; cues[nc].fim = v[i].fim; nc++; }
  }
  nf = S.vad.nseg;
  *al = audalign_estimar(fala, nf, cues, nc, ini, fim, raio);
  if (al->motivo != AUDALIGN_OK) { m = deAlign(al->motivo); goto fim; }
  off = al->offsetMs / 1000.0;
  // The shifted cues must stay inside the heard window, with a guard.
  if (!corte(fala, nf, cues, nc, off, ini + GUARDA_S + fabs(off), +1, &w0) ||
      !corte(fala, nf, cues, nc, off, fim - GUARDA_S - fabs(off), -1, &w1) || w1 - w0 < 190.0) {
    m = AUDSYNC_M_JANELA; goto fim;
  }
  rv = calloc((size_t)nf + 1, sizeof *rv); dv = calloc((size_t)n, sizeof *dv);
  if (!rv || !dv) goto fim;
  for (i = 0; i < nf; i++) {
    if (fala[i].inicio < w0 || fala[i].fim > w1) continue;
    rv[nr].inicio = fala[i].inicio; rv[nr].fim = fala[i].fim;
    snprintf(rv[nr].texto, sizeof rv[nr].texto, "fala %04d", nr + 1);   // distinct labels: no text claim
    rv[nr].cor = -1; rv[nr].posX = rv[nr].posY = -1; rv[nr].ordem = nr;
    nr++;
  }
  for (i = 0; i < n; i++) {
    double s = v[i].inicio - off;
    if (s < w0 || s > w1) continue;
    dv[nd++] = v[i];
  }
  if (nr <= 0 || nd <= 0) { m = AUDSYNC_M_SEM_FALA; goto fim; }
  memset(&info, 0, sizeof info);
  info.sessao = pi->sessao; info.flags = LEGENDA_DOC_COMPLETO;
  snprintf(info.origem, sizeof info.origem, "audio");
  snprintf(info.identidade, sizeof info.identidade, "audio:%llu", (unsigned long long)pedido);
  *ref = legenda_documento_de_cues(rv, nr, &info);
  info = *pi;
  info.duracaoSeg = 0;
  snprintf(info.identidade, sizeof info.identidade, "%.100s#janela", pi->identidade);
  *recorte = legenda_documento_de_cues(dv, nd, &info);
  if (!*ref || !*recorte) {
    legenda_documento_liberar(*ref); legenda_documento_liberar(*recorte); *ref = *recorte = NULL;
    goto fim;
  }
  m = AUDSYNC_M_OK;
fim:
  free(cues); free(rv); free(dv);
  return m;
}

static void *trabalhar(void *u) {
  (void)u;
  pthread_mutex_lock(&M);
  for (;;) {
    while (!S.parar && (travadoTeste || (!S.alinhar && !(S.st.fase == AUDSYNC_OUVINDO && (S.dN > 0 || S.resetar)))))
      pthread_cond_wait(&C, &M);
    if (S.parar) break;
    if (S.resetar) {
      S.resetar = 0; S.temSpan = 0; audvad_iniciar(&S.vad); S.st.progresso = 0;
      continue;
    }
    if (S.alinhar) {
      uint64_t ger = S.geracao, pedido = S.pedido;
      LegendaDocumento *prim = legenda_documento_reter(S.prim), *ref = NULL, *rec = NULL;
      double ini = S.spanIni / 1e6, fim = S.esperado / 1e6;
      int raio = S.raioMs;
      AudAlign al;
      AudSyncMotivo m;
      S.alinhar = 0;
      pthread_mutex_unlock(&M);
      audvad_fechar(&S.vad);
      m = montar(prim, pedido, ini, fim, raio, &ref, &rec, &al);
      legenda_documento_liberar(prim);
      fprintf(stderr, "[audsync] audio_sync window reason=%s offset_ms=%d score=%.3f alt=%.3f speech=%.2f segments=%d\n",
              audsync_motivo(m), al.offsetMs, al.score, al.alternativo, al.falaFracao, S.vad.nseg);
      pthread_mutex_lock(&M);
      if (ger == S.geracao && S.st.fase == AUDSYNC_ALINHANDO) {
        S.st.estimativaMs = al.offsetMs; S.st.confianca = al.score;
        if (m == AUDSYNC_M_OK) {
          S.ref = ref; S.recorte = rec; ref = rec = NULL;
          S.st.fase = AUDSYNC_PRONTO; S.st.motivo = AUDSYNC_M_OK;
        } else falhar(m);
      }
      pthread_mutex_unlock(&M);
      legenda_documento_liberar(ref); legenda_documento_liberar(rec);
      pthread_mutex_lock(&M);
      continue;
    }
    {
      Desc d = S.desc[S.dIni];
      uint32_t primeiro = AUDSYNC_RING - d.pos < d.n ? AUDSYNC_RING - d.pos : d.n;
      uint64_t ger = S.geracao;
      memcpy(S.local, S.ring + d.pos, primeiro * sizeof *S.local);
      if (primeiro < d.n) memcpy(S.local + primeiro, S.ring, (d.n - primeiro) * sizeof *S.local);
      S.dIni = (S.dIni + 1) % DESC_N; S.dN--;
      S.wIni = (S.wIni + d.n) % AUDSYNC_RING; S.wUsado -= d.n;
      if (!S.temSpan || d.quebra || llabs(d.pts - S.esperado) > SALTO_US) {
        if (S.temSpan) S.st.reinicios++;
        audvad_iniciar(&S.vad); S.temSpan = 1; S.spanIni = d.pts;
      }
      S.esperado = d.pts + (int64_t)d.n * 1000000 / AUDVAD_HZ;
      pthread_mutex_unlock(&M);
      audvad_pcm(&S.vad, S.local, (int)d.n, d.pts);
      pthread_mutex_lock(&M);
      if (ger != S.geracao || S.st.fase != AUDSYNC_OUVINDO) continue;
      {
        int64_t cob = S.esperado - S.spanIni;
        S.st.progresso = (int)(cob / 10000 / (S.alvoMs > 0 ? S.alvoMs / 1000 : 1));
        if (S.st.progresso > 100) S.st.progresso = 100;
        if (cob >= (int64_t)S.alvoMs * 1000) {
          S.armado = 0; S.st.fase = AUDSYNC_ALINHANDO; S.alinhar = 1; limparRing();
        }
      }
    }
  }
  pthread_mutex_unlock(&M);
  return NULL;
}

static void soltarSessao(LegendaDocumento **a, LegendaDocumento **b, LegendaDocumento **c) {   // with M
  *a = S.prim; *b = S.ref; *c = S.recorte;
  S.prim = S.ref = S.recorte = NULL;
}

uint64_t audsync_pedir(uint64_t sessao, LegendaDocumento *prim, int alvoSeg, int raioMs) {
  LegendaDocumento *a, *b, *c;
  uint64_t id = 0;
  AudSyncCap cap = audsync_capacidade();
  pthread_mutex_lock(&M);
  if (!S.fio) {
    S.parar = 0;
    if (pthread_create(&S.thread, NULL, trabalhar, NULL) == 0) S.fio = 1;
  }
  soltarSessao(&a, &b, &c);
  S.geracao++;
  memset(&S.st, 0, sizeof S.st);
  S.pedido = S.geracao; S.st.pedido = S.pedido;   // monotonic across requests
  S.sessao = sessao; S.alvoMs = (alvoSeg > 0 ? alvoSeg : AUDSYNC_ALVO_SEG) * 1000;
  S.raioMs = raioMs > 0 ? raioMs : AUDSYNC_RAIO_MS;
  S.pausado = 0; S.alinhar = 0;
  limparRing();
  if (!S.fio) { S.st.fase = AUDSYNC_FALHOU; S.st.motivo = AUDSYNC_M_MEMORIA; }
  else if (!prim) { S.st.fase = AUDSYNC_FALHOU; S.st.motivo = AUDSYNC_M_SEM_LEGENDA; }
  else if (cap == AUDSYNC_CAP_PLATAFORMA) { S.st.fase = AUDSYNC_FALHOU; S.st.motivo = AUDSYNC_M_PLATAFORMA; }
  else if (cap == AUDSYNC_CAP_PASSTHROUGH) { S.st.fase = AUDSYNC_FALHOU; S.st.motivo = AUDSYNC_M_PASSTHROUGH; }
  else {
    S.prim = legenda_documento_reter(prim);
    S.st.fase = AUDSYNC_OUVINDO; S.armado = 1; S.resetar = 1;
    pthread_cond_signal(&C);
  }
  id = S.pedido;
  pthread_mutex_unlock(&M);
  legenda_documento_liberar(a); legenda_documento_liberar(b); legenda_documento_liberar(c);
  audsync_passo();
  return id;
}

void audsync_cancelar(void) {
  LegendaDocumento *a, *b, *c;
  pthread_mutex_lock(&M);
  soltarSessao(&a, &b, &c);
  S.geracao++; S.armado = 0; S.alinhar = 0; S.pausado = 0;
  if (S.st.fase != AUDSYNC_PARADO) S.st.fase = AUDSYNC_PARADO;
  limparRing();
  pthread_mutex_unlock(&M);
  legenda_documento_liberar(a); legenda_documento_liberar(b); legenda_documento_liberar(c);
  audsync_passo();
}

void audsync_pausar(int sensivel) {
  pthread_mutex_lock(&M);
  if (S.st.fase == AUDSYNC_OUVINDO) {
    if (sensivel && !S.pausado) { S.pausado = 1; limparRing(); }
    else if (!sensivel && S.pausado) { S.pausado = 0; S.resetar = 1; pthread_cond_signal(&C); }
  } else S.pausado = 0;
  S.st.pausado = S.pausado;
  pthread_mutex_unlock(&M);
}

void audsync_passo(void) {
  void (*b)(int) = NULL;
  int quer;
  pthread_mutex_lock(&M);
  quer = S.armado;
  if (backendLigar && quer != S.tapAplicado) { b = backendLigar; S.tapAplicado = quer; }
  pthread_mutex_unlock(&M);
  if (b) b(quer);   // posts to the platform's own thread; never blocks on playback
}

AudSyncStatus audsync_status(void) {
  AudSyncStatus s;
  pthread_mutex_lock(&M); s = S.st; s.pausado = S.pausado; pthread_mutex_unlock(&M);
  return s;
}

int audsync_tomar(uint64_t pedido, LegendaDocumento **ref, LegendaDocumento **recorte) {
  int ok = 0;
  pthread_mutex_lock(&M);
  if (pedido == S.pedido && S.st.fase == AUDSYNC_PRONTO && S.ref && S.recorte) {
    *ref = S.ref; *recorte = S.recorte; S.ref = S.recorte = NULL; ok = 1;
    S.st.fase = AUDSYNC_PARADO;
    legenda_documento_liberar(S.prim); S.prim = NULL;
  }
  pthread_mutex_unlock(&M);
  return ok;
}

void audsync_destruir(void) {
  LegendaDocumento *a, *b, *c;
  int fio;
  pthread_mutex_lock(&M);
  S.parar = 1; S.armado = 0; S.geracao++; fio = S.fio;
  soltarSessao(&a, &b, &c);
  pthread_cond_broadcast(&C);
  pthread_mutex_unlock(&M);
  if (fio) pthread_join(S.thread, NULL);
  pthread_mutex_lock(&M); S.fio = 0; S.parar = 0; S.st.fase = AUDSYNC_PARADO; pthread_mutex_unlock(&M);
  legenda_documento_liberar(a); legenda_documento_liberar(b); legenda_documento_liberar(c);
  audsync_passo();
}
