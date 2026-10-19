// htmlq contra uma pagina: para cada seletor de um arquivo, imprime
// "seletor<TAB>quantidade<TAB>assinatura" — a mesma linha que o cheerio
// imprime no comparador (tests/htmlq.sh). Sem argumentos roda os casos fixos.
#include "htmlq.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static unsigned assinatura(const HqDoc *d, const int *v, int n) {
  unsigned h = 0; int i;
  for (i = 0; i < n; i++) {
    char *t = hq_texto(d, &v[i], 1);
    const char *href = hq_attr(d, v[i], "href");
    const char *tag = hq_tag(d, v[i]);
    size_t k, m = strlen(t);
    if (i) h = h * 31 + '\n';
    for (k = 0; tag[k]; k++) h = h * 31 + (unsigned char)tag[k];
    h = h * 31 + '|';
    if (m > 60) m = 60;
    for (k = 0; k < m; k++) h = h * 31 + (unsigned char)t[k];
    h = h * 31 + '|';
    for (k = 0; href && href[k]; k++) h = h * 31 + (unsigned char)href[k];
    free(t);
  }
  return h;
}

static char *lerArq(const char *c, long *n) {
  FILE *f = fopen(c, "rb"); char *b;
  if (!f) return NULL;
  fseek(f, 0, SEEK_END); *n = ftell(f); fseek(f, 0, SEEK_SET);
  b = malloc((size_t)*n + 1);
  if (fread(b, 1, (size_t)*n, f) != (size_t)*n) { fclose(f); free(b); return NULL; }
  b[*n] = 0; fclose(f); return b;
}

static int falhas;
#define CONFERE(c, m) do { if (!(c)) { printf("FALHOU: %s\n", m); falhas++; } } while (0)

static int conta(HqDoc *d, const char *s) { int *v, n = hq_selecionar(d, -1, s, &v); free(v); return n; }

static void casosFixos(void) {
  const char *h =
    "<!doctype html><html><head><title>A &amp; B</title><script>if (a<b) x='</div>';</script></head>"
    "<body><div id=main class='c1 c2'><p>um<p>dois <b>tres</b><ul><li>a<li>b<li><a href=\"/x?a=1&amp;b=2\">c</a></ul>"
    "<table><tr><td>1<td>2<tr><td>3</table><br/><img src=i.jpg alt=\"\"><svg><path d=1 /><circle/></svg>"
    "<span>Fim&nbsp;&#233;&#x41;</span><!-- <a>nao</a> --></div><div class=x><a>Inception 2010</a></div></body></html>";
  HqDoc *d = hq_carregar(h, strlen(h));
  int *v, n; char *t;
  CONFERE(conta(d, "p") == 2, "p implicito");
  CONFERE(conta(d, "li") == 3, "li implicito");
  CONFERE(conta(d, "td") == 3 && conta(d, "tr") == 2, "tabela implicita");
  CONFERE(conta(d, "a") == 2, "comentario nao vira no");
  CONFERE(conta(d, "#main.c1.c2") == 1, "id e classes");
  CONFERE(conta(d, "div > p") == 2, "filho");
  CONFERE(conta(d, "li:nth-child(3) a[href*='b=2']") == 1, "nth-child e attr decodificado");
  CONFERE(conta(d, "li:nth-child(odd)") == 2, "odd");
  CONFERE(conta(d, "svg > *") == 2, "auto-fechamento em svg");
  CONFERE(conta(d, "div:has(> a)") == 1, ":has relativo");
  CONFERE(conta(d, "div:has(a)") == 2, ":has descendente");
  CONFERE(conta(d, "a:contains('Inception')") == 1, ":contains");
  CONFERE(conta(d, "div:not(.x)") == 1, ":not");
  CONFERE(conta(d, "p + ul") == 1 && conta(d, "p ~ table") == 1, "irmaos");
  CONFERE(conta(d, "a[") == -1, "seletor invalido");
  CONFERE(conta(d, "img[alt]") == 1, "attr vazio existe");
  n = hq_selecionar(d, -1, "title", &v);
  t = hq_texto(d, v, n); CONFERE(!strcmp(t, "A & B"), "entidade no titulo"); free(t); free(v);
  n = hq_selecionar(d, -1, "script", &v);
  t = hq_html(d, v[0], 0); CONFERE(strstr(t, "'</div>'") != NULL, "script cru"); free(t); free(v);
  n = hq_selecionar(d, -1, "span", &v);
  t = hq_texto(d, v, n); CONFERE(!strcmp(t, "Fim \xc3\xa9" "A"), "nbsp vira espaco no texto"); free(t); free(v);
  n = hq_selecionar(d, -1, "p", &v);
  t = hq_texto(d, v, n); CONFERE(!strcmp(t, "um dois tres"), "texto jsoup"); free(t);
  { int *f, nf; nf = hq_selecionar(d, v[1], "b", &f); CONFERE(nf == 1, "find no contexto"); free(f);
    nf = hq_selecionar(d, v[1], "> b", &f); CONFERE(nf == 1, "find relativo"); free(f); }
  CONFERE(hq_prox(d, v[0]) == v[1], "proximo irmao");
  CONFERE(hq_pai(d, v[0]) >= 0 && !strcmp(hq_tag(d, hq_pai(d, v[0])), "div"), "pai");
  t = hq_html(d, v[1], 1); CONFERE(!strncmp(t, "<p>dois", 7), "html externo"); free(t);
  free(v);
  hq_soltar(d);
}

