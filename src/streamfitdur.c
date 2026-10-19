#include "streamfitdur.h"
#include <ctype.h>
#include <string.h>
#include <strings.h>

static const char *espacos(const char *p) { while (*p == ' ' || *p == '\t') p++; return p; }

// Reads a non-negative integer of at most 5 digits; -1 when there is none.
static long numero(const char **pp) {
  const char *p = *pp;
  long v = 0;
  int k = 0;
  while (isdigit((unsigned char)*p)) {
    if (++k > 5) return -1;
    v = v * 10 + (*p++ - '0');
  }
  if (!k) return -1;
  *pp = p;
  return v;
}

static int unidade(const char **pp, const char *const *nomes, int n) {
  for (int i = 0; i < n; i++) {
    size_t t = strlen(nomes[i]);
    if (!strncasecmp(*pp, nomes[i], t) && !isalpha((unsigned char)(*pp)[t])) { *pp += t; return 1; }
  }
  return 0;
}

static double faixa(long seg) { return seg >= 60 && seg <= 86400 ? (double)seg : 0.0; }

static double iso(const char *p) {
  long h = 0, m = 0, s = 0, v;
  int algum = 0;
  if (strncasecmp(p, "PT", 2)) return 0.0;
  p += 2;
  while (*p) {
    if ((v = numero(&p)) < 0) return 0.0;
    switch (toupper((unsigned char)*p)) {
      case 'H': if (h || m || s) return 0.0; h = v; break;
      case 'M': if (m || s) return 0.0; m = v; break;
      case 'S': if (s) return 0.0; s = v; break;
      default: return 0.0;
    }
    p++; algum = 1;
  }
  return algum ? faixa(h * 3600 + m * 60 + s) : 0.0;
}

double streamfitdur_texto(const char *s) {
  static const char *const H[] = { "hours", "hour", "hrs", "hr", "horas", "hora", "h" };
  static const char *const M[] = { "minutes", "minute", "minutos", "minuto", "mins", "min", "m" };
  const char *p;
  long h = -1, m = -1, v;
  if (!s) return 0.0;
  p = espacos(s);
  if (toupper((unsigned char)p[0]) == 'P') {
    char t[32];
    size_t n = strlen(p);
    while (n && (p[n - 1] == ' ' || p[n - 1] == '\t')) n--;
    if (n >= sizeof t) return 0.0;
    memcpy(t, p, n); t[n] = 0;
    return iso(t);
  }
  if ((v = numero(&p)) < 0) return 0.0;
  p = espacos(p);
  if (!*p) return faixa(v * 60); // bare numeric runtime is minutes
  if (unidade(&p, H, 7)) {
    h = v; p = espacos(p);
    if (*p) {
      if ((v = numero(&p)) < 0) return 0.0;
      p = espacos(p);
      if (!unidade(&p, M, 7)) return 0.0;
      m = v;
    }
  } else if (unidade(&p, M, 7)) m = v;
  else return 0.0;
  if (*espacos(p)) return 0.0;
  return faixa((h > 0 ? h : 0) * 3600 + (m > 0 ? m : 0) * 60);
}

double streamfitdur_meta_filme(const char *meta) {
  const char *p = meta, *sep;
  if (!meta) return 0.0;
  for (;;) {
    char seg[48];
    size_t n;
    sep = strstr(p, "\xc2\xb7");
    n = sep ? (size_t)(sep - p) : strlen(p);
    if (n < sizeof seg) {
      const char *q;
      int unidadeExplicita = 0;
      memcpy(seg, p, n); seg[n] = 0;
      for (q = seg; *q; q++) if (isalpha((unsigned char)*q)) unidadeExplicita = 1;
      if (unidadeExplicita) {
        double d = streamfitdur_texto(seg);
        if (d > 0) return d;
      }
    }
    if (!sep) return 0.0;
    p = sep + 2;
  }
}
