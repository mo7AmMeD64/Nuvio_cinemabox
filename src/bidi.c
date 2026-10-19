// See bidi.h. Simplified Unicode Bidi Algorithm for a single line plus
// Arabic joining (Presentation Forms-B). Limits: no explicit embeddings or
// isolates, brackets are mirrored but not paired, no Arabic-specific ligatures
// besides lam-alef.
#include "bidi.h"
#include <string.h>

#define BIDI_MAX 1024

static int precisa(unsigned c) {
  return (c >= 0x0590 && c <= 0x08FF) || (c >= 0xFB1D && c <= 0xFDFF) || (c >= 0xFE70 && c <= 0xFEFF);
}

// Lenient decoder: invalid bytes become U+FFFD, one byte at a time.
static unsigned decodificar(const unsigned char *s, int *len) {
  unsigned c = s[0];
  int n, i;
  if (c < 0x80) { *len = 1; return c; }
  if (c >= 0xC2 && c <= 0xDF) { n = 1; c &= 0x1F; }
  else if (c >= 0xE0 && c <= 0xEF) { n = 2; c &= 0x0F; }
  else if (c >= 0xF0 && c <= 0xF4) { n = 3; c &= 0x07; }
  else { *len = 1; return 0xFFFD; }
  for (i = 1; i <= n; i++)
    if ((s[i] & 0xC0) != 0x80) { *len = 1; return 0xFFFD; }
    else c = (c << 6) | (s[i] & 0x3F);
  *len = n + 1;
  if ((n == 2 && c < 0x800) || (n == 3 && (c < 0x10000 || c > 0x10FFFF)) || (c >= 0xD800 && c <= 0xDFFF)) {
    *len = 1; return 0xFFFD;
  }
  return c;
}

static int codificar(unsigned c, char *d) {
  if (c < 0x80) { d[0] = (char)c; return 1; }
  if (c < 0x800) { d[0] = (char)(0xC0 | (c >> 6)); d[1] = (char)(0x80 | (c & 0x3F)); return 2; }
  if (c < 0x10000) {
    d[0] = (char)(0xE0 | (c >> 12)); d[1] = (char)(0x80 | ((c >> 6) & 0x3F)); d[2] = (char)(0x80 | (c & 0x3F)); return 3;
  }
  d[0] = (char)(0xF0 | (c >> 18)); d[1] = (char)(0x80 | ((c >> 12) & 0x3F));
  d[2] = (char)(0x80 | ((c >> 6) & 0x3F)); d[3] = (char)(0x80 | (c & 0x3F)); return 4;
}

