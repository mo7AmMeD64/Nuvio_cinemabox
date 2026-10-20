// Provedor CinemaBox embutido: rotas, ids de 2 segmentos e formatos que
// descoberta.c/addons.c consomem. A rede e substituida por corpos fixos — o
// que se testa e a traducao API -> Stremio, nao o HTTP.
#include "cinemabox.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *rede_baixar(const char *url, int segundos) {
  (void)segundos;
  const char *corpo = NULL;
  if (strstr(url, "/home")) corpo =
    "{\"sections\":["
    "{\"id\":104,\"section_type\":\"normalPoster\",\"title\":\"Movies\",\"data\":["
    "{\"id\":123,\"type\":\"MOVIE\",\"title\":\"Filme Teste\",\"description\":\"sinopse\","
    "\"style\":{\"image\":\"https://a/1.jpg\",\"logo\":\"https://a/1.png\","
    "\"background_image\":\"https://a/1b.jpg\"}},"
    "{\"id\":999,\"type\":\"GENRE\",\"title\":\"Acao\",\"style\":{\"image\":\"https://a/g.jpg\"}}]},"
    "{\"id\":105,\"section_type\":\"normalPoster\",\"title\":\"Series\",\"data\":["
    "{\"id\":321,\"type\":\"SERIES\",\"title\":\"Serie Teste\","
    "\"style\":{\"image\":\"https://a/2.jpg\"}}]}]}";
  else if (strstr(url, "/search"))
    corpo =
    "{\"results\":["
    "{\"id\":28846,\"type\":\"MOVIE\",\"title\":\"Spider\",\"year\":2002,"
    "\"style\":{\"image\":\"https://a/s.jpg\"}},"
    "{\"id\":41156,\"type\":\"SERIES\",\"title\":\"Spider-Noir\",\"year\":2026,"
    "\"style\":{\"image\":\"https://a/n.jpg\"}},"
    "{\"id\":12,\"type\":\"ACTOR\",\"title\":\"Ninguem\",\"style\":{\"image\":\"https://a/x.jpg\"}}],"
    "\"pagination\":{\"current_page\":1,\"total_pages\":12}}";
  else if (strstr(url, "shows/shows/dynamic/28846"))
    corpo =
    "{\"post_info\":{\"genres\":[\"Drama\",\"Mystery\"],\"id\":28846,\"type\":\"MOVIE\","
    "\"title\":\"Spider\",\"logo\":\"https://a/l.png\",\"image\":\"https://a/m.jpg\","
    "\"background_image\":\"https://a/mb.jpg\",\"release_date\":\"1038700800000\","
    "\"description\":\"Um filme\",\"rating\":{\"value\":6.8,\"age\":\"R\"},"
    "\"episode_id\":584239,\"length\":5920},\"sections\":[]}";
  else if (strstr(url, "shows/shows/dynamic/20931"))
    corpo =
    "{\"post_info\":{\"id\":20931,\"type\":\"SERIES\",\"title\":\"the vampire diaries\","
    "\"image\":\"https://a/v.jpg\",\"background_image\":\"https://a/vb.jpg\","
    "\"description\":\"Vampiros\",\"release_date\":\"1252540800000\","
    "\"rating\":{\"value\":7.7,\"age\":\"PG-13\"},\"current_season_id\":10071,"
    "\"season_number\":1,\"length\":2549},"
    "\"sections\":[{\"section_type\":\"normalPoster\",\"title\":\"Seasons\",\"data\":["
    "{\"id\":10071,\"type\":\"season\",\"card_type\":\"seasonPoster\",\"title\":\"1\","
    "\"style\":{\"image\":\"https://a/t1.jpg\"}},"
    "{\"id\":10072,\"type\":\"season\",\"card_type\":\"seasonPoster\",\"title\":\"2\","
    "\"style\":{\"image\":\"https://a/t2.jpg\"}}]}]}";
  else if (strstr(url, "shows/seasons/player/10071"))
    corpo =
    "{\"episodes\":[{\"id\":846133,\"episode_number\":1,\"title\":\"episode 1\","
    "\"image\":\"https://a/e1.jpg\",\"length\":2549,\"videos\":[],\"subtitles\":[]},"
    "{\"id\":846134,\"episode_number\":null,\"title\":\"episode 2\","
    "\"image\":\"https://a/e2.jpg\",\"length\":2539}]}";
  else if (strstr(url, "shows/seasons/player/10072"))
    corpo = "{\"episodes\":[{\"id\":900001,\"episode_number\":1,\"title\":\"episode 1\","
            "\"length\":2500}]}";
  else if (strstr(url, "shows/episodes/player/584239"))
    corpo =
    "{\"subtitles\":[{\"vtt\":\"https://a/s.vtt\",\"srt\":\"https://a/s.srt\","
    "\"language\":\"ar\"}],"
    "\"videos\":[{\"url\":\"https://cdn/v1080.mp4\",\"quality\":\"1080p\"},"
    "{\"url\":\"https://cdn/v720.mp4\",\"quality\":\"720p\"}],\"id\":584239,"
    "\"show_id\":28846,\"show_type\":\"MOVIE\"}";
  else if (strstr(url, "shows/episodes/player/846133"))
    corpo =
    "{\"videos\":[{\"url\":\"https://cdn/e1.mp4\",\"quality\":\"1080p\"}],"
    "\"id\":846133,\"show_id\":20931,\"show_type\":\"SERIES\"}";
  if (!corpo) return NULL;
  size_t n = strlen(corpo);
  char *r = malloc(n + 1);
  if (r) memcpy(r, corpo, n + 1);
  return r;
}

