// Provenance for fit is stricter than the legacy human-readable size.
#include "streams.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int checks;
static void parse(const char *fields,uint64_t expected) {
  char json[4096];Stream *v=NULL;
  snprintf(json,sizeof json,"{\"streams\":[{\"url\":\"https://media.invalid/a\",%s}]}",fields);
  assert(stream_extrair(json,"Fixture",&v)==1);assert(v[0].tamanhoBytes==expected);
  if(expected==UINT64_C(1073741824)) assert(v[0].tamanhoMB==1024);
  free(v);checks++;
}
int main(void) {
  parse("\"behaviorHints\":{\"videoSize\":1073741824}",UINT64_C(1073741824));
  parse("\"behaviorHints\":{\"videoSize\":\"1073741824\"}",UINT64_C(1073741824));
  parse("\"behaviorHints\":{\"videoSize\": 1073741824 \n}",UINT64_C(1073741824));
  parse("\"videoSize\":1073741824",0);
  parse("\"title\":\"4K 50 GB\"",0);
  parse("\"torrentSize\":107374182400,\"title\":\"Season pack 100 GB\"",0);
  parse("\"behaviorHints\":{\"proxyHeaders\":{\"request\":{\"videoSize\":1073741824}}}",0);
  parse("\"behaviorHints\":{\"proxyHeaders\":{\"request\":{\"videoSize\":1}},\"videoSize\":1073741824}",UINT64_C(1073741824));
  parse("\"other\":{\"behaviorHints\":{\"videoSize\":1073741824}}",0);
  parse("\"behaviorHints\":\"videoSize:1073741824\"",0);
  parse("\"behaviorHints\":null",0);
  parse("\"behaviorHints\":[]",0);
  const char *invalid[]={"-1","0","1.2","1e99","1e9","1073741824.0","null","false","true","[]","{}","\"NaN\"","\"1.2\"","\"1e9\"","\"\"","\"-1\"","9007199254740992","18446744073709551615","999999999999999999999999999999"};
  for(unsigned i=0;i<sizeof invalid/sizeof invalid[0];i++) {
    char f[512];snprintf(f,sizeof f,"\"behaviorHints\":{\"videoSize\":%s}",invalid[i]);parse(f,0);
  }
  parse("\"behaviorHints\":{\"videoSize\":9007199254740991}",STREAMFIT_BYTES_MAX);
  Stream *v=NULL;
  assert(stream_extrair("{\"streams\":["
    "{\"url\":\"https://a.invalid/a\"},"
    "{\"url\":\"https://b.invalid/b\",\"behaviorHints\":{\"videoSize\":1073741824}},"
    "{\"url\":\"https://c.invalid/c\",\"title\":\"50 GB\"}]}","Fixture",&v)==3);
  assert(!v[0].tamanhoBytes && v[1].tamanhoBytes==UINT64_C(1073741824) && !v[2].tamanhoBytes);
  assert(v[2].tamanhoMB==51200);free(v);checks++;
  printf("streamfit parser: PASS (%d exact-size/provenance cases)\n",checks);return 0;
}
