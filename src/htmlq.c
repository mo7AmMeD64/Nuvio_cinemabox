// Ver htmlq.h. Duas metades: o leitor de HTML (monta um vetor de nos em ordem
// do documento) e o seletor CSS (compila o texto e casa da direita para a
// esquerda, como os navegadores).
#include "htmlq.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

enum { NO_DOC = 0, NO_ELEM, NO_TEXTO, NO_COMENT };

typedef struct {
  int tipo;
  int pai, prox, ant, prim, ult;   // arvore crua (todos os tipos)
  int fimSub;                      // ultimo indice da subarvore
  int tag;                         // deslocamento no reservatorio (elemento)
  int attr0, nAttr;                // em d->attr
  int texto, nTexto;               // no reservatorio (texto, ja decodificado)
  int ini, fim;                    // HTML externo na fonte
  int iniDentro, fimDentro;        // HTML interno na fonte
} HqNo;

typedef struct { int nome, valor; } HqAttr;

struct HqDoc {
  char *src; int nSrc;
  HqNo *no; int nNo, capNo;
  HqAttr *attr; int nAttr, capAttr;
  char *pool; int nPool, capPool;
  size_t max;                      // teto de bytes do documento (0 = 64 MiB)
  unsigned long passos;            // trabalho do seletor corrente (mutavel)
  int estourou;                    // 1 = o seletor passou de HQ_PASSOS_MAX
  int semEspaco;                   // 1 = alguma alocacao da carga falhou/teto
};

// F09: teto de bytes CONFERIDO ANTES de cada realloc, para um HTML hostil nao
// crescer o documento alem do orcamento do runtime; e teto de trabalho do
// seletor, porque o casamento com descendente/irmao volta atras (exponencial
// num seletor patologico) e o interrupt do QuickJS nao roda dentro do C.
#define HQ_PASSOS_MAX   4000000UL
#define HQ_PROF_SEL     8          // :not(:has(:is(...))) aninhados
#define HQ_COMP_MAX     24         // compostos por seletor complexo
#define HQ_SEL_MAX      2048       // bytes do texto do seletor
static size_t hqUso(const HqDoc *d, size_t extraNo, size_t extraAttr, size_t extraPool) {
  return sizeof *d + (size_t)d->nSrc + 1 + sizeof(HqNo) * ((size_t)d->capNo + extraNo) +
         sizeof(HqAttr) * ((size_t)d->capAttr + extraAttr) + (size_t)d->capPool + extraPool;
}
static int hqCabe(const HqDoc *d, size_t extraNo, size_t extraAttr, size_t extraPool) {
  size_t max = d->max ? d->max : 64u * 1024 * 1024;
  if (hqUso(d, extraNo, extraAttr, extraPool) <= max) return 1;
  ((HqDoc *)d)->semEspaco = 1;
  return 0;
}

// ------------------------------------------------------------ reservatorio

static int poolPor(HqDoc *d, const char *s, int n) {
  int off;
  if (d->nPool + n + 1 > d->capPool) {
    int cap = d->capPool ? d->capPool : 4096;
    char *p;
    while (d->nPool + n + 1 > cap) { if (cap > (1 << 29)) return -1; cap *= 2; }
    if (!hqCabe(d, 0, 0, (size_t)(cap - d->capPool))) return -1;
    p = realloc(d->pool, (size_t)cap);
    if (!p) { d->semEspaco = 1; return -1; }
    d->pool = p; d->capPool = cap;
  }
  off = d->nPool;
  memcpy(d->pool + off, s, (size_t)n);
  d->pool[off + n] = 0;
  d->nPool += n + 1;
  return off;
}

// Entidades: as nomeadas que pagina de verdade usa e todas as numericas. O
// resto fica como veio, que e o que o navegador faz com nome desconhecido.
static const struct { const char *n; unsigned cp; } ENT[] = {
  {"amp",'&'},{"lt",'<'},{"gt",'>'},{"quot",'"'},{"apos",'\''},{"nbsp",0xA0},
  {"copy",0xA9},{"reg",0xAE},{"hellip",0x2026},{"ndash",0x2013},{"mdash",0x2014},
  {"lsquo",0x2018},{"rsquo",0x2019},{"ldquo",0x201C},{"rdquo",0x201D},
  {"laquo",0xAB},{"raquo",0xBB},{"times",0xD7},{"middot",0xB7},{"bull",0x2022},
  {"trade",0x2122},{"deg",0xB0},{"eacute",0xE9},{"aacute",0xE1},{"iacute",0xED},
  {"oacute",0xF3},{"uacute",0xFA},{"ccedil",0xE7},{"atilde",0xE3},{"otilde",0xF5},
  {NULL,0}};

static int utf8(unsigned cp, char *o) {
  if (cp < 0x80) { o[0] = (char)cp; return 1; }
  if (cp < 0x800) { o[0] = (char)(0xC0 | (cp >> 6)); o[1] = (char)(0x80 | (cp & 63)); return 2; }
  if (cp < 0x10000) { o[0] = (char)(0xE0 | (cp >> 12)); o[1] = (char)(0x80 | ((cp >> 6) & 63));
    o[2] = (char)(0x80 | (cp & 63)); return 3; }
  if (cp > 0x10FFFF) cp = 0xFFFD;
  o[0] = (char)(0xF0 | (cp >> 18)); o[1] = (char)(0x80 | ((cp >> 12) & 63));
  o[2] = (char)(0x80 | ((cp >> 6) & 63)); o[3] = (char)(0x80 | (cp & 63)); return 4;
}

