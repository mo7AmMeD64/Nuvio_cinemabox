// ESCALA DO RETANGULO DE VIDEO: unidades de LAYOUT -> pixels da superficie.
//
// O app inteiro desenha em 1920x1080 (NV_TELA_W/NV_TELA_H) e o player pede o
// plano de video nessas unidades. So que, com "4K (experimental)" ligado — ou
// numa TV cujo compositor ja concede 3840x2160 (issue #176, LG G5, webOS 10.3.1;
// #145, "Disabling the experimental 4K interface fixes it") — o drawable e
// 3840x2160. Um retangulo 1920x1080 mandado cru ao plano de video passa a ser um
// QUARTO da tela: "the player would make the image 2 times smaller than the
// screen", metade da largura e metade da altura, com faixas em volta.
//
// O que e MEDIDO: no drawable 1920x1080 (C9, webOS 4.10) a escala e 1 e o
// caminho antigo funciona, entao nada muda ali. O que e HIPOTESE: que o destino
// do plano de video (ACB, janela exportada e uMS) seja lido no espaco da
// superficie do app/painel; e o que os relatos #176/#145 sugerem para a janela
// exportada, e nao foi medido nos outros dois caminhos. Eles so recebem escala
// quando o drawable sai de 1920x1080, que e onde nao ha medida nenhuma.
//
// Funcoes puras e sem SDL, para o teste compilar no Mac.
#ifndef NV_VIDEO_ESCALA_H
#define NV_VIDEO_ESCALA_H

typedef struct { int x, y, w, h; } NvRetInt;

// Escala um retangulo de layout (lw x lh) para uma superficie (sw x sh). As
// BORDAS sao arredondadas e a largura sai da diferenca, para que dois retangulos
// vizinhos nao deixem fresta e a tela cheia caia exatamente em 0,0,sw,sh.
// Superficie invalida (<=0) ou igual ao layout devolve o retangulo intacto.
static inline NvRetInt nv_video_escalar(NvRetInt r, int lw, int lh, int sw, int sh) {
  NvRetInt o = r;
  int x1, y1;
  if (lw < 1 || lh < 1 || sw < 1 || sh < 1) return r;
  if (sw == lw && sh == lh) return r;
  o.x = (int)(((long)r.x * sw * 2 + lw) / (2L * lw));
  o.y = (int)(((long)r.y * sh * 2 + lh) / (2L * lh));
  x1  = (int)(((long)(r.x + r.w) * sw * 2 + lw) / (2L * lw));
  y1  = (int)(((long)(r.y + r.h) * sh * 2 + lh) / (2L * lh));
  o.w = x1 - o.x; o.h = y1 - o.y;
  if (o.w < 1) o.w = 1;
  if (o.h < 1) o.h = 1;
  return o;
}

#endif