static char *pedir(const char *url, int *nosso) {
  char *r = cinemabox_responder(url, nosso);
  assert(*nosso == 1);
  assert(r != NULL);
  return r;
}

int main(void) {
  int nosso = 0;
  char *r;

  // Outro esquema: a rede segue o caminho normal.
  r = cinemabox_responder("https://exemplo/manifest.json", &nosso);
  assert(!nosso && r == NULL);

  // Manifesto: recursos, prefixo cb: e os dois alvos de busca.
  r = pedir("cinemabox://cb/manifest.json", &nosso);
  assert(strstr(r, "\"idPrefixes\":[\"cb:\"]"));
  assert(strstr(r, "\"id\":\"search\""));
  assert(strstr(r, "\"isRequired\":true"));
  assert(strstr(r, "\"id\":\"144\""));
  // Sem "subtitles" em resources o app nem pede legenda ao addon.
  assert(strstr(r, "{\"name\":\"subtitles\""));
  free(r);

  // Secoes do Explorar: os catalogos do manifesto, sem a busca e sem rede.
  // A ordem e a do manifesto (e a ordem em que o Explorar as mostra).
  { CbCatalogo cat[CB_CAT_MAX];
    int n = cinemabox_catalogos(cat, CB_CAT_MAX), i, busca = 0;
    assert(n == 10);
    for (i = 0; i < n; i++) {
      assert(cat[i].id[0] && cat[i].titulo[0]);
      assert(!strcmp(cat[i].tipo, "movie") || !strcmp(cat[i].tipo, "series"));
      if (!strcmp(cat[i].id, "search")) busca++;
    }
    assert(!busca);
    assert(!strcmp(cat[0].id, "144") && !strcmp(cat[0].titulo, "Featured"));
    assert(!strcmp(cat[n - 1].id, "108") && !strcmp(cat[n - 1].tipo, "series"));
    // Teto respeitado (o Explorar passa CB_CAT_MAX).
    assert(cinemabox_catalogos(cat, 3) == 3);
  }

  // Fileira: cartao MOVIE vira cb:<id>; GENERO e descartado; poster obrigatorio.
  r = pedir("cinemabox://cb/catalog/movie/104.json", &nosso);
  assert(strstr(r, "\"id\":\"cb:123\""));
  assert(strstr(r, "\"type\":\"movie\""));
  assert(strstr(r, "\"name\":\"Filme Teste\""));
  assert(strstr(r, "\"poster\":\"https://a/1.jpg\""));
  assert(strstr(r, "\"background\":\"https://a/1b.jpg\""));
  assert(!strstr(r, "cb:999"));
  free(r);

  // Busca por filme: so o resultado MOVIE, e o termo sobrevive a codificacao.
  r = pedir("cinemabox://cb/catalog/movie/search/search=spider%20man.json", &nosso);
  assert(strstr(r, "\"id\":\"cb:28846\""));
  assert(strstr(r, "\"releaseInfo\":\"2002\""));
  assert(!strstr(r, "cb:41156") && !strstr(r, "cb:12"));
  free(r);
  r = pedir("cinemabox://cb/catalog/series/search/search=spider.json", &nosso);
  assert(strstr(r, "\"id\":\"cb:41156\""));
  assert(!strstr(r, "cb:28846"));
  free(r);

  // Ficha de filme: campos do post_info; filme NAO declara videos (senao o
  // player o trata como serie de um episodio so).
  r = pedir("cinemabox://cb/meta/movie/cb:28846.json", &nosso);
  assert(strstr(r, "\"id\":\"cb:28846\""));
  assert(strstr(r, "\"type\":\"movie\""));
  assert(strstr(r, "\"releaseInfo\":\"2002\""));
  assert(strstr(r, "\"imdbRating\":6.8"));
  assert(strstr(r, "\"runtime\":99"));
  assert(strstr(r, "\"genres\":[\"Drama\",\"Mystery\"]"));
  assert(!strstr(r, "\"videos\""));
  free(r);

  // Ficha de serie: uma temporada por cartao, episodios com id de video de
  // dois segmentos e numero da temporada do cartao.
  r = pedir("cinemabox://cb/meta/series/cb:20931.json", &nosso);
  assert(strstr(r, "\"type\":\"series\""));
  assert(strstr(r, "\"id\":\"cb:846133\""));
  assert(strstr(r, "\"season\":1,\"episode\":1"));
  assert(strstr(r, "\"season\":1,\"episode\":2"));
  assert(strstr(r, "\"id\":\"cb:900001\""));
  assert(strstr(r, "\"season\":2,\"episode\":1"));
  assert(strstr(r, "\"thumbnail\":\"https://a/e1.jpg\""));
  free(r);

  // Fontes: um stream por qualidade, com cabecalho do CDN.
  r = pedir("cinemabox://cb/stream/series/cb:846133.json", &nosso);
  assert(strstr(r, "\"url\":\"https://cdn/e1.mp4\""));
  assert(strstr(r, "\"name\":\"CinemaBox 1080p\""));
  assert(strstr(r, "\"proxyHeaders\""));
  free(r);
  // Filme: o id do catalogo e o show; a fonte vem do episode_id dele.
  r = pedir("cinemabox://cb/stream/movie/cb:28846.json", &nosso);
  assert(strstr(r, "\"url\":\"https://cdn/v1080.mp4\""));
  assert(strstr(r, "\"url\":\"https://cdn/v720.mp4\""));
  free(r);

  // Legendas de filme: o mesmo caminho show -> episode_id; SRT na frente do
  // VTT (e o formato que legenda.c le como principal).
  r = pedir("cinemabox://cb/subtitles/movie/cb:28846.json", &nosso);
  assert(strstr(r, "\"subtitles\":["));
  assert(strstr(r, "\"url\":\"https://a/s.srt\""));
  assert(strstr(r, "\"lang\":\"ar\""));
  assert(!strstr(r, "s.vtt"));
  free(r);
  // Serie: o sufixo ":1" que addons.c acrescenta ao id nao atrapalha.
  r = pedir("cinemabox://cb/subtitles/series/cb:846133:1.json", &nosso);
  assert(!strcmp(r, "{\"subtitles\":[]}"));
  free(r);
  // O caminho com extras (nome do arquivo) tambem responde.
  r = pedir("cinemabox://cb/subtitles/movie/cb:28846/filme.2024.mkv.json", &nosso);
  assert(strstr(r, "\"url\":\"https://a/s.srt\""));
  free(r);

  printf("cinemabox ok\n");
  return 0;
}
