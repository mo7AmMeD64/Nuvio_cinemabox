// Ver leitura.h.
#include "leitura.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- utilitarios ------------------------------------------------------------

static int iguaisN(const char *a, const char *b, size_t n) {
  size_t i;
  for (i = 0; i < n; i++) {
    if (!a[i] || !b[i]) return 0;
    if (tolower((unsigned char)a[i]) != tolower((unsigned char)b[i])) return 0;
  }
  return 1;
}

// strstr sem caixa, limitado a [p, fim).
static const char *achaSemCaixa(const char *p, const char *fim, const char *agulha) {
  size_t n = strlen(agulha);
  for (; p + n <= fim && *p; p++)
    if (iguaisN(p, agulha, n)) return p;
  return NULL;
}

static void copia(char *dst, size_t tam, const char *src, size_t n) {
  if (!tam) return;
  if (n >= tam) n = tam - 1;
  memcpy(dst, src, n);
  dst[n] = 0;
}

// UTF-8 de um ponto de codigo (ate 4 bytes). Devolve quantos escreveu.
static int utf8(unsigned long c, char *o) {
  if (c < 0x80) { o[0] = (char)c; return 1; }
  if (c < 0x800) { o[0] = (char)(0xC0 | (c >> 6)); o[1] = (char)(0x80 | (c & 0x3F)); return 2; }
  if (c < 0x10000) {
    o[0] = (char)(0xE0 | (c >> 12)); o[1] = (char)(0x80 | ((c >> 6) & 0x3F));
    o[2] = (char)(0x80 | (c & 0x3F)); return 3;
  }
  if (c < 0x110000) {
    o[0] = (char)(0xF0 | (c >> 18)); o[1] = (char)(0x80 | ((c >> 12) & 0x3F));
    o[2] = (char)(0x80 | ((c >> 6) & 0x3F)); o[3] = (char)(0x80 | (c & 0x3F)); return 4;
  }
  return 0;
}

// --- entidades ----------------------------------------------------------------
//
// As nomeadas que aparecem em texto de noticia de verdade (medido nas quatro
// paginas de tests/fixtures): as de pontuacao tipografica e o bloco Latin-1
// inteiro — portugues, frances, alemao e espanhol cabem nele. O resto chega
// numerico (&#8217;), e esse caminho cobre qualquer caractere.
static const struct { const char *n; unsigned short c; } ENT[] = {
  {"nbsp",160},{"amp",38},{"lt",60},{"gt",62},{"quot",34},{"apos",39},
  {"hellip",8230},{"mdash",8212},{"ndash",8211},{"lsquo",8216},{"rsquo",8217},
  {"sbquo",8218},{"ldquo",8220},{"rdquo",8221},{"bdquo",8222},{"laquo",171},
  {"raquo",187},{"copy",169},{"reg",174},{"trade",8482},{"deg",176},
  {"middot",183},{"bull",8226},{"euro",8364},{"ordf",170},{"ordm",186},
  {"iexcl",161},{"iquest",191},{"szlig",223},{"shy",173},{"thinsp",8201},
  {"ensp",8194},{"emsp",8195},{"zwnj",8204},{"zwj",8205},{"prime",8242},
};
// 192..255, na ordem da tabela Latin-1.
static const char *const LAT1[64] = {
  "Agrave","Aacute","Acirc","Atilde","Auml","Aring","AElig","Ccedil",
  "Egrave","Eacute","Ecirc","Euml","Igrave","Iacute","Icirc","Iuml",
  "ETH","Ntilde","Ograve","Oacute","Ocirc","Otilde","Ouml","times",
  "Oslash","Ugrave","Uacute","Ucirc","Uuml","Yacute","THORN","szlig",
  "agrave","aacute","acirc","atilde","auml","aring","aelig","ccedil",
  "egrave","eacute","ecirc","euml","igrave","iacute","icirc","iuml",
  "eth","ntilde","ograve","oacute","ocirc","otilde","ouml","divide",
  "oslash","ugrave","uacute","ucirc","uuml","yacute","thorn","yuml",
};

static unsigned long entidade(const char *nome, size_t n) {
  size_t i;
  char b[16];
  if (n == 0 || n >= sizeof b) return 0;
  memcpy(b, nome, n); b[n] = 0;
  for (i = 0; i < sizeof ENT / sizeof ENT[0]; i++)
    if (!strcmp(b, ENT[i].n)) return ENT[i].c;
  for (i = 0; i < 64; i++)
    if (!strcmp(b, LAT1[i])) return 192 + (unsigned long)i;
  return 0;
}

