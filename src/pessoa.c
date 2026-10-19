#include "pessoa.h"
#include "descoberta.h"
#include "ajustes.h"
#include "idbase.h"
#include "rede.h"
#include "js.h"
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define TMDB "https://api.themoviedb.org/3"

// Nomeada porque a ordenacao por popularidade precisa de uma variavel
// temporaria do mesmo tipo, e struct anonima nao permite declarar outra.
typedef struct {
  char t[120], p[80], y[8], po[160], im[16];
  // O credito de combined_credits NAO tem imdb_id — esse campo so existe no
  // endpoint detalhado do titulo. Sem guardar o id do TMDB e o tipo, nao havia
  // como traduzir o credito em nada que o Cinemeta entenda, e o OK nao fazia
  // coisa nenhuma (o `im` ficava sempre vazio).
  long tmdb;
  char tipo[8];          // "movie" ou "tv"
  double pop;
} Credito;
static Credito cred[PES_MAX];
static int nCred;

static char nome[80], foto[200], bio[1400], area[24];
static long idPedido;
static unsigned geracao;
typedef struct {
  long id;
  unsigned geracao;
  char nome[80], foto[200], imdb[24], tipo[16];
} Pedido;
static char nomeSemente[80], fotoSemente[200];
static char ctxImdb[24], ctxTipo[16];
static int  pronta, fioVivo, falhou;
static pthread_t fio;
static pthread_mutex_t trava = PTHREAD_MUTEX_INITIALIZER;

void pessoa_contexto_titulo(const char *imdb, const char *tipo) {
  pthread_mutex_lock(&trava);
  snprintf(ctxImdb, sizeof ctxImdb, "%s", imdb ? imdb : "");
  snprintf(ctxTipo, sizeof ctxTipo, "%s", tipo ? tipo : "");
  pthread_mutex_unlock(&trava);
}

static void codificar(const char *s, char *dst, size_t cap) {
  size_t z = 0;
  for (const unsigned char *c = (const unsigned char *)s; *c && z + 4 < cap; c++) {
    if ((*c >= 'a' && *c <= 'z') || (*c >= 'A' && *c <= 'Z') || (*c >= '0' && *c <= '9'))
      dst[z++] = *c;
    else if (*c == ' ') dst[z++] = '+';
    else { snprintf(dst + z, 4, "%%%02X", *c); z += 3; }
  }
  dst[z] = 0;
}

static char normDobra(unsigned char c) {
  switch (c) {
    case 0xA0: case 0xA1: return 'a'; case 0xA2: return 'a'; case 0xA3: return 'a';
    case 0xA7: return 'c'; case 0xA9: return 'e'; case 0xAA: return 'e';
    case 0xAD: return 'i'; case 0xB3: return 'o'; case 0xB5: return 'o';
    case 0xBA: return 'u'; default: return 0;
  }
}

static void normNome(const char *s, char *dst, size_t tam) {
  const unsigned char *p = (const unsigned char *)(s ? s : "");
  size_t k = 0;
  while (*p && k + 1 < tam) {
    unsigned char c = *p++;
    char o = 0;
    if (c >= 'A' && c <= 'Z') o = (char)(c + 32);
    else if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) o = (char)c;
    else if (c == 0xC3 && *p && (o = normDobra(*p))) p++;
    else if (c >= 0x80) { dst[k++] = (char)c; continue; }
    if (o) dst[k++] = o;
  }
  dst[k] = 0;
}

static int mesmoNome(const char *a, const char *b) {
  char na[64], nb[64];
  if (!a[0] || !b[0]) return 0;
  if (!strcmp(a, b)) return 1;
  normNome(a, na, sizeof na);
  normNome(b, nb, sizeof nb);
  return na[0] && !strcmp(na, nb);
}

