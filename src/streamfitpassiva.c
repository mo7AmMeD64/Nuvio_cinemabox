#include "streamfitpassiva.h"
#include "streamfit.h"
#include "redemarca.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>

static pthread_mutex_t trava = PTHREAD_MUTEX_INITIALIZER;
static uint64_t permitida;

void streamfitpassiva_permitir(uint64_t geracao) {
  pthread_mutex_lock(&trava);
  permitida = geracao;
  pthread_mutex_unlock(&trava);
}
uint64_t streamfitpassiva_permitida(void) {
  uint64_t g;
  pthread_mutex_lock(&trava); g = permitida; pthread_mutex_unlock(&trava);
  return g;
}

int streamfitpassiva_receber(uint64_t rede, uint64_t geracao, const char *origem,
                             const int *kbps, int n, uint64_t fimMs) {
  int aceitos = 0;
  const char *motivo = "accepted";
  // Only an authority: a path or query from the producer is refused outright,
  // so a resolver URL can never be stored as if it were the media host.
  const char *autoridade = !origem ? NULL : !strncmp(origem, "https://", 8) ? origem + 8 :
                           !strncmp(origem, "http://", 7) ? origem + 7 : NULL;
  if (!autoridade || !*autoridade || strpbrk(autoridade, "/?#@")) motivo = "origin";
  else {
    // Held across the engine call: a generation switch on the UI thread cannot
    // interleave between the check and the store (lock order: this -> engine).
    pthread_mutex_lock(&trava);
    if (!geracao || geracao != permitida) motivo = "stale_generation";
    else if (!rede || rede != redemarca_atual()) motivo = "network";
    else if (!(aceitos = streamfit_passiva(rede, origem, kbps, n, fimMs))) motivo = "rejected";
    pthread_mutex_unlock(&trava);
  }
  /* English, counts only: no host, URL, network id or title. */
  printf("[stream_fit] passive %s intervals=%d accepted=%d\n", motivo, n, aceitos);
  fflush(stdout);
  return aceitos;
}

#ifdef NV_ANDROID
#include <jni.h>
JNIEXPORT void JNICALL
Java_space_nuvio_nativelegacy_NvPlayer_nativeFitPassiva(JNIEnv *env, jclass cls,
    jlong rede, jint geracao, jstring origem, jintArray kbps, jlong fimMs) {
  char o[STREAMFIT_HOST_MAX + 16];
  int v[STREAMFIT_AMOSTRAS_MAX];
  jsize n;
  const char *u;
  (void)cls;
  if (!origem || !kbps || rede <= 0 || geracao <= 0 || fimMs <= 0) return;
  n = (*env)->GetArrayLength(env, kbps);
  if (n < 5 || n > STREAMFIT_AMOSTRAS_MAX) return;
  (*env)->GetIntArrayRegion(env, kbps, 0, n, (jint *)v);
  if ((*env)->ExceptionCheck(env)) { (*env)->ExceptionClear(env); return; }
  u = (*env)->GetStringUTFChars(env, origem, NULL);
  if (!u) return;
  snprintf(o, sizeof o, "%s", u);
  (*env)->ReleaseStringUTFChars(env, origem, u);
  streamfitpassiva_receber((uint64_t)rede, (uint64_t)(unsigned)geracao, o, v, (int)n, (uint64_t)fimMs);
}
#endif