// Decodifica [s, s+n) para o reservatorio. Devolve o deslocamento; *saiN o
// tamanho. Escreve no proprio lugar: entidade nunca fica maior que o texto.
static int poolDecod(HqDoc *d, const char *s, int n, int *saiN) {
  int off = poolPor(d, s, n), i = 0, k = 0;
  char *o;
  if (off < 0) { *saiN = 0; return -1; }
  o = d->pool + off;
  if (!memchr(s, '&', (size_t)n)) { *saiN = n; return off; }
  while (i < n) {
    if (s[i] == '&') {
      int j = i + 1; unsigned cp = 0; int ok = 0;
      if (j < n && s[j] == '#') {
        int hex = 0; j++;
        if (j < n && (s[j] == 'x' || s[j] == 'X')) { hex = 1; j++; }
        { int dig = 0;
          while (j < n && (hex ? isxdigit((unsigned char)s[j]) : isdigit((unsigned char)s[j])) && dig < 8) {
            cp = cp * (hex ? 16 : 10) + (unsigned)(isdigit((unsigned char)s[j]) ? s[j] - '0'
                 : (tolower((unsigned char)s[j]) - 'a' + 10));
            j++; dig++; }
          ok = dig > 0; }
        if (ok && j < n && s[j] == ';') j++;
        if (ok && cp == 0) cp = 0xFFFD;
      } else {
        int a = j, q;
        while (j < n && isalnum((unsigned char)s[j]) && j - a < 10) j++;
        for (q = 0; ENT[q].n; q++)
          if ((int)strlen(ENT[q].n) == j - a && !strncmp(ENT[q].n, s + a, (size_t)(j - a))) {
            cp = ENT[q].cp; ok = 1; break; }
        if (ok && j < n && s[j] == ';') j++;
        else if (ok && j - a < 2) ok = 0;
      }
      if (ok) { char u[4]; int m = utf8(cp, u);
        if (k + m <= j) {} /* sempre cabe: &x; tem >= 3 bytes, utf8 <= 4 */
        memcpy(o + k, u, (size_t)m); k += m; i = j; continue; }
    }
    o[k++] = s[i++];
  }
  o[k] = 0;
  *saiN = k;
  return off;
}

// ------------------------------------------------------------ arvore

static int novoNo(HqDoc *d, int tipo, int pai, int ini) {
  HqNo *n;
  if (d->nNo == d->capNo) {
    int cap = d->capNo ? d->capNo * 2 : 256;
    HqNo *p;
    if (d->capNo > (1 << 24) || !hqCabe(d, (size_t)(cap - d->capNo), 0, 0)) return -1;
    p = realloc(d->no, sizeof(HqNo) * (size_t)cap);
    if (!p) { d->semEspaco = 1; return -1; }
    d->no = p; d->capNo = cap;
  }
  n = &d->no[d->nNo];
  memset(n, 0, sizeof *n);
  n->tipo = tipo; n->pai = pai; n->prox = n->ant = n->prim = n->ult = -1;
  n->tag = n->texto = -1;
  n->ini = n->iniDentro = ini;
  n->fim = n->fimDentro = ini;
  if (pai >= 0) {
    HqNo *p = &d->no[pai];
    if (p->ult >= 0) { d->no[p->ult].prox = d->nNo; n->ant = p->ult; }
    else p->prim = d->nNo;
    p->ult = d->nNo;
  }
  return d->nNo++;
}

static const char *tagDe(const HqDoc *d, int i) {
  return d->no[i].tag >= 0 ? d->pool + d->no[i].tag : "";
}

static int naLista(const char *t, const char *const *l) {
  for (; *l; l++) if (!strcmp(t, *l)) return 1;
  return 0;
}
static const char *const VAZIAS[] = {"area","base","br","col","embed","hr","img","input",
  "link","meta","param","source","track","wbr","keygen",NULL};
static const char *const CRUAS[] = {"script","style","textarea","title","xmp",NULL};
static const char *const FECHA_P[] = {"address","article","aside","blockquote","details",
  "div","dl","fieldset","figcaption","figure","footer","form","h1","h2","h3","h4","h5","h6",
  "header","hgroup","hr","main","menu","nav","ol","p","pre","section","table","ul",NULL};
static const char *const LIMITE[] = {"html","body","table","td","th","caption","button",
  "object","template","applet","marquee",NULL};

#define PILHA_MAX 512
typedef struct { int el[PILHA_MAX]; int n; } Pilha;

// Fecha o elemento do topo em `pos` (inicio do que o fechou).
static void fecharTopo(HqDoc *d, Pilha *p, int dentroFim, int fim) {
  int i = p->el[--p->n];
  d->no[i].fimDentro = dentroFim;
  d->no[i].fim = fim;
  d->no[i].fimSub = d->nNo - 1;
}

// Procura `alvo` na pilha (do topo), parando em `paradas` e nos LIMITE.
static int acharAberto(const HqDoc *d, const Pilha *p, const char *const *alvos,
                       const char *const *paradas) {
  int k;
  for (k = p->n - 1; k >= 1; k--) {
    const char *t = tagDe(d, p->el[k]);
    if (naLista(t, alvos)) return k;
    if ((paradas && naLista(t, paradas)) || naLista(t, LIMITE)) return -1;
  }
  return -1;
}
static void fecharAte(HqDoc *d, Pilha *p, int k, int pos) {
  while (p->n > k) fecharTopo(d, p, pos, pos);
}

static void fechamentoImplicito(HqDoc *d, Pilha *p, const char *t, int pos) {
  static const char *const P[] = {"p",NULL}, *const LI[] = {"li",NULL},
    *const OL[] = {"ul","ol",NULL}, *const DTD[] = {"dt","dd",NULL}, *const DL[] = {"dl",NULL},
    *const TR[] = {"tr",NULL}, *const TAB[] = {"table","thead","tbody","tfoot",NULL},
    *const TD[] = {"td","th",NULL}, *const TRT[] = {"tr","table",NULL},
    *const SEC[] = {"thead","tbody","tfoot",NULL}, *const TBL[] = {"table",NULL},
    *const A[] = {"a",NULL}, *const OG[] = {"option","optgroup",NULL};
  int k;
  if (naLista(t, FECHA_P) && (k = acharAberto(d, p, P, NULL)) > 0) fecharAte(d, p, k, pos);
  if (!strcmp(t, "li") && (k = acharAberto(d, p, LI, OL)) > 0) fecharAte(d, p, k, pos);
  if ((!strcmp(t, "dt") || !strcmp(t, "dd")) && (k = acharAberto(d, p, DTD, DL)) > 0) fecharAte(d, p, k, pos);
  if (!strcmp(t, "tr") && (k = acharAberto(d, p, TR, TAB)) > 0) fecharAte(d, p, k, pos);
  if ((!strcmp(t, "td") || !strcmp(t, "th")) && (k = acharAberto(d, p, TD, TRT)) > 0) fecharAte(d, p, k, pos);
  if (naLista(t, SEC) && (k = acharAberto(d, p, SEC, TBL)) > 0) fecharAte(d, p, k, pos);
  if (!strcmp(t, "a") && (k = acharAberto(d, p, A, NULL)) > 0) fecharAte(d, p, k, pos);
  if (!strcmp(t, "option") && p->n > 1 && !strcmp(tagDe(d, p->el[p->n - 1]), "option"))
    fecharTopo(d, p, pos, pos);
  if (!strcmp(t, "optgroup")) while (p->n > 1 && naLista(tagDe(d, p->el[p->n - 1]), OG))
    fecharTopo(d, p, pos, pos);
}

