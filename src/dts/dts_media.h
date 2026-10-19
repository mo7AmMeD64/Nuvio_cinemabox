/* C-only boundary shared by the FFmpeg worker and generation-specific LG ABI.
 * Time is absolute content time, in signed nanoseconds; no URL crosses it. */
#ifndef NV_DTS_MEDIA_H
#define NV_DTS_MEDIA_H
#include <stddef.h>
#include <stdint.h>
#define DTS_TRACK_MAX 64
enum { DTS_VIDEO = 1, DTS_AUDIO = 2, DTS_SUBTITLE = 3 };
typedef struct {
  int stream_index, stream_id, kind, channels, forced;
  char codec[24], language[16], title[64];
} DtsTrack;
typedef struct {
  char video_codec[16], audio_codec[16], hdr[24];
  int width, height, fps_num, fps_den, channels, sample_rate;
  int color_primaries, color_transfer, color_matrix, dovi_profile;
  int dovi_level, dovi_rpu_present, dovi_el_present, dovi_bl_present;
  int dovi_bl_compatibility_id;
  int64_t duration_ns, start_ns;
  int audio_stream, video_stream;
  DtsTrack tracks[DTS_TRACK_MAX];
  int n_tracks;
} DtsMediaInfo;
typedef struct {
  int kind, stream_index;
  const unsigned char *data;
  size_t size;
  int64_t pts_ns, dts_ns, duration_ns;
} DtsFrame;
#endif
