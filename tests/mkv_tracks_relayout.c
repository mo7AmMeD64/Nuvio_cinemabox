// #483: Tracks fora da janela de 320 KB (Void grande antes dele, SeekHead ok).
// A sonda devolve 0 faixas; mkv_tracks_alem busca o elemento inteiro.
#include "../src/mkv.h"
#include "../src/rede.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VOID_KB 500
#define BUFMAX (1024 * 1024)

static int falhas;
static unsigned char *arq; static long arqN;
static void ok(int c, const char *o) {
  printf("  %-60s %s\n", o, c ? "ok" : "FALHOU");
  if (!c) falhas++;
}

typedef struct { unsigned char *b; long n; } Buf;
static Buf nova(void) { Buf b; b.b = calloc(1, BUFMAX); b.n = 0; assert(b.b); return b; }
static void bytes(Buf *b, const void *p, long n) { assert(b->n + n <= BUFMAX); memcpy(b->b + b->n, p, (size_t)n); b->n += n; }
static void id(Buf *b, unsigned long v) {
  unsigned char t[4]; int n = v > 0xFFFFFF ? 4 : v > 0xFFFF ? 3 : v > 0xFF ? 2 : 1, i;
  for (i = 0; i < n; i++) t[i] = (unsigned char)(v >> (8 * (n - 1 - i))); bytes(b, t, n);
}
static void tam8(Buf *b, long long v) {
  unsigned char t[8]; int i; t[0] = 0x01; for (i = 1; i < 8; i++) t[i] = (unsigned char)(v >> (8 * (7 - i))); bytes(b, t, 8);
}
static void uintEl(Buf *b, unsigned long eid, unsigned long long v, int n) {
  unsigned char t[8]; int i; id(b, eid); { unsigned char s = (unsigned char)(0x80 | n); bytes(b, &s, 1); }
  for (i = 0; i < n; i++) t[i] = (unsigned char)(v >> (8 * (n - 1 - i))); bytes(b, t, n);
}
static void strEl(Buf *b, unsigned long eid, const char *s) {
  unsigned char z = (unsigned char)(0x80 | strlen(s)); id(b, eid); bytes(b, &z, 1); bytes(b, s, (long)strlen(s));
}
static void mestre(Buf *b, unsigned long eid, const Buf *corpo) { id(b, eid); tam8(b, corpo->n); bytes(b, corpo->b, corpo->n); }
static void binEl(Buf *b, unsigned long eid, const void *p, long n) { id(b, eid); tam8(b, n); bytes(b, p, n); }
static void seekEl(Buf *sh, const unsigned char alvo[4], long long pos) {
  Buf s = nova(); id(&s, 0x53AB); { unsigned char z = 0x84; bytes(&s, &z, 1); bytes(&s, alvo, 4); }
  uintEl(&s, 0x53AC, (unsigned long long)pos, 8); mestre(sh, 0x4DBB, &s); free(s.b);
}
static long idLen(unsigned long v) { return v > 0xFFFFFF ? 4 : v > 0xFFFF ? 3 : v > 0xFF ? 2 : 1; }

