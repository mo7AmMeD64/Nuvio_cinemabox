// HISTERESE DA SAUDE DA REDE (redesaude.h): "sem internet" so com 3 falhas de
// transporte seguidas, de 2 hosts, em 3 s; "de volta" no primeiro sucesso; 10 s
// de carencia antes de declarar "sem" de novo. Sem rede de verdade: o nucleo
// puro com um relogio de mentira, e a porta de rede.c (rede_saude_nota) com
// os CURLcodes e as URLs que contam ou nao.
#include "redesaude.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
  RedeSaude s;
  unsigned t = 1000;
  memset(&s, 0, sizeof s);

  // 1. Um host so dando timeout (addon lento) NUNCA declara sem internet.
  assert(!rede_saude_passo(&s, 0, 11, t));
  assert(!rede_saude_passo(&s, 0, 11, t += 2000));
  assert(!rede_saude_passo(&s, 0, 11, t += 2000));
  assert(!rede_saude_passo(&s, 0, 11, t += 2000));
  assert(!s.offline);

  // 2. Um sucesso no meio zera a sequencia.
  assert(!rede_saude_passo(&s, 1, 22, t += 10));
  assert(!rede_saude_passo(&s, 0, 11, t += 10));
  assert(!rede_saude_passo(&s, 0, 22, t += 10));
  assert(!rede_saude_passo(&s, 1, 33, t += 10));
  assert(!rede_saude_passo(&s, 0, 11, t += 10));
  assert(!s.offline);

  // 3. Tres falhas, dois hosts, mas tudo em menos de 3 s: ainda nao (uma
  //    rajada de pedidos que cai junto numa troca de Wi-Fi nao pisca).
  memset(&s, 0, sizeof s);
  t = 50000;
  assert(!rede_saude_passo(&s, 0, 11, t));
  assert(!rede_saude_passo(&s, 0, 22, t += 500));
  assert(!rede_saude_passo(&s, 0, 11, t += 500));
  assert(!s.offline);
  //    ...e passada a janela, a proxima falha declara.
  assert(rede_saude_passo(&s, 0, 33, t += 2500));
  assert(s.offline && s.seq == 1);
  //    Mais falhas com ela ja fora nao trocam nada.
  assert(!rede_saude_passo(&s, 0, 44, t += 1000));
  assert(s.seq == 1);

  // 4. O primeiro sucesso e "de volta".
  assert(rede_saude_passo(&s, 1, 11, t += 5000));
  assert(!s.offline && s.seq == 2);

  // 5. CARENCIA: caiu de novo logo depois de voltar — dentro de 10 s, nada;
  //    depois dela, a sequencia que continuou declara.
  assert(!rede_saude_passo(&s, 0, 11, t += 100));
  assert(!rede_saude_passo(&s, 0, 22, t += 2000));
  assert(!rede_saude_passo(&s, 0, 11, t += 2000));
  assert(!rede_saude_passo(&s, 0, 22, t += 2000));
  assert(!s.offline);
  assert(rede_saude_passo(&s, 0, 33, t += 5000));
  assert(s.offline && s.seq == 3);

  // 6. Host 0 (endereco local, ignorado) nao mexe em nada.
  assert(!rede_saude_passo(&s, 1, 0, t += 10));
  assert(s.offline);

  // 7. A PORTA DE rede.c: CURLcode de transporte conta, cancelamento e teto
  //    nao, endereco local nao. Comeca online.
  assert(!rede_saude_offline());
  rede_saude_nota(42, "https://a.com/x");   // cancelado: nada
  rede_saude_nota(23, "https://a.com/x");   // teto: nada
  rede_saude_nota(7, "http://127.0.0.1:8080/x");
  rede_saude_nota(7, "http://192.168.1.20:11470/x");
  rede_saude_nota(6, "http://localhost/x");
  assert(!rede_saude_offline() && rede_saude_seq() == 0);
  puts("PASS: saude da rede (histerese, carencia, codigos, hosts locais).");
  return 0;
}