void leitura_entidades(char *s) {
  char *r = s, *w = s;
  if (!s) return;
  while (*r) {
    if (*r == '&') {
      const char *f = r + 1;
      unsigned long c = 0;
      size_t n;
      while (*f && *f != ';' && f - r < 12) f++;
      if (*f == ';') {
        n = (size_t)(f - (r + 1));
        if (r[1] == '#') {
          if (r[2] == 'x' || r[2] == 'X') c = strtoul(r + 3, NULL, 16);
          else c = strtoul(r + 2, NULL, 10);
        } else c = entidade(r + 1, n);
        if (c > 0 && c < 0x110000) {
          char o[4];
          int k = utf8(c == 160 ? 32 : c, o), j;   // nbsp vira espaco: a quebra e nossa
          // Nunca escreve alem do que ja leu: toda entidade tem >= 4 bytes e a
          // saida tem <= 4, entao `w` nao alcanca `r`.
          for (j = 0; j < k; j++) *w++ = o[j];
          r = (char *)f + 1;
          continue;
        }
      }
    }
    *w++ = *r++;
  }
  *w = 0;
}

// Espacos em sequencia (inclusive \n, \t, e o nbsp ja convertido) viram um so,
// e as bordas saem.
static void compacta(char *s) {
  char *r = s, *w = s;
  int esp = 1;
  while (*r) {
    unsigned char c = (unsigned char)*r++;
    if (c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '\f' || c == '\v') {
      if (!esp) { *w++ = ' '; esp = 1; }
      continue;
    }
    *w++ = (char)c; esp = 0;
  }
  if (w > s && w[-1] == ' ') w--;
  *w = 0;
}

// Corta em `tam` sem partir palavra nem caractere UTF-8, fechando com "…".
static void cortaPalavra(char *s, size_t tam) {
  size_t n = strlen(s), k;
  if (n < tam) return;
  k = tam - 4;
  while (k > 0 && s[k] != ' ') k--;
  if (k < tam / 2) { k = tam - 4; while (k > 0 && ((unsigned char)s[k] & 0xC0) == 0x80) k--; }
  memcpy(s + k, "\xe2\x80\xa6", 4);
}

// --- atributos --------------------------------------------------------------

// Valor do atributo `nome` dentro de [ini, fim) de uma tag. 1 quando achou.
static int atributo(const char *ini, const char *fim, const char *nome,
                    char *dst, size_t tam) {
  size_t nn = strlen(nome);
  const char *p = ini;
  dst[0] = 0;
  while (p < fim) {
    const char *a;
    while (p < fim && (isspace((unsigned char)*p) || *p == '/')) p++;
    a = p;
    while (p < fim && *p != '=' && !isspace((unsigned char)*p) && *p != '>' && *p != '/') p++;
    { size_t la = (size_t)(p - a);
      const char *v = NULL; size_t lv = 0;
      while (p < fim && isspace((unsigned char)*p)) p++;
      if (p < fim && *p == '=') {
        p++;
        while (p < fim && isspace((unsigned char)*p)) p++;
        if (p < fim && (*p == '"' || *p == '\'')) {
          char q = *p++;
          v = p;
          while (p < fim && *p != q) p++;
          lv = (size_t)(p - v);
          if (p < fim) p++;
        } else {
          v = p;
          while (p < fim && !isspace((unsigned char)*p) && *p != '>') p++;
          lv = (size_t)(p - v);
        }
      }
      if (la == nn && iguaisN(a, nome, nn)) {
        if (v) copia(dst, tam, v, lv);
        return 1;
      }
      if (la == 0 && p < fim) p++;
    }
  }
  return 0;
}

// --- a extracao ---------------------------------------------------------------

// Tags cujo conteudo inteiro fica de fora.
static int tagDescarta(const char *t) {
  static const char *const L[] = { "script", "style", "noscript", "nav", "header",
    "footer", "aside", "form", "figure", "figcaption", "button", "svg", "iframe",
    "template", "select", "textarea", "video", "audio", "object", "canvas", 0 };
  int i;
  for (i = 0; L[i]; i++) if (!strcmp(t, L[i])) return 1;
  return 0;
}
// Conteudo cru (nao e HTML dentro): pula ate a tag de fechamento.
static int tagCrua(const char *t) {
  return !strcmp(t, "script") || !strcmp(t, "style") || !strcmp(t, "textarea") ||
         !strcmp(t, "template") || !strcmp(t, "noscript");
}
static int tagVazia(const char *t) {
  static const char *const L[] = { "br", "img", "meta", "link", "input", "hr",
    "source", "wbr", "area", "base", "col", "embed", "param", "track", 0 };
  int i;
  for (i = 0; L[i]; i++) if (!strcmp(t, L[i])) return 1;
  return 0;
}
// Tag de BLOCO: abrir uma dessas fecha o <p> em curso (o HTML deixa o </p>
// opcional e metade dos CMS nao escreve).
static int tagBloco(const char *t) {
  static const char *const L[] = { "p", "div", "section", "article", "ul", "ol",
    "li", "table", "h1", "h2", "h3", "h4", "h5", "h6", "blockquote", "main", 0 };
  int i;
  for (i = 0; L[i]; i++) if (!strcmp(t, L[i])) return 1;
  return 0;
}
static int ehConteiner(const char *t) {
  return !strcmp(t, "div") || !strcmp(t, "section") || !strcmp(t, "ul") ||
         !strcmp(t, "ol") || !strcmp(t, "span") || !strcmp(t, "p") ||
         !strcmp(t, "aside") || !strcmp(t, "li");
}

