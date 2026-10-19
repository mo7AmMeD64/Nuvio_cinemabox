// Logo do app. Ver logoapp.h.
#include "logoapp.h"
#include "ajustes.h"
#include "iconeapp.h"
#include "tex_cache.h"
#include <stdio.h>

static char dirArteLg[512] = ".";
static char caminhos[LOGO_N][3][600];

void logoapp_iniciar(const char *dirArte) {
  int i, f;
  snprintf(dirArteLg, sizeof dirArteLg, "%s", dirArte && dirArte[0] ? dirArte : ".");
  for (i = 0; i < LOGO_N; i++) for (f = 0; f < 3; f++) caminhos[i][f][0] = 0;
}

int logoapp_atual(void) {
  int v = ajustes_logo_app();
  return v == LOGO_CLASSICO ? LOGO_CLASSICO : LOGO_NOVO;
}

const char *logoapp_nome(int logo) {
  return logo == LOGO_NOVO ? "Novo" : logo == LOGO_CLASSICO ? "Clássico" : "";
}

const char *logoapp_caminho(int logo, int forma) {
  static const char *ARQ[LOGO_N][3] = {
    { "logo-novo-simbolo.png", "logo-novo-marca.png", "logo-novo-horizontal.png" },
    { "logo-classico.png",     "abertura.jpg",        "nuvio_wordmark.png" },
  };
  if (logo < 0 || logo >= LOGO_N || forma < 0 || forma > 2) return "";
  if (!caminhos[logo][forma][0])
    snprintf(caminhos[logo][forma], sizeof caminhos[logo][forma], "%s/marcas/%s", dirArteLg, ARQ[logo][forma]);
  return caminhos[logo][forma];
}

int logoapp_miniatura(int logo, GfxRect r, int ladrilho, float alpha) {
  GLuint t;
  GfxRect s = r;
  float asp;
  const char *c;
  if (alpha <= 0.004f) return 0;
  if (ladrilho) gfx_cor(r, 0.22f, 13.0f / 255.0f, 16.0f / 255.0f, 30.0f / 255.0f, alpha);
  c = logoapp_caminho(logo, LOGO_F_SIMBOLO);
  t = tex_obter_larg(c, r.w);
  if (!t) return 0;
  // O simbolo nao e quadrado (o novo e mais alto, o classico mais largo): cabe no
  // ladrilho com ~14% de respiro, centrado.
  asp = tex_aspecto(c);
  if (asp <= 0.0f) asp = 1.0f;
  { float lado = r.w * 0.86f, w = lado, h = lado;
    if (asp >= 1.0f) h = w / asp; else w = h * asp;
    s = (GfxRect){ r.x + (r.w - w) * 0.5f, r.y + (r.h - h) * 0.5f, w, h }; }
  gfx_tex_aspect_atual = 0.0f;
  gfx_rect(s, t, GFX_TEXTO, 0, 0, 0, 0.0f, 1, 1, 1, alpha);
  return 1;
}

int logoapp_marca(GfxRect r, float alpha) {
  GLuint t;
  const char *c;
  float asp, w, h;
  if (iconeapp_marca(r, alpha)) return 1;       // icone de apoiador em vigor vence
  if (logoapp_atual() != LOGO_NOVO || alpha <= 0.004f) return 0;
  c = logoapp_caminho(LOGO_NOVO, LOGO_F_SIMBOLO);
  t = tex_obter_larg(c, r.w);
  if (!t) return 0;
  asp = tex_aspecto(c);
  if (asp <= 0.0f) asp = 0.91f;
  h = r.h; w = h * asp;
  if (w > r.w) { w = r.w; h = w / asp; }
  gfx_tex_aspect_atual = 0.0f;
  gfx_rect((GfxRect){ r.x + (r.w - w) * 0.5f, r.y + (r.h - h) * 0.5f, w, h },
           t, GFX_TEXTO, 0, 0, 0, 0.0f, 1, 1, 1, alpha);
  return 1;
}
