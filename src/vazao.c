// Ver vazao.h. Aritmetica pura: tests/vazao.sh compila isto sozinho.
#include "vazao.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int menor(const void *a, const void *b) {
  int x = *(const int *)a, y = *(const int *)b;
  return x < y ? -1 : x > y;
}

int vazao_resumir(const int *kbps, int n, VazaoResumo *r) {
  int v[VAZAO_AMOSTRAS_MAX], m = 0, i, k;
  if (r) memset(r, 0, sizeof *r);
  if (!kbps || !r) return 0;
  for (i = 0; i < n && m < VAZAO_AMOSTRAS_MAX; i++)
    if (kbps[i] >= 0) v[m++] = kbps[i];
  if (!m) return 0;
  qsort(v, (size_t)m, sizeof v[0], menor);
  r->n = m;
  r->medianaKbps = (m & 1) ? v[m / 2] : (int)(((long)v[m / 2 - 1] + v[m / 2] + 1) / 2);
  // Posicao mais proxima: ceil(0,2 m) - 1, em inteiro. Com 5 amostras e a
  // 1a; com 24, a 5a — nunca o minimo absoluto de uma amostra grande, que e
  // um segundo isolado e nao o "trecho ruim" tipico.
  k = (m * 20 + 99) / 100 - 1;
  if (k < 0) k = 0;
  r->p20Kbps = v[k];
  r->otimoKbps = (int)(((long)r->p20Kbps * 75 + 50) / 100);
  r->maximoKbps = (int)(((long)r->medianaKbps * 90 + 50) / 100);
  return 1;
}

double vazao_gb(int kbps, int segundos) {
  if (kbps <= 0 || segundos <= 0) return 0.0;
  // kbps / 1000 = Mbps; Mbps x s / 8 = MB; / 1000 = GB.
  return (double)kbps * (double)segundos / 8.0 / 1000000.0;
}

static void fmtUmaCasa(char *dst, size_t n, double x, char sep) {
  char *p;
  if (!dst || !n) return;
  if (x < 0) x = 0;
  if (x < 9.95) snprintf(dst, n, "%.1f", x);
  else snprintf(dst, n, "%.0f", x);
  p = strchr(dst, '.');
  if (p) *p = sep;
}

void vazao_fmt_mbps(char *dst, size_t n, int kbps, char sep) {
  fmtUmaCasa(dst, n, (double)kbps / 1000.0, sep);
}

void vazao_fmt_gb(char *dst, size_t n, double gb, char sep) {
  fmtUmaCasa(dst, n, gb, sep);
}

int vazao_url_aviso(const char *u) {
  if (!u) return 0;
  return strstr(u, "downloading.mp4") || strstr(u, "/slate") ||
         strstr(u, "slate.mp4") || strstr(u, "slate.m3u8") ||
         strstr(u, "slate.elfhosted.com") || strstr(u, "static.debridio.com") ? 1 : 0;
}

int vazao_host(const char *u, char *h, size_t n) {
  const char *p = u ? strstr(u, "://") : NULL;
  size_t k;
  if (h && n) h[0] = 0;
  if (!p || !h || n == 0) return 0;
  k = (size_t)(p + 3 - u) + strcspn(p + 3, "/?#");
  if (k >= n) k = n - 1;
  memcpy(h, u, k);
  h[k] = 0;
  return 1;
}

// Degraus pelo bitrate MEDIO tipico de cada tipo de arquivo (o que um filme
// de 2 h ocupa dividido pela duracao): remux 4K 50-60 Mbps, remux 1080p
// 25-30, WEB-DL 4K 15-25, WEB-DL 1080p 5-10. O degrau usa o OTIMO, que ja
// tem a folga para o pico de cena.
const char *vazao_dica(int otimoKbps) {
  if (otimoKbps >= 60000) return "Remux 4K deve tocar sem parar.";
  if (otimoKbps >= 30000) return "4K WEB-DL e remux 1080p sem parar; remux 4K pode pausar.";
  if (otimoKbps >= 15000) return "4K WEB-DL leve ou 1080p; evite remux.";
  if (otimoKbps >= 6000) return "Prefira 1080p WEB-DL; 4K pode pausar.";
  return "Prefira 720p ou 1080p leve.";
}