// --- shaping ----------------------------------------------------------------
typedef struct { unsigned cp; unsigned char tipo; unsigned short iso, fin, ini, med; } Letra;
enum { J_R = 1, J_D = 2 };  // right-joining / dual-joining
#define D(c,a,b,i,m) { c, J_D, a, b, i, m }
#define R(c,a,b)     { c, J_R, a, b, 0, 0 }
#define U(c,a)       { c, 0,   a, 0, 0, 0 }
// FE80.. forms are stored as offsets-free absolute values (all fit in 16 bits).
static const Letra LETRAS[] = {
  U(0x0621,0xFE80), R(0x0622,0xFE81,0xFE82), R(0x0623,0xFE83,0xFE84), R(0x0624,0xFE85,0xFE86),
  R(0x0625,0xFE87,0xFE88), D(0x0626,0xFE89,0xFE8A,0xFE8B,0xFE8C), R(0x0627,0xFE8D,0xFE8E),
  D(0x0628,0xFE8F,0xFE90,0xFE91,0xFE92), R(0x0629,0xFE93,0xFE94),
  D(0x062A,0xFE95,0xFE96,0xFE97,0xFE98), D(0x062B,0xFE99,0xFE9A,0xFE9B,0xFE9C),
  D(0x062C,0xFE9D,0xFE9E,0xFE9F,0xFEA0), D(0x062D,0xFEA1,0xFEA2,0xFEA3,0xFEA4),
  D(0x062E,0xFEA5,0xFEA6,0xFEA7,0xFEA8), R(0x062F,0xFEA9,0xFEAA), R(0x0630,0xFEAB,0xFEAC),
  R(0x0631,0xFEAD,0xFEAE), R(0x0632,0xFEAF,0xFEB0),
  D(0x0633,0xFEB1,0xFEB2,0xFEB3,0xFEB4), D(0x0634,0xFEB5,0xFEB6,0xFEB7,0xFEB8),
  D(0x0635,0xFEB9,0xFEBA,0xFEBB,0xFEBC), D(0x0636,0xFEBD,0xFEBE,0xFEBF,0xFEC0),
  D(0x0637,0xFEC1,0xFEC2,0xFEC3,0xFEC4), D(0x0638,0xFEC5,0xFEC6,0xFEC7,0xFEC8),
  D(0x0639,0xFEC9,0xFECA,0xFECB,0xFECC), D(0x063A,0xFECD,0xFECE,0xFECF,0xFED0),
  D(0x0640,0x0640,0x0640,0x0640,0x0640),  // tatweel
  D(0x0641,0xFED1,0xFED2,0xFED3,0xFED4), D(0x0642,0xFED5,0xFED6,0xFED7,0xFED8),
  D(0x0643,0xFED9,0xFEDA,0xFEDB,0xFEDC), D(0x0644,0xFEDD,0xFEDE,0xFEDF,0xFEE0),
  D(0x0645,0xFEE1,0xFEE2,0xFEE3,0xFEE4), D(0x0646,0xFEE5,0xFEE6,0xFEE7,0xFEE8),
  D(0x0647,0xFEE9,0xFEEA,0xFEEB,0xFEEC), R(0x0648,0xFEED,0xFEEE), R(0x0649,0xFEEF,0xFEF0),
  D(0x064A,0xFEF1,0xFEF2,0xFEF3,0xFEF4),
  // Persian / Urdu with FB50 forms
  D(0x067E,0xFB56,0xFB57,0xFB58,0xFB59), D(0x0686,0xFB7A,0xFB7B,0xFB7C,0xFB7D),
  R(0x0698,0xFB8A,0xFB8B), D(0x06A9,0xFB8E,0xFB8F,0xFB90,0xFB91),
  D(0x06AF,0xFB92,0xFB93,0xFB94,0xFB95), D(0x06CC,0xFBFC,0xFBFD,0xFBFE,0xFBFF),
};
#undef D
#undef R
#undef U
#define NLETRAS ((int)(sizeof LETRAS / sizeof LETRAS[0]))

static const Letra *letra(unsigned c) {
  int lo = 0, hi = NLETRAS - 1;
  if (c < 0x0621 || c > 0x06CC) return NULL;
  while (lo <= hi) {
    int m = (lo + hi) / 2;
    if (LETRAS[m].cp == c) return &LETRAS[m];
    if (LETRAS[m].cp < c) lo = m + 1; else hi = m - 1;
  }
  return NULL;
}

// Combining marks: harakat, Quranic marks, Hebrew points, Latin diacritics.
static int marca(unsigned c) {
  return (c >= 0x0610 && c <= 0x061A) || (c >= 0x064B && c <= 0x065F) || c == 0x0670 ||
         (c >= 0x06D6 && c <= 0x06DC) || (c >= 0x06DF && c <= 0x06E4) || c == 0x06E7 || c == 0x06E8 ||
         (c >= 0x06EA && c <= 0x06ED) || (c >= 0x08D3 && c <= 0x08FF) ||
         (c >= 0x0591 && c <= 0x05BD) || c == 0x05BF || c == 0x05C1 || c == 0x05C2 || c == 0x05C4 ||
         c == 0x05C5 || c == 0x05C7 || (c >= 0x0300 && c <= 0x036F);
}
static int marca_arabe(unsigned c) { return marca(c) && c >= 0x0600; }

