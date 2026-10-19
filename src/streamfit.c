#include "streamfit.h"
#include "vazao.h"
#include <ctype.h>
#include <math.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>

typedef struct {
  char host[STREAMFIT_HOST_MAX];
  int kbps[STREAMFIT_AMOSTRAS_MAX], n;
  uint64_t quando[STREAMFIT_AMOSTRAS_MAX], recente;
  int origem;
} Historico;
static Historico historico[STREAMFIT_HOSTS_MAX];
static uint64_t redeAtual;
static pthread_mutex_t trava = PTHREAD_MUTEX_INITIALIZER;

// Strict authority parsing: refuse truncation rather than merging hosts.
// Credentials are skipped, path/query never stored. Ports remain distinct
// except the explicit default ports. HTTP and HTTPS share server capacity.
static int hostDe(const char *url, char *dst) {
  const char *p, *fim, *at;
  size_t n;
  dst[0] = 0;
  if (!url) return 0;
  if (!strncmp(url, "https://", 8)) p = url + 8;
  else if (!strncmp(url, "http://", 7)) p = url + 7;
  else return 0;
  fim = p + strcspn(p, "/?#");
  while ((at = memchr(p, '@', (size_t)(fim - p)))) p = at + 1;
  n = (size_t)(fim - p);
  if (!n || n >= STREAMFIT_HOST_MAX) return 0;
  for (size_t i = 0; i < n; i++) {
    unsigned char c = (unsigned char)p[i];
    if (!(isalnum(c) || c == '.' || c == '-' || c == ':' || c == '[' || c == ']')) return 0;
    dst[i] = (char)tolower(c);
  }
  dst[n] = 0;
  // An explicit default port identifies the same public host. A nondefault
  // port has a separate capacity history.
  if ((n > 4 && !strcmp(dst + n - 4, ":443") && url[4] == 's') ||
      (n > 3 && !strcmp(dst + n - 3, ":80") && url[4] == ':')) {
    n -= url[4] == 's' ? 4 : 3; dst[n] = 0;
  }
  if (n && dst[n - 1] == '.') dst[--n] = 0;
  return n > 0;
}

uint64_t streamfit_agora_ms(void) {
  struct timeval t;
  if (gettimeofday(&t, NULL) || t.tv_sec <= 0) return 0;
  return (uint64_t)t.tv_sec * 1000 + (uint64_t)t.tv_usec / 1000;
}
void streamfit_rede(uint64_t chave) {
  pthread_mutex_lock(&trava);
  if (chave != redeAtual) { memset(historico, 0, sizeof historico); redeAtual = chave; }
  pthread_mutex_unlock(&trava);
}
void streamfit_limpar(void) {
  pthread_mutex_lock(&trava);
  memset(historico, 0, sizeof historico); redeAtual = 0;
  pthread_mutex_unlock(&trava);
}

static int registrar(int origem, uint64_t rede, const char *url, const int *kbps, int n, uint64_t fim) {
  char host[STREAMFIT_HOST_MAX];
  int usados = 0, ix = -1, livre = -1, antigo = 0;
  if (!rede || !fim || !kbps || n < 5 || n > STREAMFIT_AMOSTRAS_MAX ||
      vazao_url_aviso(url) || !hostDe(url, host)) return 0;
  // 10 Gb/s is a generous ceiling for TV diagnostics and keeps the existing
  // percentile arithmetic safe on 32-bit platforms. Negative/malformed
  // intervals are excluded; genuine zero-byte intervals remain evidence.
  for (int i = 0; i < n; i++) if (kbps[i] >= 0 && kbps[i] <= 10000000) usados++;
  if (usados < 5) return 0;
  pthread_mutex_lock(&trava);
  if (rede != redeAtual) { pthread_mutex_unlock(&trava); return 0; }
  for (int i = 0; i < STREAMFIT_HOSTS_MAX; i++) {
    if (!strcmp(historico[i].host, host)) { ix = i; break; }
    if (!historico[i].host[0]) livre = i;
    if (historico[i].recente < historico[antigo].recente) antigo = i;
  }
  if (ix < 0) {
    ix = livre >= 0 ? livre : antigo;
    memset(&historico[ix], 0, sizeof historico[ix]);
    memcpy(historico[ix].host, host, strlen(host) + 1);
  }
  Historico *h = &historico[ix];
  // A stale or rolled-back delivery must not replace a newer observation.
  if (fim <= h->recente) { pthread_mutex_unlock(&trava); return 0; }
  h->n = 0; // this completed retest supersedes the prior window
  for (int i = 0; i < n; i++) if (kbps[i] >= 0 && kbps[i] <= 10000000) {
    h->kbps[h->n] = kbps[i]; h->quando[h->n++] = fim;
  }
  h->recente = fim; h->origem = origem;
  pthread_mutex_unlock(&trava);
  return usados;
}
int streamfit_diagnostico(uint64_t rede, const char *url, const int *kbps, int n, uint64_t fim) {
  return registrar(SF_ORIGEM_DIAGNOSTICO, rede, url, kbps, n, fim);
}
int streamfit_passiva(uint64_t rede, const char *url, const int *kbps, int n, uint64_t fim) {
  return registrar(SF_ORIGEM_PASSIVA, rede, url, kbps, n, fim);
}

