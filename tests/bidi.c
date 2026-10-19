// Arabic shaping + RTL ordering (src/bidi.c): Latin untouched, contextual
// forms, lam-alef, harakat, mixed ordering, bracket mirroring, Hebrew, and
// buffer / invalid-input safety. Expected codepoints are written explicitly.
#include "../src/bidi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int falhas;
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "FAIL %s:%d  %s\n", __FILE__, __LINE__, #c); falhas++; } } while (0)

static int dec(const char *s, unsigned *cp, int max) {
  const unsigned char *u = (const unsigned char *)s;
  int n = 0;
  while (*u && n < max) {
    unsigned c = *u; int l = 1;
    if (c >= 0xF0) { c = ((c & 7) << 18) | ((u[1] & 63) << 12) | ((u[2] & 63) << 6) | (u[3] & 63); l = 4; }
    else if (c >= 0xE0) { c = ((c & 15) << 12) | ((u[1] & 63) << 6) | (u[2] & 63); l = 3; }
    else if (c >= 0xC0) { c = ((c & 31) << 6) | (u[1] & 63); l = 2; }
    cp[n++] = c; u += l;
  }
  return n;
}
static void mostra(const char *rot, const char *s) {
  unsigned cp[64]; int n = dec(s, cp, 64), i;
  fprintf(stderr, "  %s:", rot);
  for (i = 0; i < n; i++) fprintf(stderr, " %04X", cp[i]);
  fprintf(stderr, "\n");
}
// Runs bidi on `in` and compares the result with the expected codepoints.
static void esperar(const char *nome, const char *in, int ret, const unsigned *exp, int nexp) {
  char out[512]; unsigned cp[64]; int n, r = bidi_visual_utf8(in, out, sizeof out);
  n = dec(out, cp, 64);
  if (r != ret || n != nexp || memcmp(cp, exp, sizeof(unsigned) * (size_t)nexp)) {
    fprintf(stderr, "FAIL %s (ret %d, want %d)\n", nome, r, ret);
    mostra("got ", out); falhas++;
  }
}
#define ESP(nome, in, ret, ...) do { static const unsigned e[] = { __VA_ARGS__ }; \
  esperar(nome, in, ret, e, (int)(sizeof e / sizeof e[0])); } while (0)

static int nega(unsigned cp, void *u) { (void)cp; (void)u; return 0; }
static int permite(unsigned cp, void *u) { (void)cp; (void)u; return 1; }
static int sem_lig(unsigned cp, void *u) { (void)u; return !(cp >= 0xFEF5 && cp <= 0xFEFC); }
static void esperar_ex(const char *nome, const char *in, int (*cb)(unsigned, void *), const unsigned *exp, int nexp) {
  char out[512]; unsigned cp[64]; int n, r = bidi_visual_utf8_ex(in, out, sizeof out, cb, NULL);
  n = dec(out, cp, 64);
  if (r != 1 || n != nexp || memcmp(cp, exp, sizeof(unsigned) * (size_t)nexp)) {
    fprintf(stderr, "FAIL %s (ret %d)\n", nome, r); mostra("got ", out); falhas++;
  }
}