static int ehNomeIni(char c) { return isalpha((unsigned char)c); }

static void textoEm(HqDoc *d, Pilha *p, int a, int b, int cru) {
  int i, n;
  if (b <= a) return;
  i = novoNo(d, NO_TEXTO, p->el[p->n - 1], a);
  if (i < 0) return;
  d->no[i].fim = d->no[i].fimDentro = b;
  if (cru) { d->no[i].texto = poolPor(d, d->src + a, b - a); n = b - a; }
  else d->no[i].texto = poolDecod(d, d->src + a, b - a, &n);
  d->no[i].nTexto = n;
  d->no[i].fimSub = i;
}

HqDoc *hq_carregar(const char *html, size_t tam) { return hq_carregar_max(html, tam, 0); }

HqDoc *hq_carregar_max(const char *html, size_t tam, size_t max) {
  HqDoc *d;
  if (!max || max > 64u * 1024 * 1024) max = 64u * 1024 * 1024;
  // A fonte e copiada inteira: recusar ANTES de alocar quando ja nao cabe.
  if (tam >= max - sizeof *d) return NULL;
  d = calloc(1, sizeof *d);
  Pilha p;
  const char *s;
  int n, i = 0, txt = 0;
  if (!d) return NULL;
  d->max = max;
  d->src = malloc(tam + 1);
  if (!d->src) { free(d); return NULL; }
  memcpy(d->src, html ? html : "", tam);
  d->src[tam] = 0;
  d->nSrc = n = (int)tam;
  s = d->src;
  if (novoNo(d, NO_DOC, -1, 0) < 0) { hq_soltar(d); return NULL; }
  d->no[0].fim = d->no[0].fimDentro = n;
  p.n = 1; p.el[0] = 0;
  while (i < n) {
    if (s[i] != '<') { i++; continue; }
    // comentario
    if (!strncmp(s + i, "<!--", 4)) {
      const char *f = strstr(s + i + 4, "-->");
      int fim = f ? (int)(f - s) + 3 : n, c;
      textoEm(d, &p, txt, i, 0);
      c = novoNo(d, NO_COMENT, p.el[p.n - 1], i);
      if (c >= 0) { d->no[c].fim = fim; d->no[c].fimSub = c; }
      i = txt = fim; continue;
    }
    // doctype, CDATA, <?xml
    if (s[i + 1] == '!' || s[i + 1] == '?') {
      const char *f = strchr(s + i, '>');
      textoEm(d, &p, txt, i, 0);
      i = txt = f ? (int)(f - s) + 1 : n; continue;
    }
    // fim de tag
    if (s[i + 1] == '/' && ehNomeIni(s[i + 2])) {
      char t[32]; int a = i + 2, k = 0, fim;
      const char *f;
      while (a < n && !isspace((unsigned char)s[a]) && s[a] != '>' && s[a] != '/') {
        if (k < 31) t[k++] = (char)tolower((unsigned char)s[a]);
        a++; }
      t[k] = 0;
      f = strchr(s + a, '>');
      fim = f ? (int)(f - s) + 1 : n;
      textoEm(d, &p, txt, i, 0);
      for (k = p.n - 1; k >= 1; k--) if (!strcmp(tagDe(d, p.el[k]), t)) break;
      if (k >= 1) {
        while (p.n > k + 1) fecharTopo(d, &p, i, i);
        fecharTopo(d, &p, i, fim);
      }
      i = txt = fim; continue;
    }
    if (!ehNomeIni(s[i + 1])) { i++; continue; }
    // inicio de tag
    { char t[32]; int a = i + 1, k = 0, e, autoF = 0, a0 = d->nAttr;
      textoEm(d, &p, txt, i, 0);
      while (a < n && !isspace((unsigned char)s[a]) && s[a] != '>' && s[a] != '/') {
        if (k < 31) t[k++] = (char)tolower((unsigned char)s[a]);
        a++; }
      t[k] = 0;
      // atributos
      for (;;) {
        int na, nb, va = -1, vb = -1;
        while (a < n && (isspace((unsigned char)s[a]) || s[a] == '/')) {
          if (s[a] == '/' && a + 1 < n && s[a + 1] == '>') autoF = 1;
          a++; }
        if (a >= n || s[a] == '>') break;
        na = a;
        while (a < n && !isspace((unsigned char)s[a]) && s[a] != '=' && s[a] != '>' &&
               !(s[a] == '/' && a + 1 < n && s[a + 1] == '>')) a++;
        nb = a;
        while (a < n && isspace((unsigned char)s[a])) a++;
        if (a < n && s[a] == '=') {
          a++;
          while (a < n && isspace((unsigned char)s[a])) a++;
          if (a < n && (s[a] == '"' || s[a] == '\'')) {
            char q = s[a++]; const char *f = memchr(s + a, q, (size_t)(n - a));
            va = a; vb = f ? (int)(f - s) : n; a = f ? vb + 1 : n;
          } else { va = a; while (a < n && !isspace((unsigned char)s[a]) && s[a] != '>') a++; vb = a; }
        }
        if (nb > na) {
          int q, dup = 0, lixo;
          char nome[64]; int m = nb - na < 63 ? nb - na : 63;
          for (q = 0; q < m; q++) nome[q] = (char)tolower((unsigned char)s[na + q]);
          nome[m] = 0;
          for (q = a0; q < d->nAttr; q++) if (!strcmp(d->pool + d->attr[q].nome, nome)) dup = 1;
          if (!dup) {
            if (d->nAttr == d->capAttr) {
              int cap = d->capAttr ? d->capAttr * 2 : 256;
              HqAttr *x = (d->capAttr > (1 << 24) || !hqCabe(d, 0, (size_t)(cap - d->capAttr), 0))
                          ? NULL : realloc(d->attr, sizeof(HqAttr) * (size_t)cap);
              if (!x) break;
              d->attr = x; d->capAttr = cap;
            }
            d->attr[d->nAttr].nome = poolPor(d, nome, m);
            d->attr[d->nAttr].valor = va >= 0 ? poolDecod(d, s + va, vb - va, &lixo) : poolPor(d, "", 0);
            d->nAttr++;
          }
        }
      }
      e = a < n ? a + 1 : n;   // depois do '>'
      fechamentoImplicito(d, &p, t, i);
      k = novoNo(d, NO_ELEM, p.el[p.n - 1], i);
      if (k < 0) break;
      d->no[k].tag = poolPor(d, t, (int)strlen(t));
      d->no[k].attr0 = a0; d->no[k].nAttr = d->nAttr - a0;
      d->no[k].iniDentro = d->no[k].fimDentro = e;
      d->no[k].fim = e; d->no[k].fimSub = k;
      i = txt = e;
      // "<x/>" so fecha em SVG/MathML; em HTML a barra e ignorada (como o
      // parse5 do cheerio e o navegador fazem).
      if (autoF && !naLista(t, VAZIAS)) { int q; autoF = 0;
        for (q = p.n - 1; q >= 1; q--)
          if (!strcmp(tagDe(d, p.el[q]), "svg") || !strcmp(tagDe(d, p.el[q]), "math")) { autoF = 1; break; }
        if (!strcmp(t, "svg") || !strcmp(t, "math")) autoF = 1; }
      if (naLista(t, VAZIAS) || autoF) continue;
      if (naLista(t, CRUAS)) {
        // texto cru ate </tag
        int j = e, L = (int)strlen(t), fimT;
        for (; j < n; j++)
          if (s[j] == '<' && s[j + 1] == '/' && !strncasecmp(s + j + 2, t, (size_t)L) &&
              (s[j + 2 + L] == '>' || isspace((unsigned char)s[j + 2 + L]) || !s[j + 2 + L])) break;
        if (p.n < PILHA_MAX) {
          p.el[p.n++] = k;
          textoEm(d, &p, e, j, strcmp(t, "textarea") && strcmp(t, "title"));
          { const char *f = j < n ? strchr(s + j, '>') : NULL;
            fimT = f ? (int)(f - s) + 1 : n; }
          fecharTopo(d, &p, j, fimT);
        }
        i = txt = j < n ? d->no[k].fim : n;
        continue;
      }
      if (p.n < PILHA_MAX) p.el[p.n++] = k;
    }
  }
  textoEm(d, &p, txt, n, 0);
  while (p.n > 1) fecharTopo(d, &p, n, n);
  d->no[0].fimSub = d->nNo - 1;
  if (d->semEspaco) { hq_soltar(d); return NULL; }   // parcial nao serve: o teto valeu
  return d;
}