static long resolverId(const char *nomeAlvo, const char *imdb, const char *tipo) {
  const char *chave = desc_chave_tmdb();
  char url[700], enc[300], *corpo = NULL;
  long idPessoa = 0, idTitulo = 0;
  int serie = !strcmp(tipo, "series");
  if (!chave || !chave[0] || !ajustes_tmdb_elenco() || !nomeAlvo[0]) return 0;
  if (idbase_e_imdb(imdb)) {
    snprintf(url, sizeof url, "%s/find/%s?api_key=%s&external_source=imdb_id",
             TMDB, imdb, chave);
    corpo = rede_baixar(url, 20);
    if (corpo) {
      const char *p = js_array(corpo, NULL, serie ? "tv_results" : "movie_results");
      if (p) idTitulo = (long)js_num(p, js_fim(p), "id", 0);
      free(corpo);
    }
  }
  if (idTitulo > 0) {
    snprintf(url, sizeof url, "%s/%s/%ld/credits?api_key=%s&language=%s",
             TMDB, serie ? "tv" : "movie", idTitulo, chave, desc_tmdb_idioma());
    corpo = rede_baixar(url, 20);
    if (corpo) {
      const char *p = js_array(corpo, NULL, "cast");
      while (p && !idPessoa) {
        const char *f = js_fim(p);
        char nome[96] = "";
        js_texto(p, f, "name", nome, sizeof nome);
        if (mesmoNome(nome, nomeAlvo)) idPessoa = (long)js_num(p, f, "id", 0);
        p = js_prox(f);
      }
      free(corpo);
    }
  }
  if (!idPessoa) {
    codificar(nomeAlvo, enc, sizeof enc);
    snprintf(url, sizeof url, "%s/search/person?api_key=%s&language=%s&query=%s",
             TMDB, chave, desc_tmdb_idioma(), enc);
    corpo = rede_baixar(url, 20);
    if (corpo) {
      const char *r = js_array(corpo, NULL, "results");
      int correspondencias = 0;
      while (r) {
        const char *fr = js_fim(r);
        char nome[96] = "";
        js_texto(r, fr, "name", nome, sizeof nome);
        if (mesmoNome(nome, nomeAlvo)) {
          if (++correspondencias > 1) { idPessoa = 0; break; }
          idPessoa = (long)js_num(r, fr, "id", 0);
        }
        r = js_prox(fr);
      }
      free(corpo);
    }
  }
  return idPessoa;
}

// known_for_department vem em ingles do TMDB mesmo com language=pt-BR.
static const char *traduzArea(const char *s) {
  if (!strcmp(s, "Acting"))    return "Atua\xc3\xa7\xc3\xa3" "o";
  if (!strcmp(s, "Directing")) return "Dire\xc3\xa7\xc3\xa3" "o";
  if (!strcmp(s, "Writing"))   return "Roteiro";
  if (!strcmp(s, "Production"))return "Produ\xc3\xa7\xc3\xa3" "o";
  if (!strcmp(s, "Sound"))     return "Som";
  if (!strcmp(s, "Camera"))    return "Fotografia";
  return s;
}

