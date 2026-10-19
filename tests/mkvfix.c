// No network: the pure half of mkvfix.c (see src/mkvfix.h). A hand-built
// MKVToolNix/mkvpropedit header whose first SeekHead only points to a second
// SeekHead (the one that lists the Cues), and a normal header.
#include "mkvfix.h"
#include <stdio.h>
#include <string.h>

static int falhas;
static void ok(int c, const char *m) { printf("%s %s\n", c ? "ok  " : "FAIL", m); if (!c) falhas++; }

#define ID_SEEKHEAD 0x114D9B74u
#define ID_CUES     0x1C53BB6Bu
#define ID_INFO     0x1549A966u

// One element: id (its natural width) + 1-byte size (payload < 127).
static long el(unsigned char *o, uint32_t id, const unsigned char *p, long n) {
  long k = 0;
  int w = id > 0xFFFFFF ? 4 : id > 0xFFFF ? 3 : id > 0xFF ? 2 : 1;
  for (int i = w - 1; i >= 0; i--) o[k++] = (unsigned char)(id >> (8 * i));
  o[k++] = (unsigned char)(0x80 | n);
  memcpy(o + k, p, (size_t)n);
  return k + n;
}
static long seek(unsigned char *o, uint32_t alvo, uint32_t pos) {
  unsigned char a[4] = { alvo >> 24, alvo >> 16, alvo >> 8, alvo }, b[4] = { pos >> 24, pos >> 16, pos >> 8, pos };
  unsigned char v[32]; long n = 0;
  n += el(v + n, 0x53AB, a, 4);
  n += el(v + n, 0x53AC, b, 4);
  return el(o, 0x4DBB, v, n);
}
// EBML header + Segment of unknown size + SeekHead (the given entries) + a
// Void that brings the SeekHead region to `regiao` bytes + Info.
static long arquivo(unsigned char *o, const uint32_t *ids, const uint32_t *pos, int n, int regiao, long *segIni) {
  unsigned char s[128], z[128] = {0};
  long k = 0, m = 0, sh;
  k += el(o, 0x1A45DFA3, (const unsigned char *)"\x42\x82\x88matroska", 11);
  o[k++] = 0x18; o[k++] = 0x53; o[k++] = 0x80; o[k++] = 0x67; o[k++] = 0x01;
  memset(o + k, 0xFF, 7); k += 7;   // unknown size
  *segIni = k;
  for (int i = 0; i < n; i++) m += seek(s + m, ids[i], pos[i]);
  sh = el(o + k, ID_SEEKHEAD, s, m); k += sh;
  k += el(o + k, 0xEC, z, regiao - sh - 2);
  k += el(o + k, ID_INFO, z, 8);
  return k;
}

int main(void) {
  unsigned char f[512], conserto[70];
  long segIni = 0, n;
  MkvfixCab c;
  uint32_t id[MKVFIX_ENTRADAS]; int64_t pos[MKVFIX_ENTRADAS];

  // THE CASE (logs 21/24, Baka and Test [KH] S1-01): one entry, to a second
  // SeekHead at the end; 70-byte SeekHead + Void region.
  { uint32_t i1[] = { ID_SEEKHEAD }, p1[] = { 900000 };
    n = arquivo(f, i1, p1, 1, 70, &segIni); }
  ok(mkvfix_ler_inicio(f, n, &c) == 1, "hidden index: Matroska header read");
  ok(c.segIni == segIni && c.regiao == segIni, "hidden index: the region starts at the first SeekHead");
  ok(c.regiaoN == 70, "hidden index: the region is the SeekHead plus the Void after it (70 B)");
  ok(c.n == 1 && mkvfix_entrada(c.id, c.pos, c.n, ID_SEEKHEAD) == 900000, "hidden index: one entry, to the second SeekHead");
  ok(mkvfix_entrada(c.id, c.pos, c.n, ID_CUES) < 0, "hidden index: the first SeekHead does not list the Cues");

  // The rewrite: Cues first, then the path to the second SeekHead, in exactly
  // the region size, so no offset of the file moves.
  { uint32_t ids[] = { ID_CUES, ID_SEEKHEAD, ID_CUES };   // repeated id stays out
    int64_t ps[] = { 5000000000LL, 900000, 1 };
    int e = mkvfix_montar(ids, ps, 3, conserto, sizeof conserto);
    int k = mkvfix_ler_seekhead(conserto, sizeof conserto, id, pos, MKVFIX_ENTRADAS);
    ok(e == 2, "rewrite: Cues and the second SeekHead fit, the duplicate is dropped");
    ok(k == 2 && mkvfix_entrada(id, pos, k, ID_CUES) == 5000000000LL, "rewrite: the new SeekHead lists the Cues (past 4 GB)");
    ok(mkvfix_entrada(id, pos, k, ID_SEEKHEAD) == 900000, "rewrite: the entry to the second SeekHead is kept");
    memcpy(f + c.regiao, conserto, sizeof conserto);
    ok(mkvfix_ler_inicio(f, n, &c) == 1 && c.regiaoN == 70 && mkvfix_entrada(c.id, c.pos, c.n, ID_CUES) == 5000000000LL,
       "rewrite: the patched header reads as a normal one, same region size");
    ok(!memcmp(f + 70 + segIni, "\x15\x49\xA9\x66", 4), "rewrite: the element after the region did not move");
  }
  { uint32_t ids[] = { ID_CUES }; int64_t ps[] = { 1234 };
    ok(mkvfix_montar(ids, ps, 1, conserto, 8) == 0, "rewrite: no room -> 0 (left to the player)"); }

  // NORMAL FILE: the first SeekHead lists the Cues; nothing to fix.
  { uint32_t i1[] = { ID_INFO, ID_CUES }, p1[] = { 100, 777777 };
    n = arquivo(f, i1, p1, 2, 90, &segIni); }
  ok(mkvfix_ler_inicio(f, n, &c) == 1 && mkvfix_entrada(c.id, c.pos, c.n, ID_CUES) == 777777,
     "normal file: the first SeekHead lists the Cues");
  ok(mkvfix_ler_inicio((const unsigned char *)"RIFF....WAVEfmt ", 16, &c) == 0, "not Matroska: 0");

  printf(falhas ? "mkvfix: %d FAIL(S)\n" : "mkvfix: ok\n", falhas);
  return falhas != 0;
}