void hq_soltar(HqDoc *d) {
  if (!d) return;
  free(d->src); free(d->no); free(d->attr); free(d->pool); free(d);
}
size_t hq_memoria(const HqDoc *d) {
  return d ? sizeof *d + (size_t)d->nSrc + 1 + sizeof(HqNo) * (size_t)d->capNo +
             sizeof(HqAttr) * (size_t)d->capAttr + (size_t)d->capPool : 0;
}
int hq_nos(const HqDoc *d) { return d ? d->nNo : 0; }

static int ehElem(const HqDoc *d, int i) {
  return d && i >= 0 && i < d->nNo && d->no[i].tipo == NO_ELEM;
}

const char *hq_attr(const HqDoc *d, int no, const char *nome) {
  int k;
  if (!ehElem(d, no) || !nome) return NULL;
  for (k = 0; k < d->no[no].nAttr; k++) {
    const HqAttr *a = &d->attr[d->no[no].attr0 + k];
    if (!strcasecmp(d->pool + a->nome, nome)) return d->pool + a->valor;
  }
  return NULL;
}
const char *hq_tag(const HqDoc *d, int no) { return ehElem(d, no) ? tagDe(d, no) : ""; }

int hq_pai(const HqDoc *d, int no) {
  int p;
  if (!d || no < 0 || no >= d->nNo) return -1;
  p = d->no[no].pai;
  return ehElem(d, p) ? p : -1;
}
int hq_prox(const HqDoc *d, int no) {
  int s;
  if (!d || no < 0 || no >= d->nNo) return -1;
  for (s = d->no[no].prox; s >= 0; s = d->no[s].prox) if (d->no[s].tipo == NO_ELEM) return s;
  return -1;
}
int hq_ant(const HqDoc *d, int no) {
  int s;
  if (!d || no < 0 || no >= d->nNo) return -1;
  for (s = d->no[no].ant; s >= 0; s = d->no[s].ant) if (d->no[s].tipo == NO_ELEM) return s;
  return -1;
}
int hq_filhos(const HqDoc *d, int no, int **saida) {
  int c, n = 0, *v;
  *saida = NULL;
  if (!d || no < -1 || no >= d->nNo) return 0;
  if (no < 0) no = 0;
  for (c = d->no[no].prim; c >= 0; c = d->no[c].prox) if (d->no[c].tipo == NO_ELEM) n++;
  if (!n) return 0;
  v = malloc(sizeof(int) * (size_t)n);
  if (!v) return 0;
  n = 0;
  for (c = d->no[no].prim; c >= 0; c = d->no[c].prox) if (d->no[c].tipo == NO_ELEM) v[n++] = c;
  *saida = v;
  return n;
}

// ------------------------------------------------------------ texto e html