// Lam-alef ligature: alef variant -> {isolated, final}.
static int lamalef(unsigned alef, unsigned *iso, unsigned *fin) {
  switch (alef) {
    case 0x0622: *iso = 0xFEF5; *fin = 0xFEF6; return 1;
    case 0x0623: *iso = 0xFEF7; *fin = 0xFEF8; return 1;
    case 0x0625: *iso = 0xFEF9; *fin = 0xFEFA; return 1;
    case 0x0627: *iso = 0xFEFB; *fin = 0xFEFC; return 1;
  }
  return 0;
}

// Shapes src[0..n) into dst, returns the new length (<= n).
typedef int (*TemGlifo)(unsigned cp, void *ctx);
static unsigned forma(unsigned cp, unsigned orig, TemGlifo tem, void *ctx) {
  return (!tem || cp == orig || tem(cp, ctx)) ? cp : orig;
}

static int moldar(const unsigned *src, int n, unsigned *dst, TemGlifo tem, void *ctx) {
  int i, m = 0;
  for (i = 0; i < n; i++) {
    const Letra *l = letra(src[i]);
    int prevD = 0, nextOk = 0, j;
    unsigned c = src[i];
    if (!l) { dst[m++] = c; continue; }
    if (!l->tipo) { dst[m++] = forma(l->iso, c, tem, ctx); continue; }
    // previous non-transparent letter: does it join forward into this one?
    for (j = i - 1; j >= 0 && marca_arabe(src[j]); j--) {}
    if (j >= 0) { const Letra *p = letra(src[j]); prevD = p && p->tipo == J_D; }
    // next non-transparent letter
    for (j = i + 1; j < n && marca_arabe(src[j]); j++) {}
    if (j < n) { const Letra *q = letra(src[j]); nextOk = q && q->tipo != 0; }
    if (src[i] == 0x0644 && j < n) {
      unsigned iso, fin;
      if (lamalef(src[j], &iso, &fin) && (!tem || tem(prevD ? fin : iso, ctx))) {
        int k;
        dst[m++] = prevD ? fin : iso;
        for (k = i + 1; k < j; k++) dst[m++] = src[k];  // harakat stay after the ligature
        i = j;
        continue;
      }
    }
    if (l->tipo == J_R) c = prevD ? l->fin : l->iso;
    else if (prevD && nextOk) c = l->med;
    else if (prevD) c = l->fin;
    else if (nextOk) c = l->ini;
    else c = l->iso;
    dst[m++] = forma(c, src[i], tem, ctx);
  }
  return m;
}

// --- bidi -------------------------------------------------------------------
enum { T_L, T_R, T_EN, T_AN, T_N };

static int tipo_base(unsigned c) {
  if ((c >= '0' && c <= '9') || (c >= 0x06F0 && c <= 0x06F9)) return T_EN;
  if (c >= 0x0660 && c <= 0x0669) return T_AN;
  // Marcas de direcao: RLM e ALM sao fortes R, LRM forte L (abaixo, pelo
  // "c != 0x200E"). Quem chama as tira da saida depois (text.c, visualDe).
  if (c == 0x200F || c == 0x061C) return T_R;
  if (c == 0x060C || c == 0x061B || c == 0x061F || c == 0x066A || c == 0x066B || c == 0x066C || c == 0x06D4) return T_N;
  if ((c >= 0x0590 && c <= 0x08FF) || (c >= 0xFB1D && c <= 0xFDFF) || (c >= 0xFE70 && c <= 0xFEFF)) return T_R;
  if (c < 0x41) return T_N;
  if (c <= 0x5A) return T_L;
  if (c <= 0x60) return T_N;
  if (c <= 0x7A) return T_L;
  if (c <= 0xBF) return (c == 0xAA || c == 0xB5 || c == 0xBA) ? T_L : T_N;
  if (c == 0xD7 || c == 0xF7) return T_N;
  if ((c >= 0x2000 && c <= 0x206F && c != 0x200E) || (c >= 0x2190 && c <= 0x2BFF) ||
      (c >= 0x3000 && c <= 0x3004) || (c >= 0x3008 && c <= 0x3020) || (c >= 0xFF01 && c <= 0xFF0F) ||
      (c >= 0xFE30 && c <= 0xFE6F) || c >= 0x1F000 || c == 0xFFFD || c == 0xFEFF)
    return T_N;
  return T_L;
}

