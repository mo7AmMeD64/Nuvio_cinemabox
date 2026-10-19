// Portao do texto da pagina de titulo (src/textogate.h): funcao pura, sem GL.
//   cc tests/textogate.c -Isrc -I/opt/homebrew/include/SDL2 -o /tmp/textogate && /tmp/textogate
#include "textogate.h"
#include <assert.h>
#include <stdio.h>
#include <math.h>

int main(void) {
  TextoGate g; textogate_reiniciar(&g);

  // Linhas pendentes: fica escondido, quadro a quadro.
  assert(textogate_passo(&g, 5, 1000) == 0.0f);
  assert(textogate_passo(&g, 2, 1016) == 0.0f);
  assert(!textogate_aberto(&g));

  // Tudo pronto: revela, e o esvanecimento e UM so, de 0 a 1 em ~180 ms.
  assert(textogate_passo(&g, 0, 1032) == 0.0f);   // primeiro quadro pronto: 0
  assert(textogate_aberto(&g));
  float a1 = textogate_passo(&g, 0, 1032 + 90);
  float a2 = textogate_passo(&g, 0, 1032 + 179);
  assert(a1 > 0.5f && a1 < 1.0f);
  assert(a2 >= a1 && a2 < 1.0f);
  assert(textogate_passo(&g, 0, 1032 + 181) == 1.0f);
  // Depois de aberto, pendentes NAO escondem de novo.
  assert(textogate_passo(&g, 9, 5000) == 1.0f);

  // Teto: 400 ms de pendencia teimosa revela mesmo assim.
  textogate_reiniciar(&g);
  assert(textogate_passo(&g, 3, 2000) == 0.0f);
  assert(textogate_passo(&g, 3, 2399) == 0.0f);
  assert(!textogate_aberto(&g));
  textogate_passo(&g, 3, 2400);
  assert(textogate_aberto(&g));
  assert(textogate_passo(&g, 3, 2400 + 200) == 1.0f);

  // Sem pendencia desde o primeiro quadro (tudo em cache): revela ja.
  textogate_reiniciar(&g);
  textogate_passo(&g, 0, 3000);
  assert(textogate_aberto(&g));

  // Animacoes reduzidas: sem esvanecimento, opacidade cheia no mesmo quadro.
  textogate_reiniciar(&g);
  anim_politica_reduzida = 1;
  assert(textogate_passo(&g, 0, 4000) == 1.0f);
  anim_politica_reduzida = 0;

  // Relogio dando a volta (Uint32) nao trava nem adianta o portao.
  textogate_reiniciar(&g);
  assert(textogate_passo(&g, 4, 0xFFFFFF00u) == 0.0f);
  assert(textogate_passo(&g, 4, 0xFFFFFF00u + 100) == 0.0f);
  assert(!textogate_aberto(&g));
  puts("textogate ok");
  return 0;
}