typedef struct { char *p; size_t n, cap; } Buf;
static void bufPor(Buf *b, const char *s, size_t n) {
  if (b->n + n + 1 > b->cap) {
    size_t cap = b->cap ? b->cap : 256;
    char *x;
    while (b->n + n + 1 > cap) cap *= 2;
    x = realloc(b->p, cap);
    if (!x) return;
    b->p = x; b->cap = cap;
  }
  memcpy(b->p + b->n, s, n);
  b->n += n;
  b->p[b->n] = 0;
}

// Texto cru (todos os nos de texto da subarvore, comentario fora).
static void textoCru(const HqDoc *d, int no, Buf *b) {
  int i;
  for (i = no; i <= d->no[no].fimSub; i++)
    if (d->no[i].tipo == NO_TEXTO) bufPor(b, d->pool + d->no[i].texto, (size_t)d->no[i].nTexto);
}

char *hq_texto(const HqDoc *d, const int *nos, int n) {
  Buf b = {0}, out = {0};
  int k;
  bufPor(&out, "", 0);
  for (k = 0; d && k < n; k++) {
    size_t i; int espaco = 0, algo = 0;
    if (nos[k] < 0 || nos[k] >= d->nNo) continue;
    b.n = 0;
    bufPor(&b, "", 0);
    textoCru(d, nos[k], &b);
    if (!b.p) continue;
    for (i = 0; i < b.n; i++) {
      unsigned char c = (unsigned char)b.p[i];
      // \s do JS (o que o web usa para imitar o Jsoup) inclui o nbsp.
      if (c == 0xC2 && i + 1 < b.n && (unsigned char)b.p[i + 1] == 0xA0) { i++; espaco = 1; continue; }
      if (isspace(c)) { espaco = 1; continue; }
      if (espaco && algo) bufPor(&out, " ", 1);
      else if (!algo && out.n) bufPor(&out, " ", 1);
      espaco = 0; algo = 1;
      bufPor(&out, (const char *)&b.p[i], 1);
    }
  }
  free(b.p);
  return out.p ? out.p : strdup("");
}

char *hq_html(const HqDoc *d, int no, int externo) {
  int a, b;
  char *r;
  if (!d) return strdup("");
  if (no < 0) { a = 0; b = d->nSrc; }
  else if (no >= d->nNo) return strdup("");
  else if (externo) { a = d->no[no].ini; b = d->no[no].fim; }
  else { a = d->no[no].iniDentro; b = d->no[no].fimDentro; }
  if (b < a) b = a;
  r = malloc((size_t)(b - a) + 1);
  if (!r) return NULL;
  memcpy(r, d->src + a, (size_t)(b - a));
  r[b - a] = 0;
  return r;
}

// ------------------------------------------------------------ seletor

typedef struct HqSel HqSel;
enum { S_ID, S_CLASSE, S_ATTR, S_PSEUDO };
enum { OP_TEM, OP_IGUAL, OP_PALAVRA, OP_TRACO, OP_COMECA, OP_TERMINA, OP_CONTEM, OP_DIF };
enum { P_PRIMEIRO, P_ULTIMO, P_UNICO, P_NTH, P_NTH_ULT, P_PRIM_TIPO, P_ULT_TIPO,
       P_NTH_TIPO, P_NTH_ULT_TIPO, P_UNICO_TIPO, P_VAZIO, P_RAIZ, P_NOT, P_HAS,
       P_CONTEM, P_CHECKED, P_IS };
typedef struct {
  int tipo, op, ci, pseudo, a, b;
  char *nome, *valor;
  HqSel *sub;
} Simp;
typedef struct { char *tag; Simp *s; int n; int comb; } Comp;
typedef struct { Comp *c; int n; int relativo; } Cx;
struct HqSel { Cx *cx; int n; };

static void selSoltar(HqSel *s);
static void compSoltar(Comp *c) {
  int k;
  free(c->tag);
  for (k = 0; k < c->n; k++) { free(c->s[k].nome); free(c->s[k].valor); selSoltar(c->s[k].sub); }
  free(c->s);
}
static void selSoltar(HqSel *s) {
  int i, k;
  if (!s) return;
  for (i = 0; i < s->n; i++) {
    for (k = 0; k < s->cx[i].n; k++) compSoltar(&s->cx[i].c[k]);
    free(s->cx[i].c);
  }
  free(s->cx); free(s);
}

typedef struct { const char *p; int erro, prof; } Ler;
static void brancos(Ler *l) { while (*l->p && isspace((unsigned char)*l->p)) l->p++; }
static int ehIdent(char c) {
  return isalnum((unsigned char)c) || c == '-' || c == '_' || (unsigned char)c >= 0x80;
}
static char *ident(Ler *l) {
  char buf[256]; int k = 0;
  while (*l->p && (ehIdent(*l->p) || *l->p == '\\')) {
    if (*l->p == '\\' && l->p[1]) l->p++;
    if (k < 255) buf[k++] = *l->p;
    l->p++;
  }
  if (!k) { l->erro = 1; return NULL; }
  buf[k] = 0;
  return strdup(buf);
}
// Valor: entre aspas ou identificador nu.
static char *valorLido(Ler *l, const char *fins) {
  char buf[512]; int k = 0;
  brancos(l);
  if (*l->p == '"' || *l->p == '\'') {
    char q = *l->p++;
    while (*l->p && *l->p != q) {
      if (*l->p == '\\' && l->p[1]) l->p++;
      if (k < 511) buf[k++] = *l->p;
      l->p++;
    }
    if (*l->p != q) { l->erro = 1; return NULL; }
    l->p++;
  } else {
    while (*l->p && !strchr(fins, *l->p)) { if (k < 511) buf[k++] = *l->p; l->p++; }
    while (k > 0 && isspace((unsigned char)buf[k - 1])) k--;
  }
  buf[k] = 0;
  return strdup(buf);
}

static HqSel *lerLista(Ler *l, char fim);

static int addSimp(Comp *c, Simp s) {
  Simp *x = realloc(c->s, sizeof(Simp) * (size_t)(c->n + 1));
  if (!x) return 0;
  c->s = x; c->s[c->n++] = s;
  return 1;
}

