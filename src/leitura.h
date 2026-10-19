// LEITURA DE UMA PAGINA DE NOTICIA: o texto principal e a imagem de capa,
// tirados do HTML do veiculo, para o modal de noticia da Agenda.
//
// Pedido do dono (29/09/2026): "quando clicar, abrir o modal com a noticia
// completa; se der, colocar imagens da pagina". O RSS do Google News so traz
// manchete, link, veiculo e data — a <description> dele e a propria manchete
// de novo, embrulhada num <a>. O texto so existe na pagina do veiculo.
//
// UM EXTRATOR PEQUENO, no espirito do Readability, e nao um parser de HTML:
//   1. <meta> de Open Graph / Twitter primeiro (og:title, og:description,
//      og:image, og:site_name): e o que TODO veiculo publica para o
//      compartilhamento em rede social, e e o dado mais estavel da pagina;
//   2. os <p> do MAIOR <article> (o com mais texto em <p>), ou do <body>
//      inteiro quando nao ha <article>;
//   3. o "articleBody" do JSON-LD quando o HTML nao da dois paragrafos — os
//      portais que montam o texto por JavaScript (o G1, por exemplo) deixam o
//      corpo inteiro ali, para o Google.
// O que fica fora: <script>, <style>, <nav>, <header>, <footer>, <aside>,
// <form>, <figure>, e todo bloco cujo class/id diz "related", "share",
// "newsletter", "comment", "advert"... (a lista esta em leitura.c). Paragrafo
// curto (< 60 bytes: credito, "Publicidade", legenda solta) nao entra.
//
// TETOS, porque a entrada e de fora: a pagina chega cortada em 512 KB por quem
// baixa (noticia.c), no maximo LEI_PAR_MAX paragrafos de ate ~900 bytes, e o
// texto total para em ~4 KB. E o que cabe num modal de TV sem rolagem longa.
//
// FUNCOES PURAS (sem rede, sem GL): tests/leitura.c roda com paginas reais
// guardadas em tests/fixtures/noticia-*.html. A MESMA regra roda no worker da
// Samsung (servidor/recomendacoes/src/noticia.js) — quem mexer num mexe no
// outro.
#ifndef NV_LEITURA_H
#define NV_LEITURA_H
#include <stddef.h>

#define LEI_PAR_MAX 6
#define LEI_PAR_TAM 900

typedef struct {
  char titulo[320];
  char resumo[640];       // og:description / description
  char imagem[700];       // og:image, ja absoluta
  char site[120];         // og:site_name
  int  n;                 // paragrafos
  char par[LEI_PAR_MAX][LEI_PAR_TAM];
} Leitura;

// Extrai de `html` (terminado em NUL). `base` e a URL da pagina, para tornar
// og:image absoluta; "" deixa a url como veio. Devolve 1 quando saiu ALGO
// util (titulo, resumo ou paragrafo), 0 quando a pagina nao tinha nada.
int leitura_extrair(const char *html, const char *base, Leitura *saida);

// Uma string JSON a partir da aspa de abertura `p` (escapes \n \" \uXXXX,
// inclusive pares substitutos), em UTF-8. Devolve o ponteiro depois da aspa de
// fechamento, NULL se nao fechou. Serve ao JSON-LD daqui e a resposta do
// worker em noticia.c.
const char *leitura_json_string(const char *p, const char *fim, char *dst, size_t tam);

// Decodifica entidades HTML (&amp; &#8217; &aacute; ...) NO LUGAR, em UTF-8.
void leitura_entidades(char *s);

// --- Google News -----------------------------------------------------------
//
// O <link> do RSS e news.google.com/rss/articles/<id>. Ate 2024 o <id> era a
// URL do veiculo em base64 (protobuf: 08 13 22 <len> <url>); hoje e um token
// opaco ("AU_yqL...") e a unica forma de chegar a URL e a do proprio site: a
// pagina do artigo traz uma assinatura (data-n-a-sg) e um instante
// (data-n-a-ts), e o POST em /_/DotsSplashUi/data/batchexecute com os dois
// devolve "garturlres" com a URL. MEDIDO em 29/09/2026 com curl: a pagina tem
// ~570 KB (a assinatura esta perto do FIM), o POST responde em ~200 ms.
int  leitura_gn_link(const char *url);                     // 1 = link do Google News
int  leitura_gn_id(const char *url, char *dst, size_t tam);
// Formato antigo (base64 com a URL dentro). 1 quando achou uma URL.
int  leitura_gn_antigo(const char *id, char *dst, size_t tam);
int  leitura_gn_assinatura(const char *html, char *sg, size_t tamSg,
                           char *ts, size_t tamTs);
// Corpo x-www-form-urlencoded do POST (f.req=...). 0 se nao coube.
int  leitura_gn_corpo(const char *id, const char *ts, const char *sg,
                      char *dst, size_t tam);
// URL de dentro da resposta do batchexecute. 1 quando achou.
int  leitura_gn_resposta(const char *resp, char *dst, size_t tam);

// Link de agregador que CARREGA a URL do veiculo num parametro: o
// apiclick.aspx do Bing News (o RSS de reserva do worker, ver
// servidor/recomendacoes/src/index.js) traz `&url=https%3a%2f%2f...`. 1 quando
// extraiu uma URL http(s).
int  leitura_link_direto(const char *url, char *dst, size_t tam);

// A URL que vai no QR "Abrir no celular": sem query e sem fragmento (o
// rastreio que os veiculos penduram — utm_*, oc=5 — e o que faz a URL passar
// dos 134 bytes que qr.h aceita). 0 quando nem assim cabe.
int  leitura_url_qr(const char *url, char *dst, size_t tam);
// So o host, sem "www.", para a linha "Leia em g1.globo.com".
void leitura_host(const char *url, char *dst, size_t tam);

#endif