void streamfit_foto(StreamfitFoto *out, uint64_t agora) {
  if (!out) return;
  memset(out, 0, sizeof *out);
  pthread_mutex_lock(&trava);
  out->rede = redeAtual; out->agoraMs = agora;
  if (redeAtual && agora) for (int i = 0; i < STREAMFIT_HOSTS_MAX; i++) {
    const Historico *h = &historico[i];
    int v[STREAMFIT_AMOSTRAS_MAX], n = 0;
    VazaoResumo r;
    uint64_t recente = 0;
    if (h->recente > agora) continue;
    for (int j = 0; j < h->n; j++)
      if (h->quando[j] <= agora && agora - h->quando[j] <= STREAMFIT_IDADE_MS) {
        v[n++] = h->kbps[j]; if (h->quando[j] > recente) recente = h->quando[j];
      }
    if (n < 5 || !vazao_resumir(v, n, &r)) continue;
    StreamfitHost *d = &out->hosts[out->n++];
    memcpy(d->host, h->host, sizeof d->host);
    d->amostras = r.n; d->otimoKbps = r.otimoKbps;
    d->maximoKbps = r.maximoKbps; d->medianaKbps = r.medianaKbps; d->medidaMs = recente;
    d->sustentadoKbps = r.p20Kbps; d->origem = h->origem;
  }
  pthread_mutex_unlock(&trava);
}

StreamfitClasse streamfit_classificar(const StreamfitFoto *foto, const char *url,
                     uint64_t bytes, double segundos, StreamfitResultado *out) {
  StreamfitResultado r = {0};
  char host[STREAMFIT_HOST_MAX];
  r.razao = SF_SEM_REDE;
  if (!foto || !foto->rede || !foto->agoraMs) goto fim;
  r.razao = SF_SEM_TAMANHO;
  if (!bytes || bytes > STREAMFIT_BYTES_MAX) goto fim;
  r.razao = SF_SEM_DURACAO;
  if (!isfinite(segundos) || segundos < 1 || segundos > 86400) goto fim;
  r.razao = SF_SEM_HOST;
  if (vazao_url_aviso(url) || !hostDe(url, host)) goto fim;
  r.razao = SF_SEM_MEDIDA;
  for (int i = 0; i < foto->n && i < STREAMFIT_HOSTS_MAX; i++) {
    const StreamfitHost *h = &foto->hosts[i];
    if (strcmp(h->host, host)) continue;
    if (h->amostras < 5 || h->medidaMs > foto->agoraMs ||
        foto->agoraMs - h->medidaMs > STREAMFIT_IDADE_MS) goto fim;
    r.necessarioKbps = (double)bytes * 8.0 / segundos / 1000.0;
    r.otimoKbps = h->otimoKbps; r.maximoKbps = h->maximoKbps;
    r.amostras = h->amostras; r.idadeMs = foto->agoraMs - h->medidaMs;
    r.sustentadoKbps = h->sustentadoKbps; r.origem = h->origem;
    r.razao = SF_BITRATE_ESTIMADO;
    r.classe = r.necessarioKbps <= r.otimoKbps ? SF_ADEQUADA : SF_PESADA;
    break;
  }
fim:
  if (out) *out = r;
  return r.classe;
}

void streamfit_particionar(int *ordem, int n, const unsigned char *classes, int total, int *tmp) {
  int k = 0;
  if (!ordem || !tmp || tmp == ordem || !classes || n < 2 || total < 1) return;
  for (int peso = 0; peso < 2; peso++) for (int i = 0; i < n; i++) {
    int ix = ordem[i], pesado = ix >= 0 && ix < total && classes[ix] == SF_PESADA;
    if (pesado == peso) tmp[k++] = ix;
  }
  memcpy(ordem, tmp, (size_t)n * sizeof *ordem);
}