// an+b. "odd", "even", "3", "2n+1", "-n+3", "n".
static int lerNth(const char *t, int *a, int *b) {
  char s[64]; int k = 0; const char *n;
  for (; *t && k < 63; t++) if (!isspace((unsigned char)*t)) s[k++] = (char)tolower((unsigned char)*t);
  s[k] = 0;
  if (!strcmp(s, "odd")) { *a = 2; *b = 1; return 1; }
  if (!strcmp(s, "even")) { *a = 2; *b = 0; return 1; }
  n = strchr(s, 'n');
  if (!n) { char *e; *a = 0; *b = (int)strtol(s, &e, 10); return *e == 0 && k > 0; }
  if (n == s) *a = 1;
  else if (n == s + 1 && s[0] == '-') *a = -1;
  else if (n == s + 1 && s[0] == '+') *a = 1;
  else { char *e; *a = (int)strtol(s, &e, 10); if (e != n) return 0; }
  if (!n[1]) { *b = 0; return 1; }
  { char *e; *b = (int)strtol(n + 1, &e, 10); return *e == 0; }
}

static int lerComp(Ler *l, Comp *c) {
  int algo = 0;
  memset(c, 0, sizeof *c);
  if (*l->p == '*') { l->p++; algo = 1; }
  else if (ehIdent(*l->p)) {
    char *t = ident(l); int k;
    if (!t) return 0;
    for (k = 0; t[k]; k++) t[k] = (char)tolower((unsigned char)t[k]);
    c->tag = t; algo = 1;
  }
  for (;;) {
    Simp s; memset(&s, 0, sizeof s);
    if (*l->p == '#') { l->p++; s.tipo = S_ID; s.valor = ident(l); }
    else if (*l->p == '.') { l->p++; s.tipo = S_CLASSE; s.valor = ident(l); }
    else if (*l->p == '[') {
      int k;
      l->p++; brancos(l);
      s.tipo = S_ATTR; s.nome = ident(l);
      if (s.nome) for (k = 0; s.nome[k]; k++) s.nome[k] = (char)tolower((unsigned char)s.nome[k]);
      brancos(l);
      if (*l->p == ']') { s.op = OP_TEM; l->p++; }
      else {
        if (*l->p == '=') { s.op = OP_IGUAL; l->p++; }
        else if (*l->p && strchr("~|^$*!", *l->p) && l->p[1] == '=') {
          char o = *l->p; l->p += 2;
          s.op = o == '~' ? OP_PALAVRA : o == '|' ? OP_TRACO : o == '^' ? OP_COMECA :
                 o == '$' ? OP_TERMINA : o == '*' ? OP_CONTEM : OP_DIF;
        } else l->erro = 1;
        if (!l->erro) {
          s.valor = valorLido(l, " \t]");
          brancos(l);
          if (*l->p == 'i' || *l->p == 'I') { s.ci = 1; l->p++; brancos(l); }
          else if (*l->p == 's' || *l->p == 'S') { l->p++; brancos(l); }
          if (*l->p == ']') l->p++; else l->erro = 1;
        }
      }
    } else if (*l->p == ':') {
      char *nome;
      l->p++;
      if (*l->p == ':') l->p++;
      nome = ident(l);
      if (!nome) return 0;
      s.tipo = S_PSEUDO;
      if (!strcmp(nome, "first-child")) s.pseudo = P_PRIMEIRO;
      else if (!strcmp(nome, "last-child")) s.pseudo = P_ULTIMO;
      else if (!strcmp(nome, "only-child")) s.pseudo = P_UNICO;
      else if (!strcmp(nome, "first-of-type")) s.pseudo = P_PRIM_TIPO;
      else if (!strcmp(nome, "last-of-type")) s.pseudo = P_ULT_TIPO;
      else if (!strcmp(nome, "only-of-type")) s.pseudo = P_UNICO_TIPO;
      else if (!strcmp(nome, "empty")) s.pseudo = P_VAZIO;
      else if (!strcmp(nome, "root")) s.pseudo = P_RAIZ;
      else if (!strcmp(nome, "checked")) s.pseudo = P_CHECKED;
      else if (!strcmp(nome, "nth-child") || !strcmp(nome, "nth-last-child") ||
               !strcmp(nome, "nth-of-type") || !strcmp(nome, "nth-last-of-type")) {
        char *arg;
        s.pseudo = !strcmp(nome, "nth-child") ? P_NTH : !strcmp(nome, "nth-last-child") ? P_NTH_ULT :
                   !strcmp(nome, "nth-of-type") ? P_NTH_TIPO : P_NTH_ULT_TIPO;
        if (*l->p != '(') { free(nome); l->erro = 1; return 0; }
        l->p++;
        arg = valorLido(l, ")");
        if (!arg || !lerNth(arg, &s.a, &s.b)) l->erro = 1;
        free(arg);
        if (*l->p == ')') l->p++; else l->erro = 1;
      } else if (!strcmp(nome, "not") || !strcmp(nome, "has") || !strcmp(nome, "is") ||
                 !strcmp(nome, "matches") || !strcmp(nome, "where")) {
        s.pseudo = !strcmp(nome, "not") ? P_NOT : !strcmp(nome, "has") ? P_HAS : P_IS;
        if (*l->p != '(') { free(nome); l->erro = 1; return 0; }
        l->p++;
        s.sub = lerLista(l, ')');
        // :has(x) e relativo ao proprio elemento: "x" quer dizer descendente.
        if (s.sub && s.pseudo == P_HAS) { int q;
          for (q = 0; q < s.sub->n; q++)
            if (!s.sub->cx[q].relativo && s.sub->cx[q].n) {
              s.sub->cx[q].relativo = 1; s.sub->cx[q].c[0].comb = ' '; } }
        if (*l->p == ')') l->p++; else l->erro = 1;
      } else if (!strcmp(nome, "contains")) {
        s.pseudo = P_CONTEM;
        if (*l->p != '(') { free(nome); l->erro = 1; return 0; }
        l->p++;
        s.valor = valorLido(l, ")");
        brancos(l);
        if (*l->p == ')') l->p++; else l->erro = 1;
      } else l->erro = 1;
      free(nome);
    } else break;
    if (l->erro) { free(s.nome); free(s.valor); selSoltar(s.sub); return 0; }
    if (!addSimp(c, s)) return 0;
    algo = 1;
  }
  if (!algo) l->erro = 1;
  return !l->erro;
}

