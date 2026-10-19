#ifndef NV_DTS_OVERLAY_H
#define NV_DTS_OVERLAY_H
#include "dts_playback.h"
/* Call only on the GL thread, including with NULL to retire the texture. */
int dts_overlay_draw(DtsPlayback *, double seconds, float x, float y,
                     float w, float h, int video_w, int video_h, float alpha);
#endif
