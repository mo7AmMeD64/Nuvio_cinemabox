// Ver plrguia.h.
#include "plrguia.h"
#include "plrilha.h"
#include "parental.h"
#include "anim.h"
#include "idioma.h"
#include "text.h"
#include "gfx.h"
#include "layout.h"
#define NV_ESCALA_TELA   // desenha na tela virtual (escala.h)
#include "escala.h"
#include <stdio.h>
#include <string.h>

#define LIN_H    50.0f
#define TOPO_PAD  8.0f
#define BASE_PAD 14.0f
#define LARG    520.0f
#define LADO     28.0f

static float tgAtual;

// O corpo: o retangulo e o da ilha abaixo do cabecalho de 64; `a` ja vem com o
// alfa da forma e da entrada do corpo.
static void corpo(GfxRect c, float a, void *u) {
  int i, np = parental_n();
  (void)u;
  for (i = 0; i < np; i++) {
    float yl = c.y + TOPO_PAD + i * LIN_H;
    float ts = anim_clamp((tgAtual - 0.18f - i * 0.10f) / 0.30f, 0.0f, 1.0f);
    float ag = a * (1.0f - (1.0f - ts) * (1.0f - ts));
    const char *gv = parental_gravidade(i);
    int forte = gv && !strcmp(gv, "Severo");
    TxtLinha lr, lg;
    if (ag <= 0.004f) continue;
    if (i) gfx_cor((GfxRect){ c.x + LADO, yl, c.w - 2.0f * LADO, 1.0f }, 0.0f, 1, 1, 1, 0.07f * ag);
    lr = txt_linha_corta(TXT_G22M, parental_rotulo(i), 243, 242, 239, 255, c.w - 2.0f * LADO - 160.0f);
    lg = forte ? txt_linha(TXT_ILHA_SUB, gv, 240, 185, 74, 255)
               : txt_linha(TXT_ILHA_SUB, gv, 243, 242, 239, 140);
    txt_desenhar_alpha(lr, c.x + LADO, yl + LIN_H * 0.5f - (float)lr.h * 0.5f, ag);
    txt_desenhar_alpha(lg, c.x + c.w - LADO - (float)lg.w, yl + LIN_H * 0.5f - (float)lg.h * 0.5f, ag);
  }
}

void plrguia_pedir(float tg, float osd, const char *classificacao) {
  static char cab[64];
  PlrIlhaPedido pd;
  int np = parental_n();
  if (np < 1) return;
  tgAtual = tg;
  if (classificacao && classificacao[0]) snprintf(cab, sizeof cab, i18n("Guia parental \xc2\xb7 %s"), classificacao);
  else snprintf(cab, sizeof cab, "%s", i18n("Guia parental"));
  memset(&pd, 0, sizeof pd);
  pd.icone = "aj_shield"; pd.texto = cab; pd.semFim = 1;
  pd.w = LARG; pd.h = TOPO_PAD + np * LIN_H + BASE_PAD;
  pd.corpo = corpo; pd.baixa = 1; pd.voltaRelogio = 1;
  plrilha_pedir(&pd);
  // Sem o OSD, o veu de cima segura a ilha em cena clara (como o dos avisos).
  { float e = anim_clamp(tg / 0.30f, 0.0f, 1.0f);
    ESCALA_INI();
    gfx_veu_css((GfxRect){ 0, 0, NV_VTELA_W, 260.0f }, 1, 1.38f, 1.0f, 0.52f * e * (1.0f - osd));
    ESCALA_FIM(); }
}
