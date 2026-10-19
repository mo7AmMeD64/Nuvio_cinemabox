// tests/ts_sonda.sh: sonda de MPEG-TS (#158).
#include "../src/ts_sonda.h"
#include <stdlib.h>
#include <assert.h>
static unsigned char buf[188 * 40];
static int np;
static unsigned char *pac(int pid, int pusi) {
  unsigned char *k = buf + 188 * np++;
  memset(k, 0xFF, 188);
  k[0] = 0x47; k[1] = (unsigned char)((pusi << 6) | (pid >> 8)); k[2] = pid & 0xFF; k[3] = 0x10;
  return k;
}
static int falhas;
#define OK(c, m) do { if (c) printf("ok  %s\n", m); else { printf("FALHOU %s\n", m); falhas++; } } while (0)
static void montar(int vtipo, unsigned char sps0, unsigned char sps1, int prof, int scr) {
  unsigned char *k;
  np = 0;
  k = pac(0, 1);  // PAT: programa 1 -> PMT 0x100
  { unsigned char s[] = {0, 0x00, 0xB0, 13, 0, 1, 0xC1, 0, 0, 0, 1, 0xE1, 0x00, 0, 0, 0, 0};
    memcpy(k + 4, s, sizeof s); }
  k = pac(0x100, 1); // PMT: video 0x101, audio 0x06+AC3 0x102
  { unsigned char s[] = {0, 0x02, 0xB0, 27, 0, 1, 0xC1, 0, 0, 0xE1, 0x01, 0xF0, 0,
                         (unsigned char)vtipo, 0xE1, 0x01, 0xF0, 0,
                         0x06, 0xE1, 0x02, 0xF0, 3, 0x6A, 1, 0,
                         0, 0, 0, 0};
    memcpy(k + 4, s, sizeof s); }
  k = pac(0x101, 1);
  if (scr) k[3] |= 0x80;
  { unsigned char s[] = {0, 0, 1, 0xE0, 0, 0, 0x80, 0x80, 5, 0x21, 0, 1, 0, 1,
                         0, 0, 0, 1, sps0, sps1, (unsigned char)prof, 40};
    memcpy(k + 4, s, sizeof s); }
  while (np < 10) pac(0x1FFF, 0);
}
int main(void) {
  TsSonda s; char r[400];
  montar(0x1B, 0x67, 110, 0, 0);
  ts_sondar(buf, 188L * np, &s); ts_resumo(&s, r, sizeof r); printf("    %s\n", r);
  OK(s.deslocamento == 0 && s.pmtPid == 0x100, "PAT -> PMT");
  OK(s.nEs == 2 && s.es[0].tipo == 0x1B && s.es[1].desc == 0x6A, "PMT: H.264 + AC3 por descritor");
  OK(s.perfil == 110 && s.nivel == 40 && strstr(r, "High 10"), "SPS H.264 High 10");
  montar(0x24, 0x42, 0x01, 0x01, 1);
  // HEVC SPS: 42 01 | 01 | 22(perfil 2) ...  monta a mao
  { unsigned char *k = buf + 188 * 2 + 4 + 14;
    unsigned char h[] = {0, 0, 1, 0x42, 0x01, 0x01, 0x02, 0x20, 0, 0, 0, 0x90, 0, 0, 0, 0, 0, 153};
    memcpy(k, h, sizeof h); }
  ts_sondar(buf, 188L * np, &s); ts_resumo(&s, r, sizeof r); printf("    %s\n", r);
  OK(s.hevc && s.perfil == 2 && s.nivel == 153 && strstr(r, "Main 10"), "SPS HEVC Main 10");
  OK(s.embaralhados == 1, "conta pacote embaralhado");
  { unsigned char lixo[2000]; memset(lixo, '#', sizeof lixo);
    ts_sondar(lixo, sizeof lixo, &s); ts_resumo(&s, r, sizeof r);
    OK(s.deslocamento < 0 && strstr(r, "nao e MPEG-TS"), "corpo que nao e TS"); }
  if (falhas) return 1;
  printf("ts_sonda: tudo ok\n");
  return 0;
}