// F09: limites contra pagina/seletor hostil.
static void casosLimites(void) {
  char sel[4096], *html;
  int *v = NULL, k, i;
  HqDoc *d;
  size_t n = 200000;
  // teto de bytes: recusado antes de copiar a fonte
  html = malloc(n + 1);
  memset(html, 'a', n); html[n] = 0;
  CONFERE(hq_carregar_max(html, n, 100000) == NULL, "fonte maior que o teto e recusada");
  // muitos nos: o teto vale durante o crescimento
  for (i = 0; i + 3 <= (int)n; i += 3) memcpy(html + i, "<b>", 3);
  CONFERE(hq_carregar_max(html, n, 1024 * 1024) == NULL, "arvore que passa do teto falha");
  d = hq_carregar_max(html, n, 32u * 1024 * 1024);
  CONFERE(d && hq_memoria(d) <= 32u * 1024 * 1024, "dentro do teto carrega");
  // aninhamento de :not acima do limite e seletor invalido (lista vazia)
  strcpy(sel, "b");
  for (i = 0; i < 20; i++) { char t[4096]; snprintf(t, sizeof t, ":not(%s)", sel); snprintf(sel, sizeof sel, "%s", t); }
  CONFERE(hq_selecionar(d, -1, sel, &v) == -1, "aninhamento profundo recusado"); free(v); v = NULL;
  // seletor patologico (descendentes em arvore funda) para no teto de trabalho
  sel[0] = 0;
  for (i = 0; i < 20; i++) strcat(sel, "b ");
  strcat(sel, "i");
  { struct timespec a, b; double ms;
    clock_gettime(CLOCK_MONOTONIC, &a);
    k = hq_selecionar(d, -1, sel, &v);
    clock_gettime(CLOCK_MONOTONIC, &b);
    ms = (b.tv_sec - a.tv_sec) * 1e3 + (b.tv_nsec - a.tv_nsec) / 1e6;
    CONFERE(k <= 0, "seletor patologico sem resultado");
    CONFERE(ms < 5000, "seletor patologico termina"); }
  free(v);
  hq_soltar(d);
  free(html);
}

int main(int argc, char **argv) {
  long n; char *html, *sels, *s;
  HqDoc *d;
  struct timespec a, b;
  if (argc < 3) { casosFixos(); casosLimites(); printf(falhas ? "htmlq: %d falha(s)\n" : "htmlq: ok\n", falhas); return falhas != 0; }
  html = lerArq(argv[1], &n);
  sels = lerArq(argv[2], &n);
  if (!html || !sels) return 2;
  clock_gettime(CLOCK_MONOTONIC, &a);
  d = hq_carregar(html, strlen(html));
  clock_gettime(CLOCK_MONOTONIC, &b);
  fprintf(stderr, "htmlq load ms %.1f mem KB %zu nos %d (html %zu KB)\n",
          (b.tv_sec - a.tv_sec) * 1e3 + (b.tv_nsec - a.tv_nsec) / 1e6, hq_memoria(d) / 1024, hq_nos(d), strlen(html) / 1024);
  for (s = strtok(sels, "\n"); s; s = strtok(NULL, "\n")) {
    int *v, k = hq_selecionar(d, -1, s, &v);
    printf("%s\t%d", s, k);
    if (k >= 0) printf("\t%u", assinatura(d, v, k));
    printf("\n");
    free(v);
  }
  hq_soltar(d);
  return 0;
}
