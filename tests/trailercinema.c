// Modo cinema do trailer (trailercinema.h): borda de subida, tecla, fim do
// trailer, animacoes reduzidas e as medidas do logo.
#include "trailercinema.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>

static void anda(TrailerCinema *c, int toca, int quadros, int red) {
  for (int i = 0; i < quadros; i++) trailercinema_passo(c, toca, 1.0f / 60.0f, red);
}

int main(void) {
  TrailerCinema c; trailercinema_zerar(&c);

  // Sem trailer tocando nada acontece.
  anda(&c, 0, 30, 0);
  assert(c.v == 0.0f && !c.oculta);

  // Borda de subida: comeca a esconder, e a mola assenta perto de 1.
  anda(&c, 1, 1, 0);
  assert(c.oculta && c.v > 0.0f && c.v < 1.0f);
  anda(&c, 1, 120, 0);
  assert(c.v > 0.99f && trailercinema_t(&c) > 0.99f);

  // Uma tecla devolve a UI COM o trailer ainda tocando, e nao rearma sozinha.
  trailercinema_tecla(&c);
  anda(&c, 1, 120, 0);
  assert(!c.oculta && c.v < 0.01f);

  // Trailer para e volta a tocar (outro titulo): nova borda, esconde de novo.
  anda(&c, 0, 5, 0);
  anda(&c, 1, 120, 0);
  assert(c.oculta && c.v > 0.99f);

  // Acabou o trailer: volta sozinho e a intencao cai.
  anda(&c, 0, 1, 0);
  assert(!c.oculta);
  anda(&c, 0, 120, 0);
  assert(c.v < 0.01f);

  // Animacoes reduzidas: salta, sem mola.
  trailercinema_zerar(&c);
  anda(&c, 1, 1, 1);
  assert(c.v == 1.0f);
  anda(&c, 0, 1, 1);
  assert(c.v == 0.0f);

  // A curva e a suave do detalhe: 0, 0.5 e 1 nos pontos conhecidos.
  c.v = 0.5f; assert(fabsf(trailercinema_t(&c) - 0.5f) < 1e-6f);

  // Logo pequeno: 0,62 da altura de 200, e no maximo metade de 1000 de largura.
  { float w, h;
    trailercinema_logo(4.0f, &w, &h);
    assert(fabsf(h - 124.0f) < 1e-3f && fabsf(w - 496.0f) < 1e-3f);
    trailercinema_logo(10.0f, &w, &h);      // larguissimo: trava em 500
    assert(fabsf(w - 500.0f) < 1e-3f && fabsf(h - 50.0f) < 1e-3f);
    trailercinema_logo(0.0f, &w, &h);       // sem proporcao: 2,5
    assert(fabsf(w / h - 2.5f) < 1e-4f); }
  assert(fabsf(trailercinema_base() - (NV_TELA_H - 96.0f)) < 1e-3f);

  puts("trailercinema ok");
  return 0;
}
