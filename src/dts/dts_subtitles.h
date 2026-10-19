/* Worker-only subtitle decoding boundary; no FFmpeg types cross this API. */
#ifndef NV_DTS_SUBTITLES_H
#define NV_DTS_SUBTITLES_H
#include "dts_media.h"
typedef struct DtsSubtitles DtsSubtitles;
typedef struct {
  char text[2048];
  /* Straight RGBA, tightly packed. Borrowed until next decode/flush/destroy. */
  uint8_t *rgba;
  int x, y, w, h, canvas_w, canvas_h;
  int64_t start_ns, end_ns;
} DtsSubtitle;
/* codec_parameters is a borrowed AVCodecParameters, copied during creation. */
DtsSubtitles *dts_subtitles_create(const void *codec_parameters);
/* 1 cue (including an empty clearing cue), 0 no cue, -1 invalid/failed.
 * Timing is absolute content time. An unspecified end is INT64_MAX; the next
 * cue replaces it. ASS is a plain-text fallback, without style/positioning. */
int dts_subtitles_decode(DtsSubtitles *, const DtsFrame *, DtsSubtitle *out);
void dts_subtitles_flush(DtsSubtitles *);
void dts_subtitles_destroy(DtsSubtitles *);
#endif