static unsigned espelho(unsigned c) {
  switch (c) {
    case '(': return ')'; case ')': return '(';
    case '[': return ']'; case ']': return '[';
    case '{': return '}'; case '}': return '{';
    case '<': return '>'; case '>': return '<';
    case 0xAB: return 0xBB; case 0xBB: return 0xAB;
  }
  return c;
}

int bidi_visual_utf8(const char *in, char *out, size_t tam) {
  return bidi_visual_utf8_ex(in, out, tam, NULL, NULL);
}

int bidi_visual_utf8_ex(const char *in, char *out, size_t tam, TemGlifo tem, void *ctx) {
  static const unsigned char vazio[1] = { 0 };
  const unsigned char *s = in ? (const unsigned char *)in : vazio;
  unsigned cp[BIDI_MAX], sh[BIDI_MAX];
  int ini[BIDI_MAX + 1], tp[BIDI_MAX], nivel[BIDI_MAX], ord[BIDI_MAX];
  int n = 0, i, len, achou = 0, ncl = 0, p, m, maxN = 0, minImp = 255;
  size_t o = 0;
  if (!tam) return 0;
  // Fast path: scan for any Hebrew/Arabic codepoint.
  for (i = 0; s[i];) {
    unsigned c = decodificar(s + i, &len);
    if (precisa(c)) achou = 1;
    i += len;
    n++;
  }
  if (!achou) {
    size_t b = (size_t)i;
    if (b >= tam) goto cru;
    memcpy(out, s, b + 1);
    return 0;
  }
  if (n > BIDI_MAX) goto cru;
  n = 0;
  for (i = 0; s[i];) { cp[n++] = decodificar(s + i, &len); i += len; }
  n = moldar(cp, n, sh, tem, ctx);
  // Clusters: a base plus the combining marks that follow it.
  for (i = 0; i < n; i++)
    if (i == 0 || !marca(sh[i])) ini[ncl++] = i;
  ini[ncl] = n;
  for (i = 0; i < ncl; i++) tp[i] = tipo_base(sh[ini[i]]);
  // Paragraph direction: first strong character.
  p = 0;
  for (i = 0; i < ncl; i++) if (tp[i] == T_L) { p = 0; break; } else if (tp[i] == T_R) { p = 1; break; }
  // Number glue: 1,234  12:30  3.5  1/2  and a % after digits.
  for (i = 1; i + 1 < ncl; i++) {
    unsigned c = sh[ini[i]];
    if (tp[i] == T_N && (c == ',' || c == '.' || c == ':' || c == '/') && tp[i - 1] == T_EN && tp[i + 1] == T_EN) tp[i] = T_EN;
  }
  for (i = 1; i < ncl; i++)
    if (tp[i] == T_N && (sh[ini[i]] == '%' || sh[ini[i]] == 0x066A) && (tp[i - 1] == T_EN || tp[i - 1] == T_AN)) tp[i] = tp[i - 1];
  // W7: a European number after strong L is L.
  { int ult = p ? T_R : T_L;
    for (i = 0; i < ncl; i++) {
      if (tp[i] == T_L || tp[i] == T_R) ult = tp[i];
      else if (tp[i] == T_EN && ult == T_L) tp[i] = T_L;
    } }
  // N0, simplified (BD16 pairs of () [] {}): a pair with a strong type of the
  // paragraph direction inside takes that direction; with only the opposite
  // inside, it takes the opposite when the context before it is also
  // opposite. "Name (2014)" in an RTL line keeps both brackets with the Latin
  // run instead of splitting them across the line.
  { int pilha[64], np = 0;
    for (i = 0; i < ncl; i++) {
      unsigned c = sh[ini[i]];
      if (tp[i] != T_N) continue;
      if (c == '(' || c == '[' || c == '{') { if (np < 64) pilha[np++] = i; continue; }
      if (c == ')' || c == ']' || c == '}') {
        unsigned abre = c == ')' ? '(' : c == ']' ? '[' : '{';
        int k = np - 1, j, e = p ? T_R : T_L, o = p ? T_L : T_R, temE = 0, temO = 0, dir;
        while (k >= 0 && sh[ini[pilha[k]]] != abre) k--;
        if (k < 0) continue;
        for (j = pilha[k] + 1; j < i; j++) {
          int t = tp[j] == T_EN || tp[j] == T_AN ? T_R : tp[j];
          if (t == e) temE = 1; else if (t == o) temO = 1;
        }
        if (temE) dir = e;
        else if (temO) {
          int antes = e;
          for (j = pilha[k] - 1; j >= 0; j--) {
            int t = tp[j] == T_EN || tp[j] == T_AN ? T_R : tp[j];
            if (t == T_L || t == T_R) { antes = t; break; }
          }
          dir = antes == o ? o : e;
        } else { np = k; continue; }
        tp[pilha[k]] = tp[i] = dir;
        np = k;
      }
    } }
  // N1/N2: neutrals between equal directions follow them, else the paragraph's.
  for (i = 0; i < ncl;) {
    int j, esq, dir;
    if (tp[i] != T_N) { i++; continue; }
    for (j = i; j < ncl && tp[j] == T_N; j++) {}
    esq = i ? tp[i - 1] : (p ? T_R : T_L);
    dir = j < ncl ? tp[j] : (p ? T_R : T_L);
    if (esq == T_EN || esq == T_AN) esq = T_R;
    if (dir == T_EN || dir == T_AN) dir = T_R;
    { int r = esq == dir ? esq : (p ? T_R : T_L);
      int k; for (k = i; k < j; k++) tp[k] = r; }
    i = j;
  }
  // Levels (I1/I2).
  for (i = 0; i < ncl; i++) {
    int t = tp[i];
    nivel[i] = p ? (t == T_R ? 1 : 2) : (t == T_L ? 0 : t == T_R ? 1 : 2);
    if (nivel[i] > maxN) maxN = nivel[i];
    if ((nivel[i] & 1) && nivel[i] < minImp) minImp = nivel[i];
    ord[i] = i;
  }
  // L2: reverse from the highest level down to the lowest odd level.
  { int lv;
    for (lv = maxN; lv >= minImp && lv >= 1; lv--)
      for (i = 0; i < ncl;) {
        int j, a, b;
        if (nivel[ord[i]] < lv) { i++; continue; }
        for (j = i; j < ncl && nivel[ord[j]] >= lv; j++) {}
        for (a = i, b = j - 1; a < b; a++, b--) { int t = ord[a]; ord[a] = ord[b]; ord[b] = t; }
        i = j;
      } }
  for (m = 0; m < ncl; m++) {
    int cl = ord[m], k;
    for (k = ini[cl]; k < ini[cl + 1]; k++) {
      unsigned c = sh[k];
      char b[4];
      int w;
      if (k == ini[cl] && (nivel[cl] & 1)) c = espelho(c);
      w = codificar(c, b);
      if (o + (size_t)w + 1 > tam) goto cru;
      memcpy(out + o, b, (size_t)w);
      o += (size_t)w;
    }
  }
  out[o] = 0;
  return 1;
cru: {
    size_t b = strlen((const char *)s);
    if (b >= tam) { b = tam - 1; while (b && (s[b] & 0xC0) == 0x80) b--; }
    memcpy(out, s, b);
    out[b] = 0;
    return -1;
  }
}
