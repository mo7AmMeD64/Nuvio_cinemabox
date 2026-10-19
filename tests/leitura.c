// O EXTRATOR DE NOTICIA (src/leitura.c) contra paginas de verdade.
//
//   bash tests/leitura.sh
//
// As fixtures sao paginas REAIS baixadas em 29/09/2026 e aparadas (scripts,
// estilos, svg e imagens fora; a estrutura de tags e as classes ficaram): cada
// uma e uma das armadilhas que o extrator precisa passar.
//   noticia-wordpress   Update or Die!: o invólucro da pagina inteira tem
//                       "cs-sidebar-enabled" na class, e os botoes de
//                       compartilhar vem ANTES do texto dentro do mesmo bloco;
//   noticia-arc         Estadao (Arc XP): o texto NAO esta no HTML, esta no JSON
//                       do Fusion; os <p> do HTML sao "leia tambem";
//   noticia-globo       Marie Claire (Globo): <aside> de relacionadas no meio
//                       do <article>, <strong>/<br> dentro dos paragrafos;
//   noticia-duplicada   Series em Cena: o texto aparece DUAS vezes (layout de
//                       celular e de mesa) e fecha com a biografia do autor;
//   noticia-en          Collider: pagina em ingles, entidades tipograficas.
// Mais os casos pequenos escritos aqui: JSON-LD articleBody, Latin-1, imagem
// relativa, e as pecas do Google News (id, formato antigo, assinatura, corpo do
// POST e a resposta) com o que foi medido na rede.
#include "leitura.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int falhas;
#define CONFERE(c, ...) do { if (!(c)) { falhas++; printf("FALHA: " __VA_ARGS__); printf("\n"); } } while (0)

static char *ler(const char *nome) {
  FILE *f = fopen(nome, "rb"); long n; char *s; size_t k;
  if (!f) return NULL;
  fseek(f, 0, SEEK_END); n = ftell(f); fseek(f, 0, SEEK_SET);
  s = malloc((size_t)n + 1);
  if (!s) { fclose(f); return NULL; }
  k = fread(s, 1, (size_t)n, f); s[k] = 0; fclose(f);
  return s;
}

static Leitura L;

static void pagina(const char *arq, const char *base) {
  char *h = ler(arq);
  CONFERE(h != NULL, "fixture %s", arq);
  memset(&L, 0x5a, sizeof L);   // lixo: a funcao tem de zerar tudo
  CONFERE(leitura_extrair(h, base, &L) == 1, "%s: extraiu algo", arq);
  free(h);
}

static int temPar(const char *agulha) {
  int i;
  for (i = 0; i < L.n; i++) if (strstr(L.par[i], agulha)) return 1;
  return 0;
}

