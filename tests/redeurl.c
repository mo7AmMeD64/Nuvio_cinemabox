#include "rede.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
  char dst[120], curto[14];
  assert(!strcmp(rede_url_publica("https://demo:fixture@example.test:443/private?key=fixture",
                                 dst, sizeof dst), "https://example.test:443/..."));
  rede_url_publica("https://demo:fixture@example.test/path", curto, sizeof curto);
  assert(!strstr(curto, "demo") && !strstr(curto, "fixture") && !strchr(curto, '@'));
  assert(!strcmp(rede_url_publica("http://example.test", dst, sizeof dst), "http://example.test"));
  assert(!strcmp(rede_url_publica("https://demo:fixture@[::1]:80/path", dst, sizeof dst),
                 "https://[::1]:80/..."));
  assert(!strcmp(rede_url_publica(NULL, dst, sizeof dst), ""));
  assert(!strcmp(rede_url_publica("https://example.test/path", dst, 1), ""));
  puts("redeurl: ok");
  return 0;
}