static void buscarPedido(const Pedido *pedido) {
  char url[400], *corpo;
  long id = pedido->id;
  const char *chave = desc_chave_tmdb();

  if (!chave || !chave[0]) return;
  if (id <= 0)
    id = resolverId(pedido->nome, pedido->imdb, pedido->tipo);
  if (id <= 0) return;
  pthread_mutex_lock(&trava);
  int atual = pedido->geracao == geracao;
  pthread_mutex_unlock(&trava);
  if (!atual) return;

  // O MESMO pedido do web (castDetailScreen.js:204): a ficha e os creditos
  // combinados numa chamada so. Separado seriam dois pedidos para desenhar uma
  // tela — e o segundo so serve se o primeiro deu certo.
  snprintf(url, sizeof url,
           "%s/person/%ld?api_key=%s&language=%s"
           "&append_to_response=combined_credits", TMDB, id, chave, desc_tmdb_idioma());
  corpo = rede_baixar(url, 20);
  if (!corpo) return;

  {
    char n[80] = "", f[160] = "", b[1400] = "", a[24] = "";
    Credito ach[PES_MAX];
    int k = 0;

    js_texto(corpo, NULL, "name", n, sizeof n);
    js_texto(corpo, NULL, "biography", b, sizeof b);
    js_texto(corpo, NULL, "known_for_department", a, sizeof a);
    { char caminho[128] = "";
      if (js_texto(corpo, NULL, "profile_path", caminho, sizeof caminho) &&
          caminho[0] == '/')
        snprintf(f, sizeof f, "https://image.tmdb.org/t/p/w342%s", caminho); }

    // `combined_credits.cast` — o array vem depois da chave, e js_array acha o
    // primeiro "cast" do documento. O objeto raiz nao tem outro "cast", entao
    // e este.
    { const char *p = js_array(corpo, NULL, "cast");
      while (p && k < PES_MAX) {
        const char *fim = js_fim(p);
        char data[16] = "", caminho[128] = "";
        ach[k].t[0] = ach[k].p[0] = ach[k].y[0] = ach[k].po[0] = ach[k].im[0] = 0;
        // Filme tem "title"/"release_date"; serie tem "name"/"first_air_date".
        if (!js_texto(p, fim, "title", ach[k].t, sizeof ach[k].t))
          js_texto(p, fim, "name", ach[k].t, sizeof ach[k].t);
        js_texto(p, fim, "character", ach[k].p, sizeof ach[k].p);
        if (!js_texto(p, fim, "release_date", data, sizeof data))
          js_texto(p, fim, "first_air_date", data, sizeof data);
        if (strlen(data) >= 4) { memcpy(ach[k].y, data, 4); ach[k].y[4] = 0; }
        if (js_texto(p, fim, "poster_path", caminho, sizeof caminho) &&
            caminho[0] == '/')
          snprintf(ach[k].po, sizeof ach[k].po,
                   "https://image.tmdb.org/t/p/w342%s", caminho);
        js_texto(p, fim, "imdb_id", ach[k].im, sizeof ach[k].im);
        ach[k].tmdb = (long)js_num(p, fim, "id", 0.0);
        ach[k].tipo[0] = 0;
        js_texto(p, fim, "media_type", ach[k].tipo, sizeof ach[k].tipo);
        ach[k].pop = js_num(p, fim, "popularity", 0.0);
        if (ach[k].t[0]) k++;
        p = js_prox(fim);
      } }

    // Por POPULARIDADE, como o web (`right.popularity - left.popularity`). A
    // ordem que o TMDB devolve e cronologica, e com ela os trabalhos pelos
    // quais a pessoa e conhecida ficam no fim da lista.
    { int i, j;
      for (i = 1; i < k; i++) {
        Credito tmp = ach[i];
        for (j = i; j > 0 && ach[j-1].pop < tmp.pop; j--) ach[j] = ach[j-1];
        ach[j] = tmp;
      } }

    pthread_mutex_lock(&trava);
    if (pedido->geracao == geracao && n[0]) {
      idPedido = id;
      int i;
      snprintf(nome, sizeof nome, "%s", n[0] ? n : pedido->nome);
      snprintf(foto, sizeof foto, "%s", f[0] ? f : pedido->foto);
      snprintf(bio,  sizeof bio,  "%s", b);
      snprintf(area, sizeof area, "%s", a[0] ? traduzArea(a) : "");
      for (i = 0; i < k; i++) {
        snprintf(cred[i].t, sizeof cred[i].t, "%s", ach[i].t);
        snprintf(cred[i].p,  sizeof cred[i].p,  "%s", ach[i].p);
        snprintf(cred[i].y,    sizeof cred[i].y,    "%s", ach[i].y);
        snprintf(cred[i].po, sizeof cred[i].po, "%s", ach[i].po);
        snprintf(cred[i].im,   sizeof cred[i].im,   "%s", ach[i].im);
        cred[i].tmdb = ach[i].tmdb;
        snprintf(cred[i].tipo, sizeof cred[i].tipo, "%s", ach[i].tipo);
      }
      nCred = k;
      pronta = 1;
    }
    pthread_mutex_unlock(&trava);
    printf("[pessoa] %ld %s -> %d creditos\n", id, n, k); fflush(stdout);
  }
  free(corpo);
}

