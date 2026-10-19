// Leitores EBML e lacing isolados: nao cria fios nem acessa rede/disco.
// Incluir o modulo permite testar os leitores privados com o mesmo codigo.
#include "../src/mkvass.c"
#include <assert.h>

static int falhas;
static void conferir(int ok, const char *caso) {
  printf("%s: %s\n", ok ? "ok" : "FALHA", caso);
  if (!ok) falhas++;
}

int main(void) {
  long *tam = NULL, cab = 0;
  int nf = 0, ok;
  // 3 frames de 1 byte. O delta zero e representado por um vint de 8 bytes:
  // 2^(7*8-1)-1, largura permitida pelo formato EBML.
  const unsigned char iguais[] = {
    2, 0x81, 1, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 'a', 'b', 'c'
  };
  ok = tamanhosLace(iguais, sizeof iguais, 0x06, &tam, &nf, &cab);
  conferir(ok && nf == 3 && cab == 10 && tam[0] == 1 && tam[1] == 1 && tam[2] == 1,
           "EBML: delta zero de 8 bytes preserva os tres frames");
  free(tam); tam = NULL;

  const unsigned char decrescente[] = { 2, 0x82, 0xbe, 'a', 'b', 'c', 'd', 'e' };
  ok = tamanhosLace(decrescente, sizeof decrescente, 0x06, &tam, &nf, &cab);
  conferir(ok && nf == 3 && cab == 3 && tam[0] == 2 && tam[1] == 1 && tam[2] == 2,
           "EBML: delta negativo preserva os tamanhos de cada frame");
  free(tam); tam = NULL;

  // Declaracoes enormes cabem no vint, mas nao no bloco recebido. Somar
  // todas antes de validar contra o payload estourava long com poucos frames.
  unsigned char enorme[1 + 255 * 7];
  enorme[0] = 255;
  for (size_t i = 1; i < sizeof enorme; i += 7) {
    enorme[i] = 3;
    memset(enorme + i + 1, 0xff, 6);
  }
  conferir(!tamanhosLace(enorme, sizeof enorme, 0x06, &tam, &nf, &cab),
           "EBML: tamanhos maiores que o payload sao descartados sem overflow");
  free(tam); tam = NULL;

  const unsigned char truncado[] = { 2, 0x81, 1, 0x7f };
  conferir(!tamanhosLace(truncado, sizeof truncado, 0x06, &tam, &nf, &cab),
           "EBML: delta truncado e descartado");
  free(tam); tam = NULL;

  const unsigned char xiph[] = { 2, 1, 1, 'a', 'b', 'c' };
  ok = tamanhosLace(xiph, sizeof xiph, 0x02, &tam, &nf, &cab);
  conferir(ok && nf == 3 && cab == 3 && tam[0] == 1 && tam[1] == 1 && tam[2] == 1,
           "Xiph: tres frames continuam inteiros");
  free(tam); tam = NULL;

  const unsigned char fixo[] = { 2, 'a', 'b', 'c' };
  ok = tamanhosLace(fixo, sizeof fixo, 0x04, &tam, &nf, &cab);
  conferir(ok && nf == 3 && cab == 1 && tam[0] == 1 && tam[1] == 1 && tam[2] == 1,
           "fixed-size: tres frames continuam inteiros");
  free(tam);

  const unsigned char filhoEnorme[] = { 0x81, 1, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
  Iter filho = { filhoEnorme, sizeof filhoEnorme, 0 };
  unsigned long id; const unsigned char *dados; long n;
  conferir(!proximo(&filho, &id, &dados, &n), "EBML: filho alem do buffer e descartado");
  return falhas ? 1 : 0;
}