static int lerCx(Ler *l, Cx *cx, char fim) {
  int comb = 0;
  memset(cx, 0, sizeof *cx);
  brancos(l);
  if (*l->p == '>' || *l->p == '+' || *l->p == '~') { comb = *l->p++; cx->relativo = 1; brancos(l); }
  for (;;) {
    Comp c, *x;
    if (cx->n >= HQ_COMP_MAX) { l->erro = 1; return 0; }
    if (!lerComp(l, &c)) { compSoltar(&c); return 0; }
    c.comb = cx->n == 0 ? (cx->relativo ? comb : 0) : comb;
    x = realloc(cx->c, sizeof(Comp) * (size_t)(cx->n + 1));
    if (!x) { compSoltar(&c); return 0; }
    cx->c = x; cx->c[cx->n++] = c;
    { const char *antes = l->p;
      brancos(l);
      if (!*l->p || *l->p == ',' || *l->p == fim) return 1;
      if (*l->p == '>' || *l->p == '+' || *l->p == '~') { comb = *l->p++; brancos(l); }
      else if (l->p != antes) comb = ' ';
      else { l->erro = 1; return 0; } }
  }
}

static HqSel *lerLista(Ler *l, char fim) {
  HqSel *s;
  if (++l->prof > HQ_PROF_SEL) { l->erro = 1; return NULL; }
  s = calloc(1, sizeof *s);
  if (!s) { l->erro = 1; return NULL; }
  for (;;) {
    Cx cx, *x;
    if (!lerCx(l, &cx, fim)) {
      int k; for (k = 0; k < cx.n; k++) compSoltar(&cx.c[k]);
      free(cx.c); l->erro = 1; return s; }
    x = realloc(s->cx, sizeof(Cx) * (size_t)(s->n + 1));
    if (!x) { int k; for (k = 0; k < cx.n; k++) compSoltar(&cx.c[k]); free(cx.c); l->erro = 1; return s; }
    s->cx = x; s->cx[s->n++] = cx;
    brancos(l);
    if (*l->p == ',') { l->p++; continue; }
    l->prof--;
    return s;
  }
}

static HqSel *compilar(const char *t) {
  Ler l = { t ? t : "", 0, 0 };
  HqSel *s;
  if (strlen(l.p) > HQ_SEL_MAX) return NULL;
  brancos(&l);
  if (!*l.p) return NULL;
  s = lerLista(&l, 0);
  brancos(&l);
  if (l.erro || *l.p) { selSoltar(s); return NULL; }
  return s;
}

// ------------------------------------------------------------ casamento

static int casaLista(const HqDoc *d, const HqSel *s, int e, int escopo);

static int palavraEm(const char *lista, const char *w, int ci) {
  size_t n = strlen(w);
  const char *p = lista;
  if (!n) return 0;
  while (*p) {
    while (*p && isspace((unsigned char)*p)) p++;
    { const char *a = p;
      while (*p && !isspace((unsigned char)*p)) p++;
      if ((size_t)(p - a) == n && (ci ? !strncasecmp(a, w, n) : !strncmp(a, w, n))) return 1; }
  }
  return 0;
}

static const char *achar(const char *h, const char *a, int ci) {
  size_t n = strlen(a);
  if (!ci) return strstr(h, a);
  for (; *h; h++) if (!strncasecmp(h, a, n)) return h;
  return NULL;
}

// Posicao (1-based) entre os irmaos elemento; `mesmoTipo` conta so a tag.
static void posicao(const HqDoc *d, int e, int mesmoTipo, int *pos, int *tot) {
  int p = d->no[e].pai, c, k = 0;
  *pos = 0; *tot = 0;
  if (p < 0) { *pos = *tot = 1; return; }
  for (c = d->no[p].prim; c >= 0; c = d->no[c].prox) {
    if (d->no[c].tipo != NO_ELEM) continue;
    if (mesmoTipo && strcmp(tagDe(d, c), tagDe(d, e))) continue;
    k++;
    if (c == e) *pos = k;
  }
  *tot = k;
}
static int nthOk(int a, int b, int pos) {
  if (a == 0) return pos == b;
  return (pos - b) % a == 0 && (pos - b) / a >= 0;
}