// Class/id que dizem "isto nao e o texto": os "unlikely candidates" do
// Readability, reduzidos ao que as paginas medidas usam de fato — e com a MESMA
// salvaguarda dele: quem tambem diz "content", "article", "main"... NAO sai.
// Sem ela, "cs-site-content cs-sidebar-enabled" (o invólucro da pagina inteira
// do Update or Die!, fixture noticia-wordpress) levava o texto junto com a
// barra lateral.
static int contem(const char *c, const char *const *L) {
  char b[240];
  size_t n = strlen(c), k;
  int i;
  if (n >= sizeof b) n = sizeof b - 1;
  for (k = 0; k < n; k++) b[k] = (char)tolower((unsigned char)c[k]);
  b[n] = 0;
  for (i = 0; L[i]; i++) if (strstr(b, L[i])) return 1;
  return 0;
}
static int classeFora(const char *c) {
  static const char *const FORA[] = { "comment", "share", "social", "related",
    "newsletter", "promo", "advert", "sponsor", "banner", "cookie", "popup",
    "breadcrumb", "subscribe", "sidebar", "recommend", "outbrain", "taboola",
    "disqus", "read-more", "leia-tambem", "saiba-mais", "veja-tambem",
    "author-bio", "byline", "caption", "credit", "footer", "menu", 0 };
  static const char *const SALVA[] = { "article", "content", "main", "body",
    "post", "entry", "story", "texto", "materia", "column", 0 };
  if (!c[0] || !contem(c, FORA)) return 0;
  return !contem(c, SALVA);
}
// Class/id/itemprop que dizem "ESTE e o corpo": o no ganha bonus de nota.
static int classeCorpo(const char *c) {
  static const char *const L[] = { "articlebody", "article-body", "article__body",
    "article-content", "entry-content", "post-content", "content-text",
    "story-body", "materia-conteudo", "news-body", "texto-materia", 0 };
  return c[0] && contem(c, L);
}

#define LEI_CAND_MAX 160
#define LEI_PILHA 256
#define LEI_TOTAL 4200

typedef struct { char nome[12]; int descarta, id, corpo; } Nivel;
// Cada <p> guarda quem e o PAI e o AVO dele: e por esses nos que o texto e
// pontuado (ver a escolha, adiante).
typedef struct { int pai, avo, paiCorpo, avoCorpo; char txt[LEI_PAR_TAM]; } Candidato;

static Candidato cand[LEI_CAND_MAX];
static int nCand;
static char parAtual[4096];
static size_t nParAtual;

static void fechaParagrafo(const Nivel *pilha, int prof) {
  if (!nParAtual) return;
  parAtual[nParAtual] = 0;
  leitura_entidades(parAtual);
  compacta(parAtual);
  if (strlen(parAtual) >= 60 && nCand < LEI_CAND_MAX) {
    Candidato *c = &cand[nCand];
    // O <p> em si esta no topo da pilha quando fechado por </p>; quando e o
    // bloco seguinte que fecha, ele tambem ainda esta la. Pai = um abaixo.
    int ip = prof - 1;
    while (ip >= 0 && strcmp(pilha[ip].nome, "p")) ip--;
    cortaPalavra(parAtual, LEI_PAR_TAM);
    // Sem pai (fragmento sem <html>/<body>, ou <p> na raiz): o no -1, a raiz
    // do documento. 0 e "nenhum" (o avo de quem esta na raiz).
    c->pai = ip >= 1 ? pilha[ip - 1].id : -1;
    c->avo = ip >= 2 ? pilha[ip - 2].id : (ip == 1 ? -1 : 0);
    c->paiCorpo = ip >= 1 ? pilha[ip - 1].corpo : 0;
    c->avoCorpo = ip >= 2 ? pilha[ip - 2].corpo : 0;
    snprintf(c->txt, sizeof c->txt, "%s", parAtual);
    nCand++;
  }
  nParAtual = 0;
}

static void juntaTexto(const char *a, const char *b) {
  while (a < b && nParAtual + 1 < sizeof parAtual) parAtual[nParAtual++] = *a++;
}

// Absoluta a partir da pagina. `base` sem esquema (captura/fixture) deixa como
// veio.
static void absoluta(const char *u, const char *base, char *dst, size_t tam) {
  const char *esq = strstr(base ? base : "", "://");
  dst[0] = 0;
  if (!u[0]) return;
  if (strstr(u, "://")) { snprintf(dst, tam, "%s", u); return; }
  if (u[0] == '/' && u[1] == '/') { snprintf(dst, tam, "https:%s", u); return; }
  if (!esq) { snprintf(dst, tam, "%s", u); return; }
  { const char *h = esq + 3, *fimHost = h;
    while (*fimHost && *fimHost != '/' && *fimHost != '?' && *fimHost != '#') fimHost++;
    if (u[0] == '/') { snprintf(dst, tam, "%.*s%s", (int)(fimHost - base), base, u); return; }
    { const char *ult = fimHost, *q;
      for (q = fimHost; *q && *q != '?' && *q != '#'; q++) if (*q == '/') ult = q;
      if (ult == fimHost) snprintf(dst, tam, "%.*s/%s", (int)(fimHost - base), base, u);
      else snprintf(dst, tam, "%.*s%s", (int)(ult + 1 - base), base, u); } }
}

