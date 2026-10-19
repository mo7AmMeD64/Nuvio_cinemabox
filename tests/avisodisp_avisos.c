// A central de avisos com o arquivo de dispensados (avisodisp.h): dispensar
// tira da lista, o mesmo evento nao volta quando a origem o repoe (o
// arranque seguinte), um evento novo volta, "Depois" so pula a estreia nesta
// sessao, e a ultima linha da lista e "Dispensar todos". Os itens vem do modo
// de demonstracao (NUVIO_AVISOS_DEMO=1), sem rede. Ver tests/avisodisp.sh.
#include "avisos.h"
#include "avisodisp.h"
#include "dados.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
  const char *d = getenv("NUVIO_DADOS");
  char id[72], imdb[64], tit[160];
  assert(d && strstr(d, "nuvio-avisodisp"));
  dados_iniciar(d);

  // Arranque: os cinco da demonstracao, mais a linha "Dispensar todos".
  avisos_iniciar();
  assert(avisos_lista_n() == 5 && avisos_lista_linhas() == 6);
  assert(avisos_lista_item(1, id, sizeof id, tit, sizeof tit, imdb, sizeof imdb));
  assert(!strcmp(id, "demo:agenda") && !imdb[0]);   // sem lembrete ligado: sem "Remover lembrete"

  // Dispensar um: sai da lista.
  avisos_dispensar("demo:canal");
  assert(avisos_lista_n() == 4);

  // Reinicio: a RAM esquece, a origem repoe os mesmos ids — o dispensado nao volta.
  avisos_encerrar();
  avisodisp_esquecer();
  avisos_iniciar();
  assert(avisos_lista_n() == 4);

  // Evento novo (outra chave) avisa normalmente.
  avisos_modo_seguro("seguro:teste-2", "Ajuste desfeito", "texto");
  assert(avisos_lista_n() == 5);

  // "Depois" na estreia: so nesta sessao, e o item continua na lista.
  assert(avisos_estreia_pendente(id, sizeof id, imdb, sizeof imdb) && !strcmp(id, "demo:agenda"));
  avisodisp_sessao_por("demo:agenda");
  assert(!avisos_estreia_pendente(id, sizeof id, imdb, sizeof imdb));
  assert(avisos_lista_n() == 5 && !avisodisp_tem("demo:agenda"));

  // Dispensar pela linha (o menu do painel) e "Dispensar todos" (a ultima linha).
  assert(avisos_lista_dispensar(0) && avisos_lista_n() == 4);
  assert(avisos_lista_ok(avisos_lista_n()) == 0);
  assert(avisos_lista_n() == 0 && avisos_lista_linhas() == 0);

  // E nada volta no arranque seguinte.
  avisos_encerrar();
  avisodisp_esquecer();
  avisos_iniciar();
  assert(avisos_lista_n() == 0);
  avisos_encerrar();
  printf("avisodisp (central): ok\n");
  return 0;
}