int main(void) {
  int i;

  pagina("tests/fixtures/noticia-wordpress.html", "https://updateordie.com/2025/07/02/x/");
  CONFERE(!strcmp(L.titulo, "Foundation temporada 3: primeiras críticas apontam evolução da série da Apple TV+"),
          "wordpress: og:title [%s]", L.titulo);
  CONFERE(!strcmp(L.site, "Update or Die!"), "wordpress: og:site_name [%s]", L.site);
  CONFERE(!strcmp(L.imagem, "https://updateordie.com/wp-content/uploads/2025/07/foundation-tv-show-poster-k-sxd3lqww.webp"),
          "wordpress: og:image [%s]", L.imagem);
  CONFERE(L.n >= 3, "wordpress: o invólucro com 'sidebar' nao leva o texto junto (%d)", L.n);
  CONFERE(L.n && !strncmp(L.par[0], "A série de ficção científica “Foundation”", 44),
          "wordpress: 1o paragrafo com as entidades decodificadas [%.60s]", L.n ? L.par[0] : "");
  CONFERE(!temPar("Share"), "wordpress: botao de compartilhar nao e paragrafo");

  pagina("tests/fixtures/noticia-arc.html", "https://www.estadao.com.br/minha-serie/play/x/");
  CONFERE(!strcmp(L.site, "Estadão"), "arc: site [%s]", L.site);
  CONFERE(L.n >= 3, "arc: o texto veio do JSON do Fusion (%d)", L.n);
  CONFERE(temPar("Os novos atores se juntam a Lou Llobell"), "arc: paragrafo do JSON");
  CONFERE(!temPar("Audi RS 3"), "arc: o 'leia tambem' do HTML ficou de fora");
  CONFERE(!temPar("<em>") && !temPar("<a "), "arc: as tags do JSON sairam");
  CONFERE(strstr(L.imagem, "https://www.estadao.com.br/resizer/") && !strstr(L.imagem, "&amp;"),
          "arc: og:image absoluta e com & decodificado [%.80s]", L.imagem);

  pagina("tests/fixtures/noticia-globo.html", "https://revistamarieclaire.globo.com/Cultura/noticia/x.html");
  CONFERE(L.n >= 4, "globo: paragrafos do artigo apesar do <aside> no meio (%d)", L.n);
  CONFERE(temPar("Lou Llobell e Leah Harvey estão entre as estrelas"), "globo: <strong> dentro do <p> vira texto");
  CONFERE(!temPar("Filme sobre jornalistas que divulgaram"), "globo: a relacionada do <aside> nao entra");

  pagina("tests/fixtures/noticia-duplicada.html", "https://seriesemcena.com.br/noticias/x/");
  { int k, rep = 0;
    for (i = 0; i < L.n; i++) for (k = i + 1; k < L.n; k++) if (!strcmp(L.par[i], L.par[k])) rep = 1;
    CONFERE(!rep, "duplicada: nenhum paragrafo repetido");
    CONFERE(L.n >= 3 && L.n <= LEI_PAR_MAX, "duplicada: %d paragrafos", L.n);
    CONFERE(!temPar("Equipe de redação"), "duplicada: a biografia do autor ficou de fora"); }

  pagina("tests/fixtures/noticia-en.html", "https://collider.com/x/");
  CONFERE(!strcmp(L.site, "Collider"), "en: site [%s]", L.site);
  CONFERE(L.n >= 3, "en: paragrafos (%d)", L.n);
  CONFERE(strstr(L.titulo, "Apple TV\xe2\x80\x99s") != NULL, "en: &rsquo;/’ no titulo [%s]", L.titulo);

  // --- os tetos -------------------------------------------------------------
  { char *h = malloc(200000), *p;
    size_t n = 0;
    n += (size_t)snprintf(h + n, 200000 - n, "<html><body><article>");
    for (i = 0; i < 40; i++) {
      n += (size_t)snprintf(h + n, 200000 - n, "<p>");
      for (p = h + n; (size_t)(p - (h + n)) < 3000; p += 5) memcpy(p, "palav", 5), p[5] = ' ', p++;
      n = (size_t)(p - h);
      n += (size_t)snprintf(h + n, 200000 - n, "</p>");
    }
    snprintf(h + n, 200000 - n, "</article></body></html>");
    leitura_extrair(h, "", &L);
    CONFERE(L.n >= 1 && L.n <= LEI_PAR_MAX, "teto de paragrafos (%d)", L.n);
    for (i = 0; i < L.n; i++) CONFERE(strlen(L.par[i]) < LEI_PAR_TAM, "paragrafo %d cabe", i);
    CONFERE(L.n && !strcmp(L.par[0] + strlen(L.par[0]) - 3, "\xe2\x80\xa6"), "paragrafo longo fecha com reticencias");
    free(h); }

  // --- JSON-LD articleBody (portal que monta o texto por JavaScript) ---------
  leitura_extrair("<html><head><meta property=\"og:title\" content=\"T\">"
                  "<script type=\"application/ld+json\">{\"@type\":\"NewsArticle\",\"articleBody\":"
                  "\"Primeiro par\\u00e1grafo com texto suficiente para passar do piso de sessenta bytes.\\n"
                  "Segundo paragrafo, tambem longo o bastante para contar como texto de materia.\"}</script>"
                  "</head><body><div id=\"root\"></div></body></html>", "", &L);
  CONFERE(L.n == 2 && !strncmp(L.par[0], "Primeiro parágrafo", 19), "json-ld: dois paragrafos (%d) [%.30s]", L.n, L.n ? L.par[0] : "");

  // --- Latin-1 declarado ------------------------------------------------------
  leitura_extrair("<html><head><meta charset=\"iso-8859-1\"><meta property=\"og:title\" content=\"S\xe9rie\">"
                  "</head><body><p>Um par\xe1grafo em Latin-1 que tem de virar UTF-8 antes de chegar a tela.</p></body></html>",
                  "", &L);
  CONFERE(!strcmp(L.titulo, "Série"), "latin-1: titulo em UTF-8 [%s]", L.titulo);
  CONFERE(L.n == 1 && strstr(L.par[0], "parágrafo"), "latin-1: paragrafo em UTF-8");

  // --- imagem relativa, script e comentario ignorados -----------------------
  leitura_extrair("<meta content=\"/img/capa.jpg\" property=\"og:image\"><!-- <p>comentario que nao e texto de materia nenhuma, mesmo longo</p> -->"
                  "<script>var p='<p>isto e javascript e nao pode virar paragrafo de jeito nenhum</p>';</script>"
                  "<p>O unico paragrafo de verdade desta pagina, com mais de sessenta bytes.</p>",
                  "https://exemplo.com.br/noticias/2026/x.html", &L);
  CONFERE(!strcmp(L.imagem, "https://exemplo.com.br/img/capa.jpg"), "imagem com / vira absoluta [%s]", L.imagem);
  CONFERE(L.n == 1, "comentario e script nao viram paragrafo (%d)", L.n);
  leitura_extrair("<meta property=\"og:image\" content=\"capa.jpg\">", "https://e.com/a/b/c.html", &L);
  CONFERE(!strcmp(L.imagem, "https://e.com/a/b/capa.jpg"), "imagem relativa ao diretorio [%s]", L.imagem);
  leitura_extrair("<meta property=\"og:image\" content=\"//cdn.e.com/x.jpg\">", "https://e.com/", &L);
  CONFERE(!strcmp(L.imagem, "https://cdn.e.com/x.jpg"), "imagem sem esquema [%s]", L.imagem);
  CONFERE(leitura_extrair("", "", &L) == 0 && leitura_extrair(NULL, "", &L) == 0, "vazio: nada");

  // --- entidades --------------------------------------------------------------
  { char s[80] = "caf&eacute; &amp; p&atilde;o &#8220;x&#x201D; &hellip;&nbsp;fim &naoexiste;";
    leitura_entidades(s);
    CONFERE(!strcmp(s, "café & pão “x” … fim &naoexiste;"), "entidades [%s]", s); }

  // --- Google News --------------------------------------------------------------
  { char id[200], u[400], sg[64], ts[32], corpo[2048];
    const char *link = "https://news.google.com/rss/articles/CBMiUkFVX3lxTE8tUlllTl9hYUxSSU80d0I5Y0NpU0lEVWd4V3ZETUFOVjZvd05Wb0V2dGFTWUYyQzRwNWxLbTY2YXFiaVFnY1VndWRXdFNKNzBNRnc?oc=5";
    CONFERE(leitura_gn_link(link), "link do Google News");
    CONFERE(!leitura_gn_link("https://www.imdb.com/news/"), "link de veiculo nao e do Google");
    CONFERE(leitura_gn_id(link, id, sizeof id) && !strncmp(id, "CBMiUkFVX3lx", 12) && !strchr(id, '?'),
            "id sem a query [%s]", id);
    CONFERE(!leitura_gn_id("https://news.google.com/rss/articles/AB\"C", id, sizeof id), "id com aspa e recusado");
    // O formato de hoje ("AU_yqL...") NAO traz a URL: tem de dar 0, e nao lixo.
    CONFERE(!leitura_gn_antigo(id, u, sizeof u), "id novo nao decodifica [%s]", u);
    // O antigo: 08 13 22 <len> <url> em base64 de URL (montado aqui a partir
    // de uma URL, como o Google fazia ate 2024).
    CONFERE(leitura_gn_antigo("CBMiK2h0dHBzOi8vd3d3LmV4ZW1wbG8uY29tLmJyL25vdGljaWEvMTIzLmh0bWzSAQA",
                              u, sizeof u) && !strcmp(u, "https://www.exemplo.com.br/noticia/123.html"),
            "formato antigo [%s]", u);
    CONFERE(leitura_gn_assinatura("<c-wiz data-n-a-ts=\"1790698643\" data-n-a-sg=\"AbIaSL_qMv9s\">",
                                  sg, sizeof sg, ts, sizeof ts) && !strcmp(sg, "AbIaSL_qMv9s") && !strcmp(ts, "1790698643"),
            "assinatura e instante");
    CONFERE(!leitura_gn_assinatura("<div data-n-a-ts=\"1x\" data-n-a-sg=\"a\">", sg, sizeof sg, ts, sizeof ts),
            "instante que nao e numero e recusado");
    CONFERE(leitura_gn_corpo(id, "1790698643", "AbIaSL_qMv9s", corpo, sizeof corpo) &&
            !strncmp(corpo, "f.req=%5B%5B%5B%22Fbv4je%22", 27) && strstr(corpo, "1790698643") && strstr(corpo, id),
            "corpo do POST [%.60s]", corpo);
    // A resposta MEDIDA em 29/09/2026 (curl), com o prefixo )]}' do Google.
    CONFERE(leitura_gn_resposta(")]}'\n\n[[\"wrb.fr\",\"Fbv4je\",\"[\\\"garturlres\\\",\\\"https://www.imdb.com/pt/news/ni64147012/\\\",1]\",null,null,null,\"generic\"]]",
                                u, sizeof u) && !strcmp(u, "https://www.imdb.com/pt/news/ni64147012/"),
            "URL da resposta do batchexecute [%s]", u);
    CONFERE(leitura_gn_resposta("[[\"wrb.fr\",\"Fbv4je\",\"[\\\"garturlres\\\",\\\"https://x.com/a?b\\\\u003d1\\\\u0026c\\\\u003d2\\\",1]\"]]",
                                u, sizeof u) && !strcmp(u, "https://x.com/a?b=1&c=2"),
            "\\u003d e \\u0026 escapados [%s]", u);
    CONFERE(!leitura_gn_resposta("[[\"wrb.fr\",\"Fbv4je\",null,null,null,[3],\"generic\"]]", u, sizeof u),
            "assinatura recusada (medido): sem URL"); }

  // --- Bing, QR, host ----------------------------------------------------------
  { char u[400];
    CONFERE(leitura_link_direto("http://www.bing.com/news/apiclick.aspx?ref=FexRss&aid=&tid=6a&url=https%3a%2f%2fwww.yahoo.com%2fentertainment%2fa.html&c=35",
                                u, sizeof u) && !strcmp(u, "https://www.yahoo.com/entertainment/a.html"),
            "Bing apiclick [%s]", u);
    CONFERE(!leitura_link_direto("https://www.bing.com/news/apiclick.aspx?url=javascript%3aalert(1)", u, sizeof u),
            "Bing com url que nao e http e recusado");
    CONFERE(leitura_url_qr("https://www.imdb.com/pt/news/ni64147012/?ref_=x&utm_source=y#topo", u, sizeof u) &&
            !strcmp(u, "https://www.imdb.com/pt/news/ni64147012/"), "QR sem query nem fragmento [%s]", u);
    { char longa[300]; memset(longa, 'a', sizeof longa); memcpy(longa, "https://e.com/", 14); longa[200] = 0;
      CONFERE(!leitura_url_qr(longa, u, sizeof u), "URL maior que a versao 6 do QR: nada"); }
    CONFERE(!leitura_url_qr("javascript:alert(1)", u, sizeof u), "so http(s) vai para o QR");
    leitura_host("https://www.estadao.com.br/minha-serie/x", u, sizeof u);
    CONFERE(!strcmp(u, "estadao.com.br"), "host sem www [%s]", u); }

  if (falhas) { printf("FALHOU: %d\n", falhas); return 1; }
  puts("PASS: leitura (extrator de noticia, Google News, QR).");
  return 0;
}