// Uma string JSON a partir da aspa de abertura `p`, decodificada em `dst`.
// Devolve o ponteiro logo depois da aspa de fechamento (NULL se nao fechou).
const char *leitura_json_string(const char *p, const char *fim, char *dst, size_t tam) {
  size_t n = 0;
  if (!tam || p >= fim || *p != '"') return NULL;
  p++;
  while (p < fim && *p && *p != '"') {
    if (*p == '\\' && p + 1 < fim) {
      p++;
      if (*p == 'u' && p + 4 < fim) {
        unsigned long c = strtoul((char[5]){ p[1], p[2], p[3], p[4], 0 }, NULL, 16);
        p += 4;
        if (c >= 0xD800 && c < 0xDC00 && p + 6 < fim && p[1] == '\\' && p[2] == 'u') {
          unsigned long lo = strtoul((char[5]){ p[3], p[4], p[5], p[6], 0 }, NULL, 16);
          c = 0x10000 + ((c - 0xD800) << 10) + (lo - 0xDC00);
          p += 6;
        }
        if (n + 5 < tam) n += (size_t)utf8(c, dst + n);
      } else if (n + 1 < tam) {
        dst[n++] = *p == 'n' ? '\n' : (*p == 'r' || *p == 't') ? ' ' : *p;
      }
      p++;
      continue;
    }
    if (n + 1 < tam) dst[n++] = *p;
    p++;
  }
  dst[n] = 0;
  return (p < fim && *p == '"') ? p + 1 : NULL;
}

// Texto de fonte JSON (articleBody, Arc) para paragrafo: sem tags, sem
// entidades, espacos compactados. 1 quando sobrou um paragrafo de verdade.
static int paragrafoDeJson(const char *s, char *par, size_t tam) {
  size_t k = 0;
  int dentro = 0;
  for (; *s && k + 1 < tam; s++) {
    if (*s == '<') { dentro = 1; continue; }
    if (*s == '>' && dentro) { dentro = 0; par[k++] = ' '; continue; }
    if (!dentro) par[k++] = *s;
  }
  par[k] = 0;
  leitura_entidades(par);
  compacta(par);
  if (strlen(par) < 60) return 0;
  cortaPalavra(par, LEI_PAR_TAM);
  return 1;
}

static void poePar(Leitura *L, const char *par, int *total) {
  int j;
  if (L->n >= LEI_PAR_MAX || *total >= LEI_TOTAL) return;
  for (j = 0; j < L->n; j++) if (!strcmp(L->par[j], par)) return;
  snprintf(L->par[L->n], sizeof L->par[L->n], "%s", par);
  *total += (int)strlen(par);
  L->n++;
}

// JSON-LD: o "articleBody", quebrado nos \n.
static void doJsonLd(const char *ini, const char *fim, Leitura *L) {
  static char corpo[12288];
  const char *p = achaSemCaixa(ini, fim, "\"articleBody\"");
  char *s, *nl;
  int total = 0;
  if (!p) return;
  p += 13;
  while (p < fim && (isspace((unsigned char)*p) || *p == ':')) p++;
  if (!leitura_json_string(p, fim, corpo, sizeof corpo) && !corpo[0]) return;
  L->n = 0;
  for (s = corpo; s && *s && L->n < LEI_PAR_MAX; s = nl ? nl + 1 : NULL) {
    char par[LEI_PAR_TAM + 8];
    nl = strchr(s, '\n');
    if (nl) *nl = 0;
    if (paragrafoDeJson(s, par, sizeof par)) poePar(L, par, &total);
  }
}

// ARC XP (Estadao, El Pais, Infobae...): o texto nao esta no HTML, esta no
// JSON do Fusion como {"_id":..,"content":"<p>..</p>","type":"text"}, na ordem
// da materia. So entra o bloco cujo "type" logo depois e "text" — os de
// imagem, video e "leia tambem" tem outros tipos.
static void doArc(const char *ini, const char *fim, Leitura *L) {
  static char txt[6144];
  const char *p = ini;
  int total = 0;
  while ((p = achaSemCaixa(p, fim, "\"content\":\"")) && L->n < LEI_PAR_MAX) {
    const char *f = leitura_json_string(p + 10, fim, txt, sizeof txt);
    char par[LEI_PAR_TAM + 8];
    if (!f) break;
    if (!strncmp(f, ",\"type\":\"text\"", 14) && paragrafoDeJson(txt, par, sizeof par))
      poePar(L, par, &total);
    p = f;
  }
}