static int casaSimp(const HqDoc *d, const Simp *s, int e) {
  const char *v;
  switch (s->tipo) {
  case S_ID: v = hq_attr(d, e, "id"); return v && s->valor && !strcmp(v, s->valor);
  case S_CLASSE: v = hq_attr(d, e, "class"); return v && s->valor && palavraEm(v, s->valor, 0);
  case S_ATTR: {
    const char *w = s->valor ? s->valor : "";
    size_t nv, nw = strlen(w);
    v = hq_attr(d, e, s->nome ? s->nome : "");
    if (s->op == OP_DIF) return !v || (s->ci ? strcasecmp(v, w) : strcmp(v, w));
    if (!v) return 0;
    nv = strlen(v);
    switch (s->op) {
    case OP_TEM: return 1;
    case OP_IGUAL: return s->ci ? !strcasecmp(v, w) : !strcmp(v, w);
    case OP_PALAVRA: return palavraEm(v, w, s->ci);
    case OP_TRACO: return (s->ci ? !strncasecmp(v, w, nw) : !strncmp(v, w, nw)) && (v[nw] == 0 || v[nw] == '-');
    case OP_COMECA: return nw && (s->ci ? !strncasecmp(v, w, nw) : !strncmp(v, w, nw));
    case OP_TERMINA: return nw && nv >= nw && (s->ci ? !strcasecmp(v + nv - nw, w) : !strcmp(v + nv - nw, w));
    case OP_CONTEM: return nw && achar(v, w, s->ci) != NULL;
    }
    return 0; }
  case S_PSEUDO: {
    int pos, tot;
    switch (s->pseudo) {
    case P_PRIMEIRO: posicao(d, e, 0, &pos, &tot); return pos == 1;
    case P_ULTIMO: posicao(d, e, 0, &pos, &tot); return pos == tot;
    case P_UNICO: posicao(d, e, 0, &pos, &tot); return tot == 1;
    case P_PRIM_TIPO: posicao(d, e, 1, &pos, &tot); return pos == 1;
    case P_ULT_TIPO: posicao(d, e, 1, &pos, &tot); return pos == tot;
    case P_UNICO_TIPO: posicao(d, e, 1, &pos, &tot); return tot == 1;
    case P_NTH: posicao(d, e, 0, &pos, &tot); return nthOk(s->a, s->b, pos);
    case P_NTH_ULT: posicao(d, e, 0, &pos, &tot); return nthOk(s->a, s->b, tot - pos + 1);
    case P_NTH_TIPO: posicao(d, e, 1, &pos, &tot); return nthOk(s->a, s->b, pos);
    case P_NTH_ULT_TIPO: posicao(d, e, 1, &pos, &tot); return nthOk(s->a, s->b, tot - pos + 1);
    case P_VAZIO: { int c;
      for (c = d->no[e].prim; c >= 0; c = d->no[c].prox)
        if (d->no[c].tipo == NO_ELEM || (d->no[c].tipo == NO_TEXTO && d->no[c].nTexto)) return 0;
      return 1; }
    case P_RAIZ: return hq_pai(d, e) < 0;
    case P_CHECKED: return hq_attr(d, e, "checked") != NULL || hq_attr(d, e, "selected") != NULL;
    case P_NOT: return s->sub && !casaLista(d, s->sub, e, -1);
    case P_IS: return s->sub && casaLista(d, s->sub, e, -1);
    case P_HAS: { int i;
      if (!s->sub) return 0;
      for (i = e + 1; i <= d->no[e].fimSub; i++)
        if (d->no[i].tipo == NO_ELEM && casaLista(d, s->sub, i, e)) return 1;
      // :has(+ x) / :has(~ x): irmaos seguintes
      for (i = hq_prox(d, e); i >= 0; i = hq_prox(d, i))
        if (casaLista(d, s->sub, i, e)) return 1;
      return 0; }
    case P_CONTEM: { Buf b = {0}; int r;
      if (!s->valor) return 0;
      bufPor(&b, "", 0);
      textoCru(d, e, &b);
      r = b.p && strstr(b.p, s->valor) != NULL;
      free(b.p);
      return r; }
    }
    return 0; }
  }
  return 0;
}

static int casaComp(const HqDoc *d, const Comp *c, int e) {
  int k;
  if (!ehElem(d, e)) return 0;
  if (c->tag && strcmp(c->tag, tagDe(d, e))) return 0;
  for (k = 0; k < c->n; k++) if (!casaSimp(d, &c->s[k], e)) return 0;
  return 1;
}

static int ehAncestral(const HqDoc *d, int a, int e) {
  return a >= 0 && e > a && e <= d->no[a].fimSub;
}

static int casaDe(const HqDoc *d, const Cx *cx, int idx, int e, int escopo) {
  int comb, p;
  HqDoc *m = (HqDoc *)d;   // so o contador de trabalho muda
  if (m->estourou || ++m->passos > HQ_PASSOS_MAX) { m->estourou = 1; return 0; }
  if (!casaComp(d, &cx->c[idx], e)) return 0;
  comb = cx->c[idx].comb;
  if (idx == 0) {
    if (!cx->relativo || escopo < 0) return 1;
    switch (comb) {
    case '>': return d->no[e].pai == escopo;
    case '+': return hq_ant(d, e) == escopo;
    case '~': for (p = hq_ant(d, e); p >= 0; p = hq_ant(d, p)) if (p == escopo) return 1; return 0;
    default: return ehAncestral(d, escopo, e);
    }
  }
  switch (comb) {
  case '>': p = hq_pai(d, e); return p >= 0 && casaDe(d, cx, idx - 1, p, escopo);
  case '+': p = hq_ant(d, e); return p >= 0 && casaDe(d, cx, idx - 1, p, escopo);
  case '~':
    for (p = hq_ant(d, e); p >= 0; p = hq_ant(d, p)) if (casaDe(d, cx, idx - 1, p, escopo)) return 1;
    return 0;
  default:
    for (p = hq_pai(d, e); p >= 0; p = hq_pai(d, p)) if (casaDe(d, cx, idx - 1, p, escopo)) return 1;
    return 0;
  }
}

static int casaLista(const HqDoc *d, const HqSel *s, int e, int escopo) {
  int i;
  for (i = 0; i < s->n; i++) if (casaDe(d, &s->cx[i], s->cx[i].n - 1, e, escopo)) return 1;
  return 0;
}

int hq_selecionar(const HqDoc *d, int ctx, const char *seletor, int **saida) {
  HqSel *s;
  int i, ini, fim, n = 0, cap = 0, *v = NULL;
  *saida = NULL;
  if (!d) return 0;
  s = compilar(seletor);
  if (!s) return -1;
  ((HqDoc *)d)->passos = 0; ((HqDoc *)d)->estourou = 0;
  if (ctx < 0 || ctx >= d->nNo) ctx = 0;
  ini = ctx + 1; fim = d->no[ctx].fimSub;
  for (i = ini; i <= fim; i++) {
    if (d->no[i].tipo != NO_ELEM || !casaLista(d, s, i, ctx)) continue;
    if (n == cap) { int *x; cap = cap ? cap * 2 : 16;
      x = realloc(v, sizeof(int) * (size_t)cap);
      if (!x) break;
      v = x; }
    v[n++] = i;
  }
  selSoltar(s);
  if (d->estourou) { free(v); return -1; }   // trabalho demais: lista vazia
  *saida = v;
  return n;
}

int hq_casa(const HqDoc *d, int no, const char *seletor) {
  HqSel *s;
  int r;
  if (!ehElem(d, no)) return 0;
  s = compilar(seletor);
  if (!s) return -1;
  ((HqDoc *)d)->passos = 0; ((HqDoc *)d)->estourou = 0;
  r = casaLista(d, s, no, -1);
  selSoltar(s);
  return d->estourou ? -1 : r;
}
