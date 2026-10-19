// Host falso do .tpk: faz o papel do GLWindow do .NET dentro do container ARM
// (Mesa por software), para provar sem TV que a libnuvio.so sobe, desenha e
// devolve o contexto a cada quadro. tools/tpk-testa.sh compila e roda.
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <dlfcn.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static void salvar(void) {
  static unsigned char buf[1920 * 1080 * 4];
  FILE *f = fopen("/tmp/dados/quadro.ppm", "wb");
  int y, xx;
  glReadPixels(0, 0, 1920, 1080, GL_RGBA, GL_UNSIGNED_BYTE, buf);
  fprintf(f, "P6 1920 1080 255\n");
  for (y = 1079; y >= 0; y--) for (xx = 0; xx < 1920; xx++) fwrite(buf + (y * 1920 + xx) * 4, 1, 3, f);
  fclose(f);
}

int main(int argc, char **argv) {
  setvbuf(stdout, NULL, _IONBF, 0);
  const char *so = argc > 1 ? argv[1] : "./libnuvio.so";
  int quadros = argc > 2 ? atoi(argv[2]) : 600;
  // O host .NET da TV poe o processo no locale do idioma da TV (registros
  // 9866-9920: "FPS=51,2" num Tizen 9). NV_HOST_LOCALE=de_DE.UTF-8 imita isso.
  if (getenv("NV_HOST_LOCALE"))
    printf("host: setlocale(%s) -> %s\n", getenv("NV_HOST_LOCALE"),
           setlocale(LC_ALL, getenv("NV_HOST_LOCALE")) ? "ok" : "falhou");
  void *h = dlopen(so, RTLD_NOW);
  if (!h) { printf("dlopen: %s\n", dlerror()); return 1; }
  int (*iniciar)(const char *, const char *, int, int) = dlsym(h, "nv_tpk_iniciar");
  int (*quadro)(void) = dlsym(h, "nv_tpk_quadro");
  void (*tecla)(const char *, int) = dlsym(h, "nv_tpk_tecla");
  void (*config)(int, int) = dlsym(h, "nv_tpk_config");
  if (!iniciar || !quadro || !tecla) { printf("simbolos faltando\n"); return 1; }

  printf("host: dlopen ok\n");
  EGLDisplay d = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  eglInitialize(d, NULL, NULL);
  EGLint ca[] = { EGL_SURFACE_TYPE, EGL_PBUFFER_BIT, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                  EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8, EGL_NONE };
  EGLConfig c; EGLint n;
  if (!eglChooseConfig(d, ca, &c, 1, &n) || n < 1) { printf("sem config EGL\n"); return 1; }
  EGLint sa[] = { EGL_WIDTH, 1920, EGL_HEIGHT, 1080, EGL_NONE };
  printf("host: config ok\n");
  EGLSurface s = eglCreatePbufferSurface(d, c, sa);
  eglBindAPI(EGL_OPENGL_ES_API);
  EGLint xa[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
  printf("host: pbuffer %p\n", (void *)s);
  EGLContext x = eglCreateContext(d, c, EGL_NO_CONTEXT, xa);
  if (!eglMakeCurrent(d, s, s, x)) { printf("makecurrent falhou\n"); return 1; }
  printf("host: GL %s\n", glGetString(GL_RENDERER));

  if (config) config(50, 0);   // o mesmo do GLWindow API9+ e da TVGLApplication
  if (iniciar("/work/tizen-tpk/NuvioTpk60/res/art", "/tmp/dados", 1920, 1080) != 0) { printf("iniciar falhou\n"); return 1; }
  struct timespec t0, t1; clock_gettime(CLOCK_MONOTONIC, &t0);
  int i, trocas = 0, pulos = 0;
  for (i = 0; trocas < quadros; i++) {
    int r = quadro();
    if (r < 0) { printf("host: app pediu para sair no quadro %d\n", i); break; }
    // Folga larga: no qemu+llvmpipe um quadro do app com o alvo de 720p e a
    // ampliacao chega a 2,5 s, e 50 chamadas por quadro pedido desistiam antes.
    if (r == 0) { pulos++; if (i > quadros * 500) break; usleep(16000); continue; }
    trocas++;
    if (trocas == quadros / 2) { tecla("Down", 1); tecla("Down", 0); }
    if (trocas == quadros - 3) salvar();
    eglSwapBuffers(d, s);
  }
  clock_gettime(CLOCK_MONOTONIC, &t1);
  double seg = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) / 1e9;
  unsigned char px[4] = {0};
  glReadPixels(960, 540, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, px);
  printf("host: %d trocas, %d pulos; ", trocas, pulos);
  printf("host: %d chamadas em %.1f s; pixel central %d,%d,%d,%d\n", i, seg, px[0], px[1], px[2], px[3]);

  return 0;
}
