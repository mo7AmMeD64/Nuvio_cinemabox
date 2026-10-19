#include "ilha_voo.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
// Um voo: monotono, dentro da tela, sem salto entre quadros de 16 ms, e com a
// imagem na proporcao de origem ate o ultimo terco (cover forcado depois).
static void testar(GfxRect de, float asp, GfxRect alvo) {
  float anterior = -1.0f, w = 1e9f, h = 1e9f, f, cxA = 0, cyA = 0;
  for (unsigned ms = 0; ms <= 800; ms++) {
    float t = ilha_voo_fracao(ms);
    GfxRect r = ilha_voo_rect_de(de, asp, alvo, t, &f);
    float cx = r.x + r.w * .5f, cy = r.y + r.h * .5f;
    assert(t >= anterior && t >= 0 && t <= 1);
    assert(r.w <= w + .001f && r.h <= h + .001f && r.w >= alvo.w - .5f && r.h >= alvo.h - .001f);
    assert(r.x >= -.5f && r.y >= -.5f && r.x + r.w <= 1920.5f && r.y + r.h <= 1080.5f);
    if (ms == 0) assert(fabsf(r.x - de.x) < .5f && fabsf(r.y - de.y) < .5f &&
                        fabsf(r.w - de.w) < .5f && fabsf(r.h - de.h) < .5f);
    if (t < 0.6f) assert(fabsf(r.w / r.h - asp) < 0.01f);   // nao estica
    // 60 Hz: nenhum quadro anda mais de 1/8 da diagonal da tela.
    if (ms >= 16 && ms % 16 == 0) assert(hypotf(cx - cxA, cy - cyA) < 280.0f);
    if (ms % 16 == 0) { cxA = cx; cyA = cy; }
    if (ms >= NV_ILHA_VOO_MS) {
      assert(fabsf(r.x - alvo.x) < .001f && fabsf(r.y - alvo.y) < .001f);
      assert(fabsf(r.w - alvo.w) < .001f && fabsf(r.h - alvo.h) < .001f);
    }
    anterior = t; w = r.w; h = r.h;
  }
}
int main(void) {
  GfxRect cheia = {0, 0, 1920, 1080}, scope = {0, 138, 1920, 803};
  testar(cheia, 1920.0f / 1080.0f, (GfxRect){180, 44, 30, 44});
  testar(cheia, 1920.0f / 1080.0f, (GfxRect){1530, 44, 30, 44});
  testar(scope, 1920.0f / 803.0f, (GfxRect){180, 44, 30, 44});
  // Mola: parte parada, passa da metade cedo e chega com velocidade ~0.
  assert(ilha_voo_fracao(0) == 0.0f);
  assert(ilha_voo_fracao(16) < 0.02f);
  assert(ilha_voo_fracao(200) > 0.6f && ilha_voo_fracao(200) < 0.75f);
  assert(ilha_voo_fracao(NV_ILHA_VOO_MS - 16) > 0.985f);
  assert(ilha_voo_fracao(NV_ILHA_VOO_MS - 1) < 1 && ilha_voo_fracao(NV_ILHA_VOO_MS) == 1);
  // Pulso: sobe, nunca abaixo de 1, volta a 1.
  for (unsigned ms = 0; ms <= NV_ILHA_PULSO_MS + 10; ms++) {
    float s = ilha_voo_pulso(ms);
    assert(s >= 1.0f && s < 1.07f);
  }
  assert(ilha_voo_pulso(NV_ILHA_PULSO_MS) == 1.0f && ilha_voo_pulso(NV_ILHA_PULSO_MS / 2) > 1.03f);
  printf("ilha_voo: 3 voos, mola %u ms, proporcao mantida, sem salto, pulso ok\n", NV_ILHA_VOO_MS);
}
