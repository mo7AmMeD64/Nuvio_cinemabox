#include "js.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void truncados(void) {
  const char *casos[] = {"{\"\\", "{\"x\":\"\\u", "{\"x\":\"\\uD800",
                         "{\"x\":\"\\uD800\\u", "{\"x\":\"\\uD800\\uD", NULL};
  char dst[80];
  for (int i = 0; casos[i]; i++) {
    char *s = strdup(casos[i]);
    assert(js_fim(s) == s + strlen(s));
    js_texto(s, NULL, "x", dst, sizeof dst);
    js_texto_raiz(s, "x", dst, sizeof dst);
    free(s);
  }
}

static void faixas(void) {
  const char *s = "{\"x\":\"abcdef\",\"n\":12345,\"v\":{\"a\":1}}";
  const char *f;
  char dst[80];
  /* A faixa termina no meio do valor; o resto do documento nao pertence a ela. */
  f = strstr(s, "def");
  assert(!js_texto(s, f, "x", dst, sizeof dst));
  assert(!js_texto_raiz_em(s, f, "x", dst, sizeof dst));
  f = strstr(s, "345");
  assert(js_num(s, f, "n", -1) == 12);
  f = strstr(s, "1}}");
  assert(!js_bruto(s, f, "v", dst, sizeof dst));
  assert(!js_texto(s, NULL, "x", dst, 0));
  assert(!js_bruto(s, NULL, "v", dst, 0));
  assert(!js_texto(NULL, NULL, "x", dst, sizeof dst));
  assert(js_num(NULL, NULL, "n", -1) == -1);
  assert(js_fim(NULL) == NULL);
  /* Faixa sem NUL, tamanho exato de alocacao: ASan vigia qualquer leitura extra. */
  char *raw = malloc(8);
  memcpy(raw, "{\"x\":\"ab", 8);
  assert(!js_texto(raw, raw + 8, "x", dst, sizeof dst));
  assert(!js_texto_raiz_em(raw, raw + 8, "x", dst, sizeof dst));
  free(raw);
}

static void valores(void) {
  char dst[80];
  const char *s = "{\"literal\":\"\\\"x\\\":\\\"falso\\\"\",\"x\":\"certo\"}";
  assert(js_texto(s, NULL, "x", dst, sizeof dst));
  assert(!strcmp(dst, "certo"));
  assert(js_texto("{\"x\":\"Not\\u00edcias \\uD83D\\uDE00\"}", NULL,
                  "x", dst, sizeof dst));
  assert(!strcmp(dst, "Notícias 😀"));
  assert(js_num("{\"n\":1e999}", NULL, "n", -1) == -1);
  assert(js_tem("{\"n\":1e999}", NULL, "n"));
  assert(js_tem("{\"n\":null}", NULL, "n"));
  assert(!js_tem("{\"x\":null}", NULL, "n"));
  assert(!js_tem(NULL, NULL, "n"));
  assert(js_num("{\"n\":\"8.1\"}", NULL, "n", -1) == 8.1);
  assert(js_num("{\"n\":{\"n\":8580}}", NULL, "n", -1) == 8580);
  assert(js_bruto("{\"v\":true     }", NULL, "v", dst, 5));
  assert(!strcmp(dst, "true"));
  assert(js_texto_raiz("{\"nested\":{\"x\":\"errado\"},\"x\":\"certo\"}",
                       "x", dst, sizeof dst));
  assert(!strcmp(dst, "certo"));
}

static void prefixos(void) {
  const char *s = "{\"x\":\"Not\\u00edcias \\uD83D\\uDE00\",\"n\":12.5e2,\"v\":[{\"x\":\"a\"}]}";
  char dst[80];
  for (size_t n = 0; n <= strlen(s); n++) {
    char *raw = malloc(n ? n : 1);
    memcpy(raw, s, n);
    js_texto(raw, raw + n, "x", dst, sizeof dst);
    js_texto_raiz_em(raw, raw + n, "x", dst, sizeof dst);
    js_bruto(raw, raw + n, "v", dst, sizeof dst);
    js_num(raw, raw + n, "n", -1);
    js_array(raw, raw + n, "v");
    free(raw);
  }
}

static void padroes(void) {
  char dst[24] = "nome conhecido";
  assert(!js_texto("{}", NULL, "name", dst, sizeof dst));
  assert(!strcmp(dst, "nome conhecido"));
  assert(!js_texto("{\"name\":null}", NULL, "name", dst, sizeof dst));
  assert(!strcmp(dst, "nome conhecido"));
  assert(!js_texto("{\"name\":12}", NULL, "name", dst, sizeof dst));
  assert(!strcmp(dst, "nome conhecido"));
  assert(!js_texto("{\"name\":\"truncado", NULL, "name", dst, sizeof dst));
  assert(!strcmp(dst, "nome conhecido"));
  assert(!js_bruto("{}", NULL, "name", dst, sizeof dst));
  assert(!strcmp(dst, "nome conhecido"));
  assert(!js_bruto("{\"name\":{", NULL, "name", dst, sizeof dst));
  assert(!strcmp(dst, "nome conhecido"));
  assert(!js_bruto("{\"name\":\"longo\"}", NULL, "name", dst, 2));
  assert(!strcmp(dst, "nome conhecido"));
  /* O leitor de raiz ja possuia um contrato diferente: limpa a ausencia. */
  assert(!js_texto_raiz("{}", "name", dst, sizeof dst));
  assert(!dst[0]);
}

int main(void) {
  truncados();
  faixas();
  valores();
  prefixos();
  padroes();
  puts("js limites: ok");
  return 0;
}