// ---------------------------------------------------------------------------
// CICLO COMPLETO / POR ADD-ON (ver vazao.h).

unsigned long vazao_chave(const char *u) {
  unsigned long h = 2166136261UL;
  if (!u || !*u) return 0;
  for (; *u; u++) { h ^= (unsigned char)*u; h = (h * 16777619UL) & 0xffffffffUL; }
  return h ? h : 1;
}

int vazao_host_publico(const char *u, char *h, size_t n) {
  const char *p = u ? strstr(u, "://") : NULL, *arroba, *fim;
  size_t k;
  if (h && n) h[0] = 0;
  if (!p || !h || n == 0) return 0;
  p += 3;
  fim = p + strcspn(p, "/?#");
  // usuario:senha@host — a credencial nunca sai daqui. O '@' so vale ANTES do
  // primeiro '/', '?' ou '#' (um '@' no caminho e parte do caminho).
  arroba = memchr(p, '@', (size_t)(fim - p));
  while (arroba) {
    const char *outra = memchr(arroba + 1, '@', (size_t)(fim - arroba - 1));
    if (!outra) break;
    arroba = outra;
  }
  if (arroba) p = arroba + 1;
  k = (size_t)(fim - p);
  if (k >= n) k = n - 1;
  memcpy(h, p, k);
  h[k] = 0;
  return k > 0;
}

int vazao_selecionar(VazCicloModo modo, const VazItem *it, int n, int cap,
                     int *fila, VazSelecao *s) {
  VazSelecao z;
  int i, j, q = 0;
  unsigned char *fora;
  if (!s) s = &z;
  memset(s, 0, sizeof *s);
  if (!it || n < 1 || !fila || cap < 1) return 0;
  s->candidatas = n;
  fora = calloc((size_t)n, 1);
  if (!fora) return 0;
  // Debrid e link repetido saem antes de qualquer ordem.
  for (i = 0; i < n; i++) {
    if (!it[i].medivel) { fora[i] = 1; s->debrid++; continue; }
    if (it[i].chave)
      for (j = 0; j < i; j++)
        if (!fora[j] && it[j].chave == it[i].chave) { fora[i] = 1; s->duplicadas++; break; }
  }
  if (modo == VCM_ADDON) {
    for (i = 0; i < n; i++) {
      int visto = 0;
      if (fora[i]) continue;
      for (j = 0; j < i; j++) if (!fora[j] && it[j].addon == it[i].addon) { visto = 1; break; }
      if (visto) continue;
      if (q < cap) fila[q++] = i; else s->foraDoLimite++;
    }
  } else if (modo == VCM_COMPLETO) {
    // Revezamento: na rodada r entra a r-esima medivel de cada add-on, na
    // ordem em que o add-on apareceu.
    unsigned char *usada = calloc((size_t)n, 1), *vez = calloc((size_t)n, 1);
    int restam = 0;
    if (!usada || !vez) { free(usada); free(vez); free(fora); return 0; }
    for (i = 0; i < n; i++) if (!fora[i]) restam++;
    while (restam > 0) {
      // Quem e a 1a nao usada do seu add-on, medido ANTES de esta rodada
      // marcar qualquer uma (senao a 2a do mesmo add-on entrava junto).
      for (i = 0; i < n; i++) {
        vez[i] = !fora[i] && !usada[i];
        for (j = 0; vez[i] && j < i; j++)
          if (!fora[j] && !usada[j] && it[j].addon == it[i].addon) vez[i] = 0;
      }
      for (i = 0; i < n; i++) {
        if (!vez[i]) continue;
        usada[i] = 1;
        restam--;
        if (q < cap) fila[q++] = i; else s->foraDoLimite++;
      }
    }
    free(usada);
    free(vez);
  } else {
    for (i = 0; i < n; i++) {
      if (fora[i]) continue;
      if (q < cap) fila[q++] = i; else s->foraDoLimite++;
    }
  }
  free(fora);
  s->fila = q;
  return q;
}

