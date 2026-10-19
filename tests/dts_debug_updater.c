#include "../src/atualizacao.c"
#include <assert.h>
int main(void) {
  assert(!mtx);
  atualizacao_verificar(); atualizacao_procurar_agora();
  assert(!mtx && !emCurso && !manualPendente && !tagNova[0]);
  strcpy(ipkUrl,"https://example.invalid/production.ipk");
  assert(!podeInstalar()); assert(fioInstalar(NULL)==0);
  return 0;
}
