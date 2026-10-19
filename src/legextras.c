// Ver legextras.h (#201, #269).
#include "legextras.h"
#include "addonurl.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>

// Acrescenta `chave=valor` codificado. Devolve a nova posicao ou -1 se nao
// coube.
static long poe(char *dst, size_t n, long k, const char *chave, const char *valor) {
  static const char HEX[] = "0123456789ABCDEF";
  const unsigned char *v = (const unsigned char *)valor;
  int w = snprintf(dst + k, n - (size_t)k, "%s%s=", k ? "&" : "", chave);
  if (w < 0 || (size_t)(k + w) >= n) return -1;
  k += w;
  for (; *v; v++) {
    int livre = (*v >= 'A' && *v <= 'Z') || (*v >= 'a' && *v <= 'z') ||
                (*v >= '0' && *v <= '9') || *v == '-' || *v == '.' || *v == '_' || *v == '~';
    if ((size_t)k + (livre ? 1 : 3) >= n) return -1;
    if (livre) dst[k++] = (char)*v;
    else { dst[k++] = '%'; dst[k++] = HEX[*v >> 4]; dst[k++] = HEX[*v & 15]; }
  }
  dst[k] = 0;
  return k;
}

int legextras_segmento(char *dst, size_t n, const char *arquivo,
                       uint64_t tamanho, const char *hash) {
  long k = 0;
  if (!dst || !n) return -1;
  dst[0] = 0;
  // Mesma ordem do NuvioMobile#2102: hash, tamanho, nome.
  if (hash && hash[0] && (k = poe(dst, n, k, "videoHash", hash)) < 0) goto naoCoube;
  if (tamanho) {
    char t[24];
    snprintf(t, sizeof t, "%llu", (unsigned long long)tamanho);
    if ((k = poe(dst, n, k, "videoSize", t)) < 0) goto naoCoube;
  }
  if (arquivo && arquivo[0] && (k = poe(dst, n, k, "filename", arquivo)) < 0) goto naoCoube;
  return (int)k;
naoCoube:
  dst[0] = 0;
  return -1;
}

int legextras_url(char *dst, size_t n, const char *base, const char *tipo,
                  const char *id, const char *seg) {
  char idUrl[768];
  int w;
  if (!dst || !n) return 0;
  // nv_addon_url: a query da URL instalada vai DEPOIS do caminho, e o id sai
  // codificado (nv_addon_id), como no buildSubtitlesUrl do Nuvio oficial.
  if (!nv_addon_id(idUrl, sizeof idUrl, id) || !idUrl[0]) { dst[0] = 0; return 0; }
  if (seg && seg[0]) w = nv_addon_url(dst, n, base, "/subtitles/%s/%s/%s.json", tipo, idUrl, seg);
  else w = nv_addon_url(dst, n, base, "/subtitles/%s/%s.json", tipo, idUrl);
  if (w < 0 || (size_t)w >= n) { dst[0] = 0; return 0; }
  return 1;
}

static uint64_t somaLE(const unsigned char *p, size_t n) {
  uint64_t s = 0;
  size_t i;
  for (i = 0; i + 8 <= n; i += 8)
    s += (uint64_t)p[i] | (uint64_t)p[i + 1] << 8 | (uint64_t)p[i + 2] << 16 |
         (uint64_t)p[i + 3] << 24 | (uint64_t)p[i + 4] << 32 | (uint64_t)p[i + 5] << 40 |
         (uint64_t)p[i + 6] << 48 | (uint64_t)p[i + 7] << 56;
  return s;
}

int legextras_hash(const unsigned char *ini, size_t nIni,
                   const unsigned char *fim, size_t nFim,
                   uint64_t tamanho, char out[17]) {
  uint64_t h;
  if (out) out[0] = 0;
  if (!ini || !fim || !out || nIni != LEGEXTRAS_BLOCO || nFim != LEGEXTRAS_BLOCO ||
      tamanho < 2u * LEGEXTRAS_BLOCO) return 0;
  h = tamanho + somaLE(ini, nIni) + somaLE(fim, nFim);
  snprintf(out, 17, "%016llx", (unsigned long long)h);
  return 1;
}

static int hexval(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -1;
}

int legextras_nome_da_url(const char *url, char *dst, size_t n) {
  static const char *const EXT[] = { ".mkv", ".mp4", ".m4v", ".avi", ".mov", ".webm", ".ts", ".wmv" };
  const char *p, *ini, *fim;
  size_t k = 0, i, len;
  if (!dst || !n) return 0;
  dst[0] = 0;
  if (!url || !(p = strstr(url, "://"))) return 0;
  p = strchr(p + 3, '/');
  if (!p) return 0;
  fim = p + strcspn(p, "?#");
  for (ini = fim; ini > p && ini[-1] != '/'; ini--) {}
  for (; ini < fim && k + 1 < n; ini++) {
    if (*ini == '%' && fim - ini >= 3 && hexval(ini[1]) >= 0 && hexval(ini[2]) >= 0) {
      dst[k++] = (char)(hexval(ini[1]) * 16 + hexval(ini[2]));
      ini += 2;
    } else dst[k++] = *ini;
  }
  dst[k] = 0;
  len = strlen(dst);
  for (i = 0; i < sizeof EXT / sizeof *EXT; i++) {
    size_t e = strlen(EXT[i]);
    if (len > e && !strcasecmp(dst + len - e, EXT[i])) return 1;
  }
  dst[0] = 0;
  return 0;
}

int legextras_url_remota(const char *url) {
  const char *h;
  size_t n;
  if (!url) return 0;
  if (!strncasecmp(url, "https://", 8)) h = url + 8;
  else if (!strncasecmp(url, "http://", 7)) h = url + 7;
  else return 0;
  n = strcspn(h, ":/?#");
  if ((n == 9 && !strncasecmp(h, "localhost", 9)) ||
      (n >= 4 && !strncmp(h, "127.", 4)) || (n == 7 && !strncmp(h, "0.0.0.0", 7)) ||
      (h[0] == '[' && !strncmp(h, "[::1]", 5)))
    return 0;
  return 1;
}
