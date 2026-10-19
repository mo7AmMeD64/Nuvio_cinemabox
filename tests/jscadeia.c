// js_cadeia: elemento de array JSON com \uXXXX. O caso medido: addon de canal
// em Python (json.dumps, ensure_ascii padrao) manda "genre":["Not\u00edcias"],
// e o guia lia "Notu00edcias" — barra comida, escape cru.
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/js.h"

static const char *meta =
  "{\"id\":\"c1\",\"name\":\"Canal \\u00c1gua\",\"description\":\"Jornalismo 24h \\ud83d\\udcfa\","
  "\"genre\":[\"Not\\u00edcias\",\"Outro\"],\"genres\":[\"\\u65b0\\u95fb\"]}";

int main(void) {
  char b[64];
  const char *g;

  g = js_array(meta, NULL, "genre");
  assert(g && js_cadeia(g, b, sizeof b));
  assert(!strcmp(b, "Not\xc3\xad" "cias"));

  g = js_array(meta, NULL, "genres");          // 3 bytes por letra
  assert(g && js_cadeia(g, b, sizeof b));
  assert(!strcmp(b, "\xe6\x96\xb0\xe9\x97\xbb"));

  // name e description ja vinham por js_texto; o mesmo decodificador agora
  assert(js_texto(meta, NULL, "name", b, sizeof b));
  assert(!strcmp(b, "Canal \xc3\x81gua"));
  assert(js_texto(meta, NULL, "description", b, sizeof b));
  assert(!strcmp(b, "Jornalismo 24h \xf0\x9f\x93\xba"));   // par de substitutos

  // escapes comuns e invalidos
  assert(js_cadeia("\"a\\\"b\\\\c\\/d\"", b, sizeof b) && !strcmp(b, "a\"b\\c/d"));
  assert(js_cadeia("\"x\\uZZ\"", b, sizeof b) && !strcmp(b, "x ZZ"));

  // corte no meio de um caractere de 2 bytes volta a fronteira
  assert(js_cadeia("\"abc\xc3\xad\"", b, 5) && !strcmp(b, "abc"));

  // nao e string
  b[0] = 'q';
  assert(!js_cadeia("123", b, sizeof b));
  assert(!js_cadeia(NULL, b, sizeof b));
  puts("jscadeia: ok");
  return 0;
}
