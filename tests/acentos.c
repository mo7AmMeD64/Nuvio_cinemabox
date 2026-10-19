// Cor de destaque (acentos-mockup.html, 03/10/2026): cada acento FIXO de
// TEMA_ACENTO (src/ajustes.c, lido do fonte) passa as travas medidas:
//   - a tinta da familia sobre o preenchimento a >= 4,5:1 (claro: #121316;
//     profundo: branco, e ai >= 4,6 com a folga do mockup);
//   - a familia gravada e a que corviva_tokens escolhe (a tinta que contrasta
//     mais) — e a mesma conta que vale para os dinamicos;
//   - a marca sobre a ilha #121316 a >= 7:1 e a pilula contra a ilha >= 3:1
//     nos claros (os profundos ficam abaixo de proposito: a marca e o par claro);
//   - marca, HDR e luz iguais aos de corviva_tokens (a 6/255: na borda do gamute o JS e o C cortam o croma em passos diferentes).
#include "../src/corviva.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

char *dados_ler(const char *n) { (void)n; return NULL; }
int dados_gravar_leve(const char *n, const char *c) { (void)n; (void)c; return 1; }

static int falhas;
static void ok(int c, const char *m) { printf("%s %s\n", c ? "ok   " : "FALHA", m); if (!c) falhas++; }
static void hex(unsigned h, float c[3]) { c[0] = ((h >> 16) & 255) / 255.0f; c[1] = ((h >> 8) & 255) / 255.0f; c[2] = (h & 255) / 255.0f; }
static int perto(const float a[3], const float b[3]) {
  return fabsf(a[0] - b[0]) <= 6.5f / 255 && fabsf(a[1] - b[1]) <= 6.5f / 255 && fabsf(a[2] - b[2]) <= 6.5f / 255;
}

int main(void) {
  static const float BR[3] = { 1, 1, 1 }, TI[3] = { 18 / 255.0f, 19 / 255.0f, 22 / 255.0f };
  FILE *f = fopen("src/ajustes.c", "r");
  char ln[512], m[256];
  int n = 0, claros = 0, dentro = 0;
  if (!f) { printf("FALHA sem src/ajustes.c\n"); return 1; }
  while (fgets(ln, sizeof ln, f)) {
    unsigned fi, ma, hd, lu;
    char fam, nome[64] = "";
    char *c;
    if (strstr(ln, "static const AcentoFixo TEMA_ACENTO[]")) { dentro = 1; continue; }
    if (!dentro) continue;
    if (strstr(ln, "};")) break;
    if (sscanf(ln, " { 0x%x, 0x%x, 0x%x, 0x%x, '%c' }", &fi, &ma, &hd, &lu, &fam) != 5) continue;
    if ((c = strstr(ln, "//"))) { snprintf(nome, sizeof nome, "%s", c + 2); nome[strcspn(nome, "\n")] = 0; }
    { float F[3], M[3], H[3], L[3], cp;
      CorvivaTokens t;
      hex(fi, F); hex(ma, M); hex(hd, H); hex(lu, L);
      corviva_tokens(F, &t);
      cp = corviva_contraste(F, fam == 'c' ? TI : BR);
      snprintf(m, sizeof m, "%s: tinta %s %.2f:1", nome, fam == 'c' ? "escura" : "branca", cp);
      ok(cp >= (fam == 'c' ? 4.5f : 4.58f), m);
      snprintf(m, sizeof m, "%s: familia = a tinta que contrasta mais", nome);
      ok((fam == 'p') == t.tintaBranca, m);
      snprintf(m, sizeof m, "%s: marca sobre a ilha %.1f:1 (>= 7)", nome, corviva_contraste(M, TI));
      ok(corviva_contraste(M, TI) >= 7.0f, m);
      if (fam == 'c') {
        snprintf(m, sizeof m, "%s: pilula contra a ilha %.1f:1 (>= 3)", nome, corviva_contraste(F, TI));
        ok(corviva_contraste(F, TI) >= 3.0f, m);
        claros++;
      }
      snprintf(m, sizeof m, "%s: marca/HDR/luz = corviva_tokens", nome);
      ok(perto(M, t.marca) && perto(H, t.hdr) && perto(L, t.luz), m);
      n++; }
  }
  fclose(f);
  snprintf(m, sizeof m, "18 acentos fixos (%d), 9 claros (%d)", n, claros);
  ok(n == 18 && claros == 9, m);
  if (falhas) { printf("%d falha(s)\n", falhas); return 1; }
  printf("tudo ok\n");
  return 0;
}