static void montar(void) {
  Buf f = nova(), ebml = nova(), info = nova(), seekHead = nova(), tracks = nova(), v = nova(), s = nova();
  Buf voidCorpo = nova(), clu = nova(), corpo = nova(), block = nova(), grupo = nova();
  long segIniL, lenPos, trAbs;
  static const unsigned char TR[4] = { 0x16, 0x54, 0xAE, 0x6B };
  char priv[512];
  snprintf(priv, sizeof priv,
           "[Script Info]\r\nScriptType: v4.00+\r\nPlayResX: 1280\r\nPlayResY: 720\r\n"
           "[V4+ Styles]\r\nFormat: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding\r\n"
           "Style: Default,Arial,48,&H00FFFFFF,&H000000FF,&H00000000,&H00000000,0,0,0,0,100,100,0,0,1,2,0,2,10,10,10,1\r\n"
           "[Events]\r\nFormat: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\r\n");

  uintEl(&ebml, 0x4286, 1, 1); strEl(&ebml, 0x4282, "matroska"); mestre(&f, 0x1A45DFA3UL, &ebml);
  id(&f, 0x18538067UL); lenPos = f.n; tam8(&f, 0); segIniL = f.n;
  uintEl(&info, 0x2AD7B1, 1000000, 4);
  seekEl(&seekHead, TR, 0);

  uintEl(&v, 0xD7, 1, 1); uintEl(&v, 0x83, 1, 1); strEl(&v, 0x86, "V_MPEG4/ISO/AVC");
  uintEl(&s, 0xD7, 2, 1); uintEl(&s, 0x83, 17, 1); strEl(&s, 0x86, "S_TEXT/ASS");
  strEl(&s, 0x22B59C, "eng");
  binEl(&s, 0x63A2, priv, (long)strlen(priv));
  mestre(&tracks, 0xAE, &v); mestre(&tracks, 0xAE, &s);

  memset(voidCorpo.b, 0, (size_t)VOID_KB * 1024); voidCorpo.n = (long)VOID_KB * 1024;
  trAbs = f.n + idLen(0x114D9B74UL) + 8 + seekHead.n + idLen(0x1549A966UL) + 8 + info.n
        + idLen(0xEC) + 8 + voidCorpo.n;
  seekHead.n = 0;
  seekEl(&seekHead, TR, trAbs - segIniL);
  mestre(&f, 0x114D9B74UL, &seekHead);
  mestre(&f, 0x1549A966UL, &info);
  mestre(&f, 0xEC, &voidCorpo);
  assert(f.n == trAbs);
  mestre(&f, 0x1654AE6BUL, &tracks);

  uintEl(&corpo, 0xE7, 1000, 4);
  { unsigned char h[4] = { 0x82, 0, 0, 0 }; const char *p = "0,0,Default,,0,0,0,,relayout";
    bytes(&block, h, 4); bytes(&block, p, (long)strlen(p)); }
  mestre(&grupo, 0xA1, &block); uintEl(&grupo, 0x9B, 1500, 2); mestre(&corpo, 0xA0, &grupo); mestre(&clu, 0x1F43B675UL, &corpo);
  bytes(&f, clu.b, clu.n);
  { Buf t = nova(); tam8(&t, f.n - segIniL); memcpy(f.b + lenPos, t.b, 8); free(t.b); }
  arq = f.b; arqN = f.n;
  assert(trAbs > 320 * 1024);
  // A sonda real le so os primeiros 320 KB; o SeekHead ja tem de estar certo
  // nesse prefixo (como no remux remontado que o #483 descreve).
  assert(arqN > 320L * 1024);
}

char *rede_baixar_trecho_st(const char *url, int s, long long ini, long long fim, long *tam,
                            int *status, int *erro, char *final, unsigned tamFinal) {
  long n; char *p; (void)s;
  if (tam) *tam = 0; if (status) *status = 206; if (erro) *erro = 0;
  if (final && tamFinal) snprintf(final, tamFinal, "%s", url);
  if (ini < 0 || fim < ini || ini >= arqN) { if (status) *status = 416; return NULL; }
  if (fim >= arqN) fim = arqN - 1;
  n = (long)(fim - ini + 1); p = malloc((size_t)n + 1); assert(p);
  memcpy(p, arq + ini, (size_t)n); if (tam) *tam = n; return p;
}
char *rede_baixar_trecho(const char *url, int s, long ini, long fim, long *tam) {
  return rede_baixar_trecho_st(url, s, ini, fim, tam, NULL, NULL, NULL, 0);
}
char *rede_baixar(const char *url, int s) { (void)url; (void)s; return NULL; }
char *rede_baixar_bin(const char *url, int s, long *n) { (void)url; (void)s; (void)n; return NULL; }
const char *rede_url_publica(const char *url, char *dst, unsigned tam) { (void)url; snprintf(dst, tam, "local"); return dst; }
long rede_corte_host(const char *url) { (void)url; return 0; }
int rede_resto_recusado(void) { return 0; }

int main(void) {
  MkvFaixa fx[MKV_MAX_FAIXAS];
  int n;
  montar();
  n = mkv_faixas_do_trecho(arq, 320L * 1024, fx, MKV_MAX_FAIXAS, NULL, 0, NULL);
  ok(n == 0, "320 KB da sonda nao alcancam o Tracks");
  n = mkv_faixas_e_caps("https://example.test/relayout.mkv", fx, MKV_MAX_FAIXAS, NULL, 0, NULL);
  ok(n == 0, "mkv_faixas_e_caps tambem para em 0 faixas");
  n = mkv_tracks_alem("https://example.test/relayout.mkv", fx, MKV_MAX_FAIXAS, NULL);
  ok(n == 2, "mkv_tracks_alem le video+legenda pelo SeekHead");
  ok(n == 2 && fx[1].tipo == 17 && !strncmp(fx[1].codec, "S_TEXT/ASS", 10),
     "legenda e S_TEXT/ASS (codec conhecido, overlay pode assumir)");
  return falhas ? 1 : 0;
}
