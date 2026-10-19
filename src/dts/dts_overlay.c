#include "dts_overlay.h"
#include "../gfx.h"
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

int dts_overlay_draw(DtsPlayback *p, double seconds, float x, float y,
                     float w, float h, int video_w, int video_h, float alpha) {
  static DtsPlayback *owner;
  static GLuint texture;
  static unsigned revision;
  static DtsSubtitle bitmap;
  DtsSubtitle next;
  unsigned changed;
  if (owner != p || !p || !isfinite(seconds) || seconds < 0) {
    if (texture) glDeleteTextures(1, &texture);
    texture = 0; revision = 0; owner = p;
  }
  if (!p || !isfinite(seconds) || seconds < 0) return 0;
  if (dts_playback_subtitle_bitmap(p, seconds, revision, &next, &changed)) {
    revision = changed;
    if (texture) glDeleteTextures(1, &texture);
    texture = 0;
    if (next.rgba) {
      glGenTextures(1, &texture);
      glBindTexture(GL_TEXTURE_2D, texture);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, next.w, next.h, 0,
                   GL_RGBA, GL_UNSIGNED_BYTE, next.rgba);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      free(next.rgba); next.rgba = NULL;
      bitmap = next;
    }
    gfx_tex_esquecer(0);
  }
  if (!texture) return 0;
  int cw = bitmap.canvas_w > 0 ? bitmap.canvas_w : video_w;
  int ch = bitmap.canvas_h > 0 ? bitmap.canvas_h : video_h;
  if (cw <= 0 || ch <= 0) return 0;
  GfxRect rect = {x + w * bitmap.x / cw, y + h * bitmap.y / ch,
                  w * bitmap.w / cw, h * bitmap.h / ch};
  gfx_rect(rect, texture, GFX_TEXTO, 0, 0, 0, 0, 1, 1, 1, alpha);
  return 1;
}
