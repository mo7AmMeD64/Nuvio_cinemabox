// libnvprobe.so — carga minima para o spike de UEP no Tizen 4/5.
//
// NAO tem segredo nenhum: so um retorno fixo e um teste de pthread. O objetivo
// unico e ter uma .so PROPRIA, NAO assinada por autor confiavel, com um
// segmento PROT_EXEC — exatamente o que a UEP (Unauthorized Execution
// Prevention) da Samsung recusa quando o arquivo esta em disco fora da base
// de assinaturas (SFD). O tamanho nao importa para a UEP: ela decide pela
// origem/assinatura do inode, nao pelo conteudo. Por isso um .so de 3 simbolos
// testa o mesmo portao que a libnuvio.so inteira, sem carregar chave alguma.
//
// A hipotese do spike (pesquisa califio em TV KantS2/Tizen, ARMv7, mesma
// geracao 2018-2019 das TVs dos testadores): a UEP bloqueia mmap PROT_EXEC de
// ARQUIVO nao assinado, mas NAO bloqueia execucao a partir de memoria anonima
// / memfd (senao o proprio JIT do .NET nao rodaria). Entao carregar esta .so
// por memfd_create + dlopen("/proc/self/fd/N") deveria passar onde o
// dlopen("lib/libnvprobe.so") falha.
#include <pthread.h>

__attribute__((visibility("default")))
int nv_probe(void) { return 42; }

static void *fn(void *arg) { *(int *)arg = 1; return 0; }

__attribute__((visibility("default")))
int nv_probe_thread(void) {
  pthread_t t;
  int ok = 0;
  if (pthread_create(&t, 0, fn, &ok) != 0) return 0;
  pthread_join(t, 0);
  return ok;
}