// Latin-1 declarado vira UTF-8 antes de tudo. Devolve um buffer novo, ou NULL
// quando a pagina ja e UTF-8 (o caso de quase todas).
static char *paraUtf8(const char *html) {
  const char *fimCab = html + strnlen(html, 4096);
  const char *c = achaSemCaixa(html, fimCab, "charset");
  size_t n, k = 0;
  char *o;
  const unsigned char *p;
  if (!c) return NULL;
  c += 7;
  while (c < fimCab && (*c == '=' || *c == '"' || *c == '\'' || isspace((unsigned char)*c))) c++;
  if (!iguaisN(c, "iso-8859-1", 10) && !iguaisN(c, "windows-1252", 12) &&
      !iguaisN(c, "latin1", 6) && !iguaisN(c, "iso8859-1", 9)) return NULL;
  n = strlen(html);
  o = malloc(n * 2 + 1);
  if (!o) return NULL;
  for (p = (const unsigned char *)html; *p; p++) {
    if (*p < 0x80) o[k++] = (char)*p;
    else { o[k++] = (char)(0xC0 | (*p >> 6)); o[k++] = (char)(0x80 | (*p & 0x3F)); }
  }
  o[k] = 0;
  return o;
}

// A NOTA DE UM NO: soma do texto dos <p> de quem ele e pai, mais metade do dos
// netos — a regra do Readability. Corpo reconhecido (<article>, itemprop,
// class de corpo) vale 1,5x. O no de maior nota e o texto; os <p> que entram
// sao os filhos e netos dele. Por no, e nao "o maior <article>", porque a
// mesma pagina traz o texto DUAS vezes (layout de celular e de mesa, fixture
// noticia-seriesemcena) e cartoes de "leia tambem" dentro de <article>: o no
// de verdade ganha pela massa de texto, e a copia fica de fora sozinha.
static int melhorNo(void) {
  static int ids[LEI_CAND_MAX * 2];
  static double nota[LEI_CAND_MAX * 2];
  int n = 0, k, j, melhor = -1;
  for (k = 0; k < nCand; k++) {
    int alvo[2] = { cand[k].pai, cand[k].avo };
    double peso[2] = { cand[k].paiCorpo ? 1.5 : 1.0, cand[k].avoCorpo ? 0.75 : 0.5 };
    int t;
    for (t = 0; t < 2; t++) {
      if (!alvo[t]) continue;
      for (j = 0; j < n && ids[j] != alvo[t]; j++) {}
      if (j == n) { ids[n] = alvo[t]; nota[n] = 0.0; n++; }
      nota[j] += peso[t] * (double)strlen(cand[k].txt);
    }
  }
  for (j = 0; j < n; j++) if (melhor < 0 || nota[j] > nota[melhor]) melhor = j;
  return melhor >= 0 ? ids[melhor] : 0;
}

