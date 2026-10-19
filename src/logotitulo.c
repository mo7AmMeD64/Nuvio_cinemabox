// Ver logotitulo.h.
#include "logotitulo.h"
#include "artehero.h"
#include "gfx.h"
#include "tex_cache.h"

const char *logotitulo_url(const CatItem *ci, float maxW) {
  const char *u;
  if (!ci) return NULL;
  u = artehero_logo_sessao_larg(ci, maxW);
  return u && u[0] ? u : NULL;
}

int logotitulo_desenhar(const CatItem *ci, const char *nome, TxtEstilo estilo,
                        float x, float y, float maxW, float maxH,
                        float largTexto, float a) {
  const char *u = logotitulo_url(ci, maxW);
  // A textura menor que ja existe serve enquanto a do tamanho certo decodifica
  // (o mesmo do hero): abrir o menu em cima do destaque nao pisca o nome.
  GLuint t = u ? tex_obter_logo_larg_qualquer(u, maxW) : 0;
  float asp = t ? tex_aspecto(u) : 0.0f;
  if (t && asp > 0.01f) {
    float w = maxW, h = w / asp;
    GfxModo m;
    if (h > maxH) { h = maxH; w = h * asp; }
    m = tex_marca_escura(u) ? GFX_MARCA : GFX_TEXTO;
    gfx_tex_aspect_atual = 0.0f;
    gfx_rect((GfxRect){ x, y + (maxH - h) * 0.5f, w, h }, t, m,
             0, 0, 0, 0.0f, 1, 1, 1, a);
    return 1;
  }
  if (nome && nome[0]) {
    TxtLinha l = txt_linha_corta(estilo, nome, 245, 248, 255, 255,
                                 largTexto > 0.0f ? largTexto : maxW);
    txt_desenhar_alpha(l, x, y + (maxH - (float)l.h) * 0.5f, a);
  }
  return 0;
}
