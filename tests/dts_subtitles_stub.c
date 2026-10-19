#include "dts/dts_subtitles.h"
#include <assert.h>
int main(void) {
  DtsSubtitle out;
  assert(!dts_subtitles_create(NULL));
  assert(dts_subtitles_decode(NULL,NULL,&out)==-1 && !out.rgba && !out.text[0]);
  dts_subtitles_flush(NULL); dts_subtitles_destroy(NULL);
  return 0;
}
