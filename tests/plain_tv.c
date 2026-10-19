// Run real discovery with a plain SDK-shaped TV manifest and no Home selection.
#define NV_TEST_SESSAO_PROPRIA
#define NV_TEST_ADDONS_PROPRIOS
#define NV_TEST_SOCIAL_PROPRIO
#define main montagem_cedo_main
#define NV_TEST_FIL_REGISTRAR_PROPRIO
#include "montagem_cedo.c"
#undef main

static int tvListed;
void fil_registrar_se_couber_de(const FilPassada *p, const char *key,
                               const char *title, const char *addon, const char *type) {
  assert(fil_passada_valida(p));
  assert(!strcmp(key, "org.example.plaintv_tv_x"));
  assert(!strcmp(title, "X - Canais"));
  assert(!strcmp(addon, "Plain TV") && !strcmp(type, "tv"));
  tvListed++;
}

int main(void) {
  FILE *f = fopen("tests/fixtures/plain_tv/manifest.json", "rb");
  char manifest[2048];
  assert(f);
  size_t n = fread(manifest, 1, sizeof manifest - 1, f);
  assert(!ferror(f) && feof(f)); fclose(f); manifest[n] = 0;
  MANIFESTO = manifest;
  snprintf(dirDados, sizeof dirDados, "%snuvio-plain-tv-%d", nvt("/"), (int)getpid());
  assert(mkdir(dirDados, 0700) == 0);
  desc_tmdb(dirDados);
  homeestado_iniciar();
  desc_iniciar(); esperarQuieto();
  assert(pedidosCatalogo == 0 && !temFileira("x"));
  assert(tvListed == 1); // Before the fix: discarded before any management registration.
  puts("PASS: plain TV catalog listed outside Home; no automatic Home row/request");
  char command[PATH_MAX + 16];
  snprintf(command, sizeof command, "rm -rf '%s'", dirDados);
  assert(system(command) == 0);
  return 0;
}