void vazao_agendar(const VazPlano *pl, const int *fila, int n, const VazOps *ops,
                   VazAgenda *out) {
  VazAgenda a;
  unsigned long ini = 0;
  int k;
  memset(&a, 0, sizeof a);
  if (!pl || !fila || !ops || !ops->medir) { if (out) *out = a; return; }
  if (ops->agora) ini = ops->agora(ops->u);
  for (k = 0; k < n; k++) {
    int sit;
    if (ops->cancelado && ops->cancelado(ops->u)) { a.cancelado = 1; break; }
    if (pl->maxMedidas > 0 && a.medidas >= pl->maxMedidas) break;
    if (pl->maxTentativas > 0 && a.tentadas >= pl->maxTentativas) break;
    if (pl->orcamentoMs && ops->agora && (pl->orcamentoAposMedida ? a.medidas > 0 : a.tentadas > 0) &&
        ops->agora(ops->u) - ini > pl->orcamentoMs) { a.semTempo = 1; break; }
    a.tentadas++;
    sit = ops->medir(fila[k], ops->u);
    if (ops->cancelado && ops->cancelado(ops->u)) a.cancelado = 1;
    if (sit == VS_OK) a.medidas++;
    else if (sit == VS_HOST_REPETIDO) a.hostRepetido++;
    else a.falhas++;
    if (ops->passo) ops->passo(ops->u, k + 1, n);
    if (a.cancelado) { k++; break; }
  }
  a.restantes = n - k;
  if (a.restantes < 0) a.restantes = 0;
  if (out) *out = a;
}

int vazao_necessario_kbps(int altura, long tamanhoMB, int duracaoS) {
  if (tamanhoMB > 0 && duracaoS > 0)
    return (int)((double)tamanhoMB * 8388.608 / (double)duracaoS + 0.5);
  if (altura >= 2160) return 25000;
  if (altura >= 1440) return 16000;
  if (altura >= 1080) return 8000;
  if (altura >= 720) return 4000;
  if (altura > 0) return 2500;
  return 0;
}

VazSuf vazao_suficiencia(const VazaoResumo *r, int necessarioKbps) {
  if (!r || necessarioKbps <= 0) return VSU_SEM_REF;
  if (r->otimoKbps >= necessarioKbps) return VSU_OK;
  if (r->maximoKbps >= necessarioKbps) return VSU_JUSTO;
  return VSU_NAO;
}

static int classe(const VazCicloRes *r) {
  return r->sit == VS_OK ? 0 : r->sit == VS_DEBRID ? 2 : 1;
}

void vazao_ordenar(const VazCicloRes *r, int n, int *ordem) {
  int i, j;
  for (i = 0; i < n; i++) ordem[i] = i;
  // Insercao: n <= 56 e a estabilidade (ordem de teste no empate) sai de graca.
  for (i = 1; i < n; i++) {
    int x = ordem[i];
    j = i - 1;
    while (j >= 0) {
      const VazCicloRes *a = &r[ordem[j]], *b = &r[x];
      int ca = classe(a), cb = classe(b), antes;
      if (ca != cb) antes = cb < ca;
      else if (ca == 0)
        antes = b->r.medianaKbps > a->r.medianaKbps ||
                (b->r.medianaKbps == a->r.medianaKbps && b->esperaMs < a->esperaMs);
      else antes = 0;
      if (!antes) break;
      ordem[j + 1] = ordem[j];
      j--;
    }
    ordem[j + 1] = x;
  }
}

int vazao_mediana_addon(const VazCicloRes *r, int n, int addon, int *usadas) {
  int v[VAZ_CICLO_LISTA_MAX], m = 0, i, a, b;
  if (usadas) *usadas = 0;
  for (i = 0; i < n && m < VAZ_CICLO_LISTA_MAX; i++)
    if (r[i].sit == VS_OK && r[i].addon == addon) v[m++] = r[i].r.medianaKbps;
  if (!m) return 0;
  if (usadas) *usadas = m;
  for (a = 1; a < m; a++) {
    int x = v[a];
    for (b = a - 1; b >= 0 && v[b] > x; b--) v[b + 1] = v[b];
    v[b + 1] = x;
  }
  return (m & 1) ? v[m / 2] : (int)(((long)v[m / 2 - 1] + v[m / 2] + 1) / 2);
}
