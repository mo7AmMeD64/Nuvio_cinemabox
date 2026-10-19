// Synthetic pressure test: 32 MiB of payload and twelve live 8 MiB stacks.
// This is a repeatable budget check, not a measurement of a Samsung TV.
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <emscripten.h>
#include <emscripten/heap.h>
#include <emscripten/threading.h>

static atomic_int release_workers;
static void *worker(void *arg) {
  while (!atomic_load(&release_workers)) emscripten_thread_sleep(1);
  return arg;
}
int main(void) {
  pthread_t threads[12];
  volatile unsigned char *payload = malloc(32 * 1024 * 1024);
  if (!payload) return 2;
  // Volatile stores prevent -O2 from deleting an otherwise unused payload.
  for (size_t i = 0; i < 32 * 1024 * 1024; ++i) payload[i] = 1;
  for (int i = 0; i < 12; ++i) {
    if (pthread_create(&threads[i], NULL, worker, NULL)) return 3;
    printf("worker %d criado\n", i + 1);
  }
  atomic_store(&release_workers, 1);
  for (int i = 0; i < 12; ++i) pthread_join(threads[i], NULL);
  free((void *)payload);
  printf("PASS: 12 workers e payload, memoria=%zu MiB\n",
         emscripten_get_heap_size() / (1024 * 1024));
  return 0;
}
