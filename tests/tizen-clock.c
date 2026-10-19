#include "marco.h"
#include <stdio.h>

int main(void) {
  puts("antes de marco_iniciar");
  marco_iniciar();
  puts("depois de marco_iniciar");
  marco("relogio e arquivo ok");
  return 0;
}
