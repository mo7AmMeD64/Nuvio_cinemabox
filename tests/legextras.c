// Ver tests/legextras.sh (#201): extras do Stremio na busca de legendas.
#include "../src/legextras.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int falhas;
static void ok(int c, const char *o, const char *v) {
  printf("  %-58s %s  [%s]\n", o, c ? "ok" : "FALHOU", v ? v : "");
  if (!c) falhas++;
}

// argv: arquivo, hash de referencia (Python), nome de arquivo UTF-8 e a
// codificacao esperada dele (urllib.parse.quote com safe="").
int main(int argc, char **argv) {
  FILE *f;
  long tam;
  unsigned char *ini, *fim;
  char hash[17], seg[512], url[700], nome[256];
  if (argc < 5 || !(f = fopen(argv[1], "rb"))) return 2;
  fseek(f, 0, SEEK_END); tam = ftell(f);
  ini = malloc(LEGEXTRAS_BLOCO); fim = malloc(LEGEXTRAS_BLOCO);
  fseek(f, 0, SEEK_SET); if (fread(ini, 1, LEGEXTRAS_BLOCO, f) != LEGEXTRAS_BLOCO) return 2;
  fseek(f, tam - LEGEXTRAS_BLOCO, SEEK_SET); if (fread(fim, 1, LEGEXTRAS_BLOCO, f) != LEGEXTRAS_BLOCO) return 2;
  fclose(f);
  ok(legextras_hash(ini, LEGEXTRAS_BLOCO, fim, LEGEXTRAS_BLOCO, (uint64_t)tam, hash) &&
     !strcmp(hash, argv[2]), "hash = referencia em Python", hash);
  ok(!legextras_hash(ini, LEGEXTRAS_BLOCO, fim, LEGEXTRAS_BLOCO, 100000, hash) && !hash[0],
     "arquivo < 128 KiB: sem hash", NULL);
  ok(!legextras_hash(ini, 1000, fim, LEGEXTRAS_BLOCO, (uint64_t)tam, hash), "trecho curto: sem hash", NULL);

  legextras_segmento(seg, sizeof seg, argv[3], 1234567890123ull, "8e245d9679d31e12");
  { char esp[512];
    snprintf(esp, sizeof esp, "videoHash=8e245d9679d31e12&videoSize=1234567890123&filename=%s", argv[4]);
    ok(!strcmp(seg, esp), "segmento: ordem e codificacao (espaco, UTF-8, &, +)", seg); }
  ok(legextras_segmento(seg, sizeof seg, NULL, 0, NULL) == 0 && !seg[0], "sem nada: segmento vazio", NULL);
  ok(legextras_segmento(seg, sizeof seg, "a b.mkv", 0, "") > 0 && !strcmp(seg, "filename=a%20b.mkv"),
     "so o nome", seg);
  ok(legextras_segmento(seg, 20, argv[3], 1, "x") < 0 && !seg[0], "nao coube: vazio, nunca cortado", NULL);

  ok(legextras_url(url, sizeof url, "https://a.example/cfg", "movie", "tt0111161", "videoSize=1") &&
     !strcmp(url, "https://a.example/cfg/subtitles/movie/tt0111161/videoSize=1.json"), "url com extras", url);
  ok(legextras_url(url, sizeof url, "https://a.example/cfg", "series", "tt1:2:3", NULL) &&
     !strcmp(url, "https://a.example/cfg/subtitles/series/tt1:2:3.json"), "url sem extras = formato antigo", url);
  ok(!legextras_url(url, 20, "https://a.example/cfg", "movie", "tt1", "x") && !url[0], "url que nao cabe", NULL);
  // #202: a query da URL instalada vai depois do caminho, e o id e codificado
  // (buildSubtitlesUrl do Nuvio web).
  ok(legextras_url(url, sizeof url, "https://a.example/cfg?k=1", "movie", "tt1", "videoSize=1") &&
     !strcmp(url, "https://a.example/cfg/subtitles/movie/tt1/videoSize=1.json?k=1"), "query depois do caminho", url);
  ok(legextras_url(url, sizeof url, "https://a.example/cfg", "movie", "x y", NULL) &&
     !strcmp(url, "https://a.example/cfg/subtitles/movie/x%20y.json"), "id codificado", url);

  ok(legextras_nome_da_url("https://cdn.example/d/abc/My%20Movie.2024.mkv?token=1", nome, sizeof nome) &&
     !strcmp(nome, "My Movie.2024.mkv"), "nome pela URL, decodificado, sem query", nome);
  ok(!legextras_nome_da_url("https://cdn.example/play/12345", nome, sizeof nome) && !nome[0],
     "URL sem cara de video: nada", NULL);
  ok(legextras_url_remota("https://cdn.example/x") && !legextras_url_remota("http://127.0.0.1:11470/x") &&
     !legextras_url_remota("http://localhost:8080/x") && !legextras_url_remota("http://[::1]/x") &&
     !legextras_url_remota("file:///x") && !legextras_url_remota(""), "remota x local/P2P", NULL);
  free(ini); free(fim);
  printf(falhas ? "legextras: %d FALHA(S)\n" : "legextras: tudo ok\n", falhas);
  return falhas != 0;
}