int main(void) {
  char out[2048];
  // Latin / CJK / empty: byte-identical, return 0.
  { const char *t[] = { "", "Hello, world", "Caf\xc3\xa9 na\xc3\xafve", "\xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e 123 (ok)", "\xf0\x9f\x98\x80 x" };
    int i;
    for (i = 0; i < 5; i++) { CHECK(bidi_visual_utf8(t[i], out, sizeof out) == 0); CHECK(!strcmp(out, t[i])); } }
  CHECK(bidi_visual_utf8(NULL, out, sizeof out) == 0 && !out[0]);

  // "سلام": seen + lam-alef(final) + meem(isolated), reversed.
  ESP("salam", "\xd8\xb3\xd9\x84\xd8\xa7\xd9\x85", 1, 0xFEE1, 0xFEFC, 0xFEB3);
  // lam-alef alone: isolated ligature.
  ESP("lam-alef", "\xd9\x84\xd8\xa7", 1, 0xFEFB);
  // alef-madda variant after a joining letter: "بلآ" -> beh initial, lam-alef madda final.
  ESP("lam-alef-madda", "\xd8\xa8\xd9\x84\xd8\xa2", 1, 0xFEF6, 0xFE91);
  // harakat stay attached after their base: "كَتَبَ".
  ESP("harakat", "\xd9\x83\xd9\x8e\xd8\xaa\xd9\x8e\xd8\xa8\xd9\x8e", 1,
      0xFE90, 0x064E, 0xFE98, 0x064E, 0xFEDB, 0x064E);
  // a haraka between lam and alef keeps the ligature: after it.
  ESP("lam-fatha-alef", "\xd9\x84\xd9\x8e\xd8\xa7", 1, 0xFEFB, 0x064E);
  // tatweel joins on both sides: "بـب".
  ESP("tatweel", "\xd8\xa8\xd9\x80\xd8\xa8", 1, 0xFE90, 0x0640, 0xFE91);
  // right-joiners break the chain: "رد" -> two isolated... reh isolated, dal final? (reh is R: dal does not join).
  ESP("right-joiners", "\xd8\xb1\xd8\xaf", 1, 0xFEA9, 0xFEAD);
  // Persian: "پی" -> peh initial + yeh final.
  ESP("persian", "\xd9\xbe\xdb\x8c", 1, 0xFBFD, 0xFB58);
  // mixed, RTL paragraph: "مرحبا Nuvio 123" -> "Nuvio 123" LTR run stays whole, left of the word.
  ESP("mixed", "\xd9\x85\xd8\xb1\xd8\xad\xd8\xa8\xd8\xa7 Nuvio 123", 1,
      'N', 'u', 'v', 'i', 'o', ' ', '1', '2', '3', ' ', 0xFE8E, 0xFE92, 0xFEA3, 0xFEAE, 0xFEE3);
  // LTR paragraph with Arabic at the end.
  ESP("ltr-para", "Hello \xd9\x85\xd8\xb1\xd8\xad\xd8\xa8\xd8\xa7", 1,
      'H', 'e', 'l', 'l', 'o', ' ', 0xFE8E, 0xFE92, 0xFEA3, 0xFEAE, 0xFEE3);
  // digits after Arabic keep their order and go to the left.
  ESP("digits", "\xd8\xb3\xd9\x84\xd8\xa7\xd9\x85 12", 1, '1', '2', ' ', 0xFEE1, 0xFEFC, 0xFEB3);
  // Arabic-Indic digits keep internal order too.
  ESP("indic-digits", "\xd8\xb3\xd9\x84\xd8\xa7\xd9\x85 \xd9\xa1\xd9\xa2", 1, 0x0661, 0x0662, ' ', 0xFEE1, 0xFEFC, 0xFEB3);
  // thousands separator inside a number is not split: "1,234".
  ESP("number-glue", "\xd8\xb3 1,234", 1, '1', ',', '2', '3', '4', ' ', 0xFEB1);
  // brackets mirror in RTL runs: "(سلام)".
  ESP("brackets", "(\xd8\xb3\xd9\x84\xd8\xa7\xd9\x85)", 1, '(', 0xFEE1, 0xFEFC, 0xFEB3, ')');
  // leading dash / trailing question mark: "؟ سلام" -> question mark ends on the left.
  ESP("punct", "\xd8\xb3\xd9\x84\xd8\xa7\xd9\x85\xd8\x9f", 1, 0x061F, 0xFEE1, 0xFEFC, 0xFEB3);
  // Hebrew: reversed, nothing shaped.
  ESP("hebrew", "\xd7\xa9\xd7\x9c\xd7\x95\xd7\x9d", 1, 0x05DD, 0x05D5, 0x05DC, 0x05E9);
  // Hebrew + Latin
  ESP("hebrew-mixed", "\xd7\xa9\xd7\x9c\xd7\x95\xd7\x9d ok", 1, 'o', 'k', ' ', 0x05DD, 0x05D5, 0x05DC, 0x05E9);

  // --- font coverage callback ---
  // Font without presentation forms: base U+06xx codepoints, still visual order.
  { static const unsigned e1[] = { 0x0645, 0x0627, 0x0644, 0x0633 };            // "سلام" reordered, unshaped
    esperar_ex("deny-all", "\xd8\xb3\xd9\x84\xd8\xa7\xd9\x85", nega, e1, 4);
    // allow-all callback == no callback
    { static const unsigned e2[] = { 0xFEE1, 0xFEFC, 0xFEB3 };
      esperar_ex("allow-all", "\xd8\xb3\xd9\x84\xd8\xa7\xd9\x85", permite, e2, 3); }
    // font has the forms but not the lam-alef ligature: lam and alef shaped separately
    { static const unsigned e3[] = { 0xFEE1, 0xFE8E, 0xFEE0, 0xFEB3 };
      esperar_ex("no-ligature", "\xd8\xb3\xd9\x84\xd8\xa7\xd9\x85", sem_lig, e3, 4); }
    // Latin untouched whatever the callback says
    { char o[64]; CHECK(bidi_visual_utf8_ex("Hello", o, sizeof o, nega, NULL) == 0 && !strcmp(o, "Hello")); } }

  // --- safety ---
  { char p[16]; const char *a = "\xd8\xb3\xd9\x84\xd8\xa7\xd9\x85";  // result is 9 bytes
    memset(p, 'X', sizeof p);
    CHECK(bidi_visual_utf8(a, p, 10) == 1 && strlen(p) == 9);
    memset(p, 'X', sizeof p);
    CHECK(bidi_visual_utf8(a, p, 9) == -1 && strlen(p) == 8 && p[9] == 'X');   // too small: unchanged truncated
    CHECK(bidi_visual_utf8(a, p, 1) == -1 && p[0] == 0);
    memset(p, 'X', sizeof p);
    CHECK(bidi_visual_utf8(a, p, 0) == 0 && p[0] == 'X');
    // plain text, buffer smaller than the text: truncated at a char boundary
    memset(p, 'X', sizeof p);
    bidi_visual_utf8("caf\xc3\xa9!", p, 5);                 // 'caf' + 2-byte e-acute = 5 bytes + NUL needs 7
    CHECK(strlen(p) == 3 && p[4] == 'X'); }
  // invalid UTF-8 never crashes or overflows
  { const char *ruim[] = { "\xff\xfe", "\xd8", "\xd8\xb3\xd8", "\xe0\x80\x80", "\xf8\x88\x80\x80\x80 \xd8\xb3", "\xd8\xb3\xff\xd9\x84" };
    int i;
    for (i = 0; i < 6; i++) { char b[16]; bidi_visual_utf8(ruim[i], b, sizeof b); CHECK(strlen(b) < sizeof b); } }
  // very long line (> internal limit) falls back to the unchanged text
  { static char longo[6000], res[6000]; int i;
    for (i = 0; i + 2 < 4000; i += 2) memcpy(longo + i, "\xd8\xb3", 2);
    longo[i] = 0;
    CHECK(bidi_visual_utf8(longo, res, sizeof res) == -1 && !strcmp(res, longo)); }

  if (falhas) { fprintf(stderr, "%d failure(s)\n", falhas); return 1; }
  puts("bidi: ok");
  return 0;
}