int leitura_extrair(const char *htmlCru, const char *base, Leitura *L) {
  static Nivel pilha[LEI_PILHA];
  int prof = 0, emP = 0, nos = 0, i;
  char *convertido;
  const char *html, *p, *fimDoc;
  char tituloTag[320] = "", twTit[320] = "", twDesc[640] = "", desc[640] = "";
  char img[700] = "", twImg[700] = "";
  const char *ldIni = NULL, *ldFim = NULL;

  memset(L, 0, sizeof *L);
  if (!htmlCru || !htmlCru[0]) return 0;
  convertido = paraUtf8(htmlCru);
  html = convertido ? convertido : htmlCru;
  fimDoc = html + strlen(html);
  nCand = 0; nParAtual = 0;

  for (p = html; p < fimDoc; ) {
    if (*p != '<') {
      const char *t = p;
      while (p < fimDoc && *p != '<') p++;
      if (emP && !(prof > 0 && pilha[prof - 1].descarta)) juntaTexto(t, p);
      continue;
    }
    if (!strncmp(p, "<!--", 4)) {
      const char *f = strstr(p + 4, "-->");
      p = f ? f + 3 : fimDoc;
      continue;
    }
    if (p[1] == '!' || p[1] == '?') {
      const char *f = strchr(p, '>');
      p = f ? f + 1 : fimDoc;
      continue;
    }
    { int fecha = p[1] == '/';
      const char *n = p + 1 + fecha, *a, *f;
      char nome[12];
      size_t ln = 0;
      while (n + ln < fimDoc && (isalnum((unsigned char)n[ln]) || n[ln] == '-') && ln < sizeof nome - 1) {
        nome[ln] = (char)tolower((unsigned char)n[ln]); ln++;
      }
      nome[ln] = 0;
      if (!ln) { if (emP) juntaTexto(p, p + 1); p++; continue; }
      a = n + ln;
      // Fim da tag respeitando aspas: um ">" dentro de atributo nao fecha.
      for (f = a; f < fimDoc && *f != '>'; f++)
        if (*f == '"' || *f == '\'') { char q = *f; f++; while (f < fimDoc && *f != q) f++; if (f >= fimDoc) break; }
      if (f >= fimDoc) break;

      if (fecha) {
        int k;
        // </p> fecha o paragrafo, e o fim de QUALQUER bloco tambem: o <p> sem
        // fechamento dentro de um <div> nao pode engolir o texto seguinte.
        if (emP && tagBloco(nome)) { fechaParagrafo(pilha, prof); emP = 0; }
        for (k = prof - 1; k >= 0; k--) if (!strcmp(pilha[k].nome, nome)) break;
        if (k >= 0) prof = k;
        p = f + 1;
        continue;
      }

      // --- abertura ---
      if (!strcmp(nome, "meta")) {
        char chave[80] = "", val[700] = "";
        if (!atributo(a, f, "property", chave, sizeof chave) || !chave[0])
          if (!atributo(a, f, "name", chave, sizeof chave) || !chave[0])
            atributo(a, f, "itemprop", chave, sizeof chave);
        if (chave[0] && atributo(a, f, "content", val, sizeof val) && val[0]) {
          for (i = 0; chave[i]; i++) chave[i] = (char)tolower((unsigned char)chave[i]);
          if (!strcmp(chave, "og:title") && !L->titulo[0]) snprintf(L->titulo, sizeof L->titulo, "%s", val);
          else if (!strcmp(chave, "twitter:title") && !twTit[0]) snprintf(twTit, sizeof twTit, "%s", val);
          else if (!strcmp(chave, "og:description") && !L->resumo[0]) snprintf(L->resumo, sizeof L->resumo, "%s", val);
          else if (!strcmp(chave, "twitter:description") && !twDesc[0]) snprintf(twDesc, sizeof twDesc, "%s", val);
          else if (!strcmp(chave, "description") && !desc[0]) snprintf(desc, sizeof desc, "%s", val);
          else if ((!strcmp(chave, "og:image") || !strcmp(chave, "og:image:url") ||
                    !strcmp(chave, "og:image:secure_url")) && !img[0]) snprintf(img, sizeof img, "%s", val);
          else if ((!strcmp(chave, "twitter:image") || !strcmp(chave, "twitter:image:src")) && !twImg[0])
            snprintf(twImg, sizeof twImg, "%s", val);
          else if (!strcmp(chave, "og:site_name") && !L->site[0]) snprintf(L->site, sizeof L->site, "%s", val);
        }
        p = f + 1;
        continue;
      }
      if (!strcmp(nome, "title") && !tituloTag[0]) {
        const char *fimT = achaSemCaixa(f + 1, fimDoc, "</title");
        if (fimT) { copia(tituloTag, sizeof tituloTag, f + 1, (size_t)(fimT - (f + 1))); p = fimT; continue; }
      }
      if (tagCrua(nome)) {
        char fim2[16];
        const char *fimC;
        snprintf(fim2, sizeof fim2, "</%s", nome);
        fimC = achaSemCaixa(f + 1, fimDoc, fim2);
        if (!strcmp(nome, "script") && !ldIni) {
          char tipo[40] = "";
          atributo(a, f, "type", tipo, sizeof tipo);
          if (iguaisN(tipo, "application/ld+json", 19) && fimC &&
              achaSemCaixa(f + 1, fimC, "\"articleBody\"")) { ldIni = f + 1; ldFim = fimC; }
        }
        // O fechamento e consumido na volta seguinte, como tag de fecho sem
        // par na pilha (a crua nunca foi empilhada).
        p = fimC ? fimC : fimDoc;
        continue;
      }
      if (tagBloco(nome) && emP) { fechaParagrafo(pilha, prof); emP = 0; }
      if (!strcmp(nome, "br") && emP) { static const char esp[] = " "; juntaTexto(esp, esp + 1); }
      if (!tagVazia(nome) && f[-1] != '/') {
        char cls[240] = "", id[120] = "", ip[40] = "";
        int pai = prof > 0 ? prof - 1 : -1;
        atributo(a, f, "class", cls, sizeof cls);
        atributo(a, f, "id", id, sizeof id);
        atributo(a, f, "itemprop", ip, sizeof ip);
        if (prof < LEI_PILHA) {
          Nivel *nv = &pilha[prof++];
          snprintf(nv->nome, sizeof nv->nome, "%s", nome);
          nv->descarta = (pai >= 0 && pilha[pai].descarta) || tagDescarta(nome) ||
                         (ehConteiner(nome) && (classeFora(cls) || classeFora(id)));
          nv->id = ++nos;
          nv->corpo = !strcmp(nome, "article") || classeCorpo(cls) || classeCorpo(id) ||
                      !strcmp(ip, "articleBody");
          if (!strcmp(nome, "p") && !nv->descarta) { emP = 1; nParAtual = 0; }
        }
      }
      p = f + 1;
    }
  }
  if (emP) fechaParagrafo(pilha, prof);

  // --- o no vencedor -----------------------------------------------------------
  { int no = melhorNo(), k, total = 0;
    for (k = 0; k < nCand; k++)
      if (no && (cand[k].pai == no || cand[k].avo == no)) poePar(L, cand[k].txt, &total); }

  // --- as fontes JSON, quando o HTML nao deu texto -----------------------------
  if (L->n < 2 && ldIni) doJsonLd(ldIni, ldFim, L);
  if (L->n < 2) { Leitura arc; memset(&arc, 0, sizeof arc); doArc(html, fimDoc, &arc);
    if (arc.n > L->n) { memcpy(L->par, arc.par, sizeof arc.par); L->n = arc.n; } }

  if (!L->titulo[0]) snprintf(L->titulo, sizeof L->titulo, "%s", twTit[0] ? twTit : tituloTag);
  if (!L->resumo[0]) snprintf(L->resumo, sizeof L->resumo, "%s", twDesc[0] ? twDesc : desc);
  leitura_entidades(L->titulo);  compacta(L->titulo);
  leitura_entidades(L->resumo);  compacta(L->resumo);
  leitura_entidades(L->site);    compacta(L->site);
  { char cru[700];
    snprintf(cru, sizeof cru, "%s", img[0] ? img : twImg);
    leitura_entidades(cru);
    compacta(cru);
    // "data:" (pixel de rastreio embutido) nao e capa.
    if (!strncmp(cru, "data:", 5)) cru[0] = 0;
    absoluta(cru, base, L->imagem, sizeof L->imagem); }
  // O resumo repetido como primeiro paragrafo (varios CMS fazem isso) sai.
  if (L->n > 1 && L->resumo[0] &&
      !strncmp(L->par[0], L->resumo, strlen(L->resumo) > 80 ? 80 : strlen(L->resumo))) {
    for (i = 1; i < L->n; i++) memcpy(L->par[i - 1], L->par[i], sizeof L->par[i]);
    L->n--;
  }
  free(convertido);
  return L->titulo[0] || L->resumo[0] || L->n > 0;
}

