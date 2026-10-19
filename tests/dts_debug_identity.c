#include "app_id.h"
#include "dts/dts_playback.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
static int adapter = 1, engine = 1;
int dts_engine_available(void) { return engine; }
int dts_pipeline_available(int major) { assert(major==0); return adapter; }
int main(void) {
#if defined(NV_WEBOS) && defined(NV_DTS_FFMPEG)
  const int supported = 1;
#else
  const int supported = 0;
#endif
  unsetenv("NUVIO_DTS_ENABLE");
#ifdef NV_DTS_DEBUG
  assert(strcmp(NV_APP_ID,NV_APP_ID_PRODUCTION));
  assert(strstr(NV_LS_MEDIA_CLIENT,NV_APP_ID));
#else
  assert(!strcmp(NV_APP_ID,"space.nuvio.native.legacy"));
  assert(!strcmp(NV_LS_MEDIA_CLIENT,"com.webos.media.client.nuvio"));
#endif
  assert(dts_playback_enabled()==supported);
  /* Launch-variable opt-in is no longer required, including on production. */
  setenv("NUVIO_DTS_ENABLE","0",1); assert(dts_playback_enabled()==supported);
  adapter=0; assert(!dts_playback_enabled());adapter=1;
  engine=0; assert(!dts_playback_enabled());
  return 0;
}