static void *buscar(void *arg) {
  (void)arg;
  pthread_mutex_lock(&trava);
  for (;;) {
    Pedido pedido = { .id = idPedido, .geracao = geracao };
    snprintf(pedido.nome, sizeof pedido.nome, "%s", nomeSemente);
    snprintf(pedido.foto, sizeof pedido.foto, "%s", fotoSemente);
    snprintf(pedido.imdb, sizeof pedido.imdb, "%s", ctxImdb);
    snprintf(pedido.tipo, sizeof pedido.tipo, "%s", ctxTipo);
    pthread_mutex_unlock(&trava);
    buscarPedido(&pedido);
    pthread_mutex_lock(&trava);
    if (pedido.geracao != geracao) continue;
    falhou = !pronta;
    fioVivo = 0;
    pthread_mutex_unlock(&trava);
    return NULL;
  }
}

void pessoa_pedir(long tmdbId, const char *nomeConhecido, const char *fotoConhecida) {
  if (tmdbId <= 0 && !(nomeConhecido && nomeConhecido[0])) return;
  pthread_mutex_lock(&trava);
  if (tmdbId > 0 && idPedido == tmdbId && pronta) { pthread_mutex_unlock(&trava); return; }
  geracao++;
  idPedido = tmdbId;
  nCred = 0; pronta = 0; falhou = 0; bio[0] = 0; area[0] = 0;
  // O nome e a foto que o elenco ja tinha entram na hora, para a tela abrir
  // com conteudo em vez de vazia enquanto a rede responde.
  snprintf(nomeSemente, sizeof nomeSemente, "%s", nomeConhecido ? nomeConhecido : "");
  snprintf(fotoSemente, sizeof fotoSemente, "%s", fotoConhecida ? fotoConhecida : "");
  snprintf(nome, sizeof nome, "%s", nomeSemente);
  snprintf(foto, sizeof foto, "%s", fotoSemente);
  // One worker drains the latest selection; older results cannot publish.
  if (!fioVivo) {
    fioVivo = 1;
    if (pthread_create(&fio, NULL, buscar, NULL) != 0) {
      fioVivo = 0;
      falhou = 1;
    } else pthread_detach(fio);
  }
  pthread_mutex_unlock(&trava);
}

int pessoa_falhou(void) {
  pthread_mutex_lock(&trava);
  int erro = falhou;
  pthread_mutex_unlock(&trava);
  return erro;
}
int  pessoa_pronta(void)     { return pronta; }
const char *pessoa_nome(void){ return nome; }
const char *pessoa_foto(void){ return foto; }
const char *pessoa_bio(void) { return bio; }
const char *pessoa_area(void){ return area; }

int pessoa_n_creditos(void)  { return nCred; }
const char *pessoa_credito_titulo(int i) {
  return (i >= 0 && i < nCred) ? cred[i].t : "";
}
const char *pessoa_credito_papel(int i) {
  return (i >= 0 && i < nCred) ? cred[i].p : "";
}
const char *pessoa_credito_ano(int i) {
  return (i >= 0 && i < nCred) ? cred[i].y : "";
}
const char *pessoa_credito_poster(int i) {
  return (i >= 0 && i < nCred) ? cred[i].po : "";
}
const char *pessoa_credito_imdb(int i) {
  return (i >= 0 && i < nCred) ? cred[i].im : "";
}
long pessoa_credito_tmdb(int i) {
  return (i >= 0 && i < nCred) ? cred[i].tmdb : 0;
}
const char *pessoa_credito_tipo(int i) {
  return (i >= 0 && i < nCred) ? cred[i].tipo : "";
}
