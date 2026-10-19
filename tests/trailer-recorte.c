// Recorte da fonte do trailer no plano de video (trailer_recorte, trailer.h).
// O plano estica a fonte no destino; sem o "cover" o banner 1920x528 do
// layout Padrao achatava o trailer pela metade (log da C9, 01/10/2026:
// "fonte 244,138 1432x804 -> destino 0,0 1920x528").
#include "trailer.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>

static float asp(int w, int h) { return (float)w / (float)h; }

int main(void) {
  int sx, sy, sw, sh;
  // Destino 16:9 (tela cheia, Dinamica, Moderna cheia): o de sempre.
  trailer_recorte(1920, 1080, 1.0f, 1920, 1080, &sx, &sy, &sw, &sh);
  assert(sx == 0 && sy == 0 && sw == 1920 && sh == 1080);
  trailer_recorte(1920, 1080, 1.34f, 1920, 1080, &sx, &sy, &sw, &sh);
  assert(sw == 1432 && sh == 804 && sx == 244 && sy == 138);   // o numero do log
  // Apple matted em destino 16:9: so as laterais.
  trailer_recorte(1920, 804, 1.34f, 1920, 1080, &sx, &sy, &sw, &sh);
  assert(sh == 804 && sw == 1428 && sy == 0);

  // O BANNER DO PADRAO, 1920x528: a fonte sai na proporcao dele.
  trailer_recorte(1920, 1080, 1.34f, 1920, 528, &sx, &sy, &sw, &sh);
  // A largura volta ao quadro inteiro: 528 de altura ja fica dentro da
  // imagem (a tarja do IMDb ocupa ~138 px em cima e embaixo).
  assert(sw == 1920 && sx == 0 && sh == 528 && sy == 276);
  assert(fabsf(asp(sw, sh) - 1920.0f / 528.0f) < 0.02f);
  printf("padrao zoom: fonte %d,%d %dx%d -> 1920x528\n", sx, sy, sw, sh);
  // "Original" no banner: sem zoom, mas tambem sem achatar.
  trailer_recorte(1920, 1080, 1.0f, 1920, 528, &sx, &sy, &sw, &sh);
  assert(sw == 1920 && fabsf(asp(sw, sh) - 1920.0f / 528.0f) < 0.02f);
  // Moderna em faixa, 1421x670: alarga ate 1920 e corta altura, que fica
  // ainda dentro dos 804 sem tarja.
  trailer_recorte(1920, 1080, 1.34f, 1421, 670, &sx, &sy, &sw, &sh);
  assert(fabsf(asp(sw, sh) - 1421.0f / 670.0f) < 0.02f && sh <= 804);
  // Apple matted no banner: o quadro inteiro na largura.
  trailer_recorte(1920, 804, 1.34f, 1920, 528, &sx, &sy, &sw, &sh);
  assert(sw == 1920 && sh == 528 && sy == 138);
  // Tudo par e dentro do quadro.
  assert(!(sx & 1) && !(sy & 1) && !(sw & 1) && !(sh & 1));
  assert(sx >= 0 && sy >= 0 && sx + sw <= 1920 && sy + sh <= 1080);
  // Destino mais estreito que a fonte (retrato): corta as laterais.
  trailer_recorte(1920, 1080, 1.0f, 600, 900, &sx, &sy, &sw, &sh);
  assert(sh == 1080 && fabsf(asp(sw, sh) - 600.0f / 900.0f) < 0.02f && sx > 0);
  // Destino vazio nao divide por zero e devolve o zoom puro.
  trailer_recorte(1920, 1080, 1.0f, 0, 0, &sx, &sy, &sw, &sh);
  assert(sw == 1920 && sh == 1080);
  printf("trailer-recorte: ok\n");
  return 0;
}