// --- Google News ----------------------------------------------------------------

int leitura_gn_link(const char *url) {
  return url && (strstr(url, "://news.google.com/rss/articles/") ||
                 strstr(url, "://news.google.com/articles/")) ? 1 : 0;
}

int leitura_gn_id(const char *url, char *dst, size_t tam) {
  const char *p = url ? strstr(url, "/articles/") : NULL;
  size_t n = 0;
  if (!tam) return 0;
  dst[0] = 0;
  if (!p) return 0;
  p += 10;
  while (p[n] && p[n] != '?' && p[n] != '#' && p[n] != '/') n++;
  if (!n || n >= tam) return 0;
  // Id so tem o alfabeto do base64 de URL; o resto seria injecao no POST.
  { size_t k;
    for (k = 0; k < n; k++)
      if (!isalnum((unsigned char)p[k]) && p[k] != '-' && p[k] != '_') return 0; }
  copia(dst, tam, p, n);
  return 1;
}

static int b64v(int c) {
  if (c >= 'A' && c <= 'Z') return c - 'A';
  if (c >= 'a' && c <= 'z') return c - 'a' + 26;
  if (c >= '0' && c <= '9') return c - '0' + 52;
  if (c == '-' || c == '+') return 62;
  if (c == '_' || c == '/') return 63;
  return -1;
}

int leitura_gn_antigo(const char *id, char *dst, size_t tam) {
  unsigned char bin[1024];
  size_t nb = 0, i;
  unsigned long acc = 0;
  int bits = 0;
  const char *h;
  if (!tam) return 0;
  dst[0] = 0;
  for (; *id && nb < sizeof bin; id++) {
    int v = b64v((unsigned char)*id);
    if (v < 0) break;
    acc = (acc << 6) | (unsigned long)v; bits += 6;
    if (bits >= 8) { bits -= 8; bin[nb++] = (unsigned char)((acc >> bits) & 0xFF); }
  }
  // O protobuf antigo e 08 13 22 <len> <url>; procurar "http" direto cobre
  // tambem as variantes de prefixo que ja apareceram.
  for (i = 0; i + 8 < nb; i++) {
    if (!memcmp(bin + i, "http://", 7) || !memcmp(bin + i, "https://", 8)) {
      size_t k = 0;
      while (i + k < nb && k + 1 < tam && bin[i + k] > 0x20 && bin[i + k] < 0x7F) {
        dst[k] = (char)bin[i + k]; k++;
      }
      dst[k] = 0;
      h = strstr(dst, "://");
      return h && strchr(h + 3, '.') ? 1 : 0;
    }
  }
  return 0;
}

static int valorAtributo(const char *html, const char *nome, char *dst, size_t tam) {
  const char *p = strstr(html, nome), *f;
  if (!tam) return 0;
  dst[0] = 0;
  if (!p) return 0;
  p += strlen(nome);
  if (*p != '=' || (p[1] != '"' && p[1] != '\'')) return 0;
  p += 2;
  f = strpbrk(p, "\"'");
  if (!f || (size_t)(f - p) >= tam || f == p) return 0;
  copia(dst, tam, p, (size_t)(f - p));
  return 1;
}

