// MODO DO LOAD DA LIVE TV (#158), comum a todos os alvos: so o video.c da
// webOS le (video.h). Fica num arquivo proprio para os outros backends nao
// precisarem de coto.
#include "video.h"
static int modoPedido;
void video_definir_modo_live(int modo) { modoPedido = modo >= 0 && modo <= 2 ? modo : 0; }
int  video_modo_live_consumir(void) { int m = modoPedido; modoPedido = 0; return m; }