int leitura_gn_assinatura(const char *html, char *sg, size_t tamSg,
                          char *ts, size_t tamTs) {
  size_t k;
  if (!html) return 0;
  if (!valorAtributo(html, "data-n-a-sg", sg, tamSg)) return 0;
  if (!valorAtributo(html, "data-n-a-ts", ts, tamTs)) return 0;
  for (k = 0; ts[k]; k++) if (!isdigit((unsigned char)ts[k])) return 0;
  for (k = 0; sg[k]; k++)
    if (!isalnum((unsigned char)sg[k]) && sg[k] != '-' && sg[k] != '_') return 0;
  return 1;
}

static size_t pct(const char *s, char *dst, size_t tam) {
  size_t n = 0;
  for (; *s && n + 4 < tam; s++) {
    unsigned char c = (unsigned char)*s;
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') dst[n++] = (char)c;
    else { snprintf(dst + n, 4, "%%%02X", c); n += 3; }
  }
  dst[n] = 0;
  return n;
}

int leitura_gn_corpo(const char *id, const char *ts, const char *sg,
                     char *dst, size_t tam) {
  char req[1400];
  int n = snprintf(req, sizeof req,
    "[[[\"Fbv4je\",\"[\\\"garturlreq\\\",[[\\\"X\\\",\\\"X\\\",[\\\"X\\\",\\\"X\\\"],"
    "null,null,1,1,\\\"US:en\\\",null,1,null,null,null,null,null,0,1],\\\"X\\\","
    "\\\"X\\\",1,[1,1,1],1,1,null,0,0,null,0],\\\"%s\\\",%s,\\\"%s\\\"]\",null,"
    "\"generic\"]]]", id, ts, sg);
  if (n <= 0 || (size_t)n >= sizeof req || tam < 8) return 0;
  memcpy(dst, "f.req=", 6);
  pct(req, dst + 6, tam - 6);
  return strlen(dst) + 1 < tam;
}

int leitura_gn_resposta(const char *resp, char *dst, size_t tam) {
  const char *p = resp ? strstr(resp, "garturlres") : NULL;
  size_t n = 0;
  if (!tam) return 0;
  dst[0] = 0;
  if (!p) return 0;
  p = strstr(p, "http");
  if (!p) return 0;
  // A URL vem dentro de uma string JSON que esta dentro de outra: termina no
  // \" e traz =, & e \/ escapados.
  while (*p && n + 1 < tam) {
    if (p[0] == '\\' && p[1] == '"') break;
    if (p[0] == '"') break;
    if (p[0] == '\\' && p[1] == '\\' && p[2] == 'u') p++;   // \\u0026 duplo
    if (p[0] == '\\' && p[1] == 'u' && isxdigit((unsigned char)p[2])) {
      unsigned long c = strtoul((char[5]){ p[2], p[3], p[4], p[5], 0 }, NULL, 16);
      if (c > 0x20 && c < 0x7F) dst[n++] = (char)c;
      p += 6;
      continue;
    }
    if (p[0] == '\\' && p[1] == '/') { dst[n++] = '/'; p += 2; continue; }
    if (p[0] == '\\') { p++; continue; }
    if ((unsigned char)*p <= 0x20) break;
    dst[n++] = *p++;
  }
  dst[n] = 0;
  return n > 10 && (!strncmp(dst, "https://", 8) || !strncmp(dst, "http://", 7));
}

int leitura_url_qr(const char *url, char *dst, size_t tam) {
  size_t n = 0;
  if (!tam) return 0;
  dst[0] = 0;
  if (!url || (strncmp(url, "https://", 8) && strncmp(url, "http://", 7))) return 0;
  while (url[n] && url[n] != '?' && url[n] != '#') n++;
  if (n > 134 || n >= tam) return 0;   // o teto da versao 6 de qr.h
  copia(dst, tam, url, n);
  return 1;
}

void leitura_host(const char *url, char *dst, size_t tam) {
  const char *h = url ? strstr(url, "://") : NULL;
  size_t n = 0;
  if (!tam) return;
  dst[0] = 0;
  if (!h) return;
  h += 3;
  if (!strncmp(h, "www.", 4)) h += 4;
  while (h[n] && h[n] != '/' && h[n] != '?' && h[n] != ':' && h[n] != '#') n++;
  copia(dst, tam, h, n);
}

int leitura_link_direto(const char *url, char *dst, size_t tam) {
  const char *p;
  size_t n = 0;
  if (!tam) return 0;
  dst[0] = 0;
  if (!url || !strstr(url, "bing.com/news/apiclick")) return 0;
  p = strstr(url, "?url=");
  if (!p) p = strstr(url, "&url=");
  if (!p) p = strstr(url, "&amp;url=");
  if (!p) return 0;
  p = strstr(p, "url=") + 4;
  while (*p && *p != '&' && n + 1 < tam) {
    if (*p == '%' && isxdigit((unsigned char)p[1]) && isxdigit((unsigned char)p[2])) {
      char h[3] = { p[1], p[2], 0 };
      dst[n++] = (char)strtol(h, NULL, 16);
      p += 3;
    } else if (*p == '+') { dst[n++] = ' '; p++; }
    else dst[n++] = *p++;
  }
  dst[n] = 0;
  return !strncmp(dst, "https://", 8) || !strncmp(dst, "http://", 7);
}
