// Ver pluginjs.h. Quatro partes: o orcamento global, o alocador que mede, a
// ponte nativa (`__nv`, consumida por src/pluginsjs/base.js) e o laco.
// Portado de agente/plugins2 (F09) com a rede trocada por plugrede/N01 e os
// limites globais, de fila e de geracao acrescentados.
#include "pluginjs.h"
#include "htmlq.h"
#include "plugrede.h"
#include "vendor/quickjs/quickjs.h"
#include "pluginsjs_gerado.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/time.h>
#include <time.h>

#define PJ_PRAZO_PADRAO_MS   20000
#define PJ_REDE_PADRAO_S     15
#define PJ_BYTES_COPIA       (1024L * 1024)       // ArrayBuffer so ate 1 MB
// 5 MB por resposta, e nao o 1 MB do Android: o data.json do StreamFlix tem
// 3,6 MB (medido em agente/plugins2, 01/10).
#define PJ_REDE_MAX_CORPO    (5L * 1024 * 1024)
#define PJ_REDE_MIN_RESERVA  (256L * 1024)
#define PJ_MEM_PADRAO        (48L * 1024 * 1024)
#define PJ_DOM_PADRAO        (16L * 1024 * 1024)
#define PJ_PILHA_JS          (1024 * 1024)
#define PJ_LOG_MAX           12
#define PJ_FETCH_VOO         6     // por runtime, ao mesmo tempo no pool
#define PJ_FETCH_FILA        64    // por runtime, esperando vaga (alem do voo)
#define PJ_FETCH_TOTAL       300   // por execucao
#define PJ_TIMERS_MAX        256
#define PJ_FIOS_REDE         6

// Orcamento global padrao por alvo. O WGT tem heap fixo de 256 MiB e malloc
// que ABORTA (ABORTING_MALLOC, tools/tizen.sh): o teto aqui tem de pegar antes.
#if defined(__EMSCRIPTEN__)
#define PJ_HEAP_GLOBAL  (40u * 1024 * 1024)
#define PJ_REDE_GLOBAL  (12u * 1024 * 1024)
#elif defined(NV_WEBOS) || defined(NV_TPK40)
#define PJ_HEAP_GLOBAL  (64u * 1024 * 1024)
#define PJ_REDE_GLOBAL  (16u * 1024 * 1024)
#else
#define PJ_HEAP_GLOBAL  (128u * 1024 * 1024)
#define PJ_REDE_GLOBAL  (24u * 1024 * 1024)
#endif

size_t pj_pilha(void) { return 4u * 1024 * 1024; }

static unsigned long agoraMs(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (unsigned long)ts.tv_sec * 1000UL + (unsigned long)ts.tv_nsec / 1000000UL;
}

// ------------------------------------------------------------ orcamento global
static size_t heapTeto = PJ_HEAP_GLOBAL, redeTeto = PJ_REDE_GLOBAL;
static size_t heapUso, redeUso, heapPico;

static int reservar(size_t *uso, size_t teto, size_t n) {
  size_t atual = __atomic_load_n(uso, __ATOMIC_RELAXED);
  for (;;) {
    size_t t = __atomic_load_n(&teto, __ATOMIC_RELAXED);
    if (n > t || atual > t - n) return 0;
    if (__atomic_compare_exchange_n(uso, &atual, atual + n, 1, __ATOMIC_ACQ_REL, __ATOMIC_RELAXED)) {
      if (uso == &heapUso) {
        size_t p = __atomic_load_n(&heapPico, __ATOMIC_RELAXED);
        while (atual + n > p && !__atomic_compare_exchange_n(&heapPico, &p, atual + n, 1,
                                                            __ATOMIC_RELAXED, __ATOMIC_RELAXED)) {}
      }
      return 1;
    }
  }
}
static void devolver(size_t *uso, size_t n) { if (n) __atomic_fetch_sub(uso, n, __ATOMIC_ACQ_REL); }
#define HEAP_RESERVAR(n) reservar(&heapUso, __atomic_load_n(&heapTeto, __ATOMIC_RELAXED), (n))
#define REDE_RESERVAR(n) reservar(&redeUso, __atomic_load_n(&redeTeto, __ATOMIC_RELAXED), (n))

void pj_orcamento_definir(size_t heap, size_t rede) {
  if (heap) __atomic_store_n(&heapTeto, heap, __ATOMIC_RELAXED);
  if (rede) __atomic_store_n(&redeTeto, rede, __ATOMIC_RELAXED);
}
size_t pj_orcamento_heap_uso(void) { return __atomic_load_n(&heapUso, __ATOMIC_RELAXED); }
size_t pj_orcamento_rede_uso(void) { return __atomic_load_n(&redeUso, __ATOMIC_RELAXED); }
size_t pj_orcamento_heap_pico(void) { return __atomic_load_n(&heapPico, __ATOMIC_RELAXED); }
void   pj_orcamento_zerar_pico(void) { __atomic_store_n(&heapPico, pj_orcamento_heap_uso(), __ATOMIC_RELAXED); }

// ------------------------------------------------------------ alocador
// Cabecalho de 16 bytes com o tamanho: da o pico do runtime sem depender de
// malloc_usable_size, e debita/credita o orcamento global ANTES do malloc.
typedef struct { size_t atual, pico; int recusou; } Medidor;
#define CAB 16
static void *mMalloc(void *op, size_t n) {
  Medidor *m = op; char *p;
  if (n > (size_t)-1 - CAB || !HEAP_RESERVAR(n)) { m->recusou = 1; return NULL; }
  p = malloc(n + CAB);
  if (!p) { devolver(&heapUso, n); m->recusou = 1; return NULL; }
  *(size_t *)p = n; m->atual += n; if (m->atual > m->pico) m->pico = m->atual;
  return p + CAB;
}
static void *mCalloc(void *op, size_t c, size_t n) {
  void *p;
  if (n && c > ((size_t)-1 - CAB) / n) return NULL;
  p = mMalloc(op, c * n);
  if (p) memset(p, 0, c * n);
  return p;
}
static void mFree(void *op, void *p) {
  Medidor *m = op; size_t n;
  if (!p) return;
  n = *(size_t *)((char *)p - CAB);
  m->atual -= n;
  free((char *)p - CAB);
  devolver(&heapUso, n);
}
static void *mRealloc(void *op, void *p, size_t n) {
  Medidor *m = op; char *q; size_t velho;
  if (!p) return mMalloc(op, n);
  if (!n) { mFree(op, p); return NULL; }
  if (n > (size_t)-1 - CAB) { m->recusou = 1; return NULL; }
  velho = *(size_t *)((char *)p - CAB);
  if (n > velho && !HEAP_RESERVAR(n - velho)) { m->recusou = 1; return NULL; }
  q = realloc((char *)p - CAB, n + CAB);
  if (!q) { if (n > velho) devolver(&heapUso, n - velho); m->recusou = 1; return NULL; }
  if (n < velho) devolver(&heapUso, velho - n);
  *(size_t *)q = n; m->atual += n; m->atual -= velho;
  if (m->atual > m->pico) m->pico = m->atual;
  return q + CAB;
}
static size_t mUsavel(const void *p) { return p ? *(const size_t *)((const char *)p - CAB) : 0; }

// ------------------------------------------------------------ fetch (fios)
// Caixa de entrega: compartilhada entre o laco e os fios de rede, viva ate o
// ultimo dos dois soltar (refs). `morta` = o laco foi embora; quem chegar
// depois solta o proprio pedido.
typedef struct Job Job;
typedef struct {
  pthread_mutex_t m;
  pthread_cond_t c;
  int refs, morta;
  Job *prontos;
} Caixa;

struct Job {
  Job *prox;
  Caixa *cx;
  int id;
  char *url, *metodo, *corpo, *cabs, **vet;
  size_t nCorpo;
  int seguir;
  unsigned prazoMs;
  size_t max, reserva;        // teto do corpo e bytes reservados no orcamento
  int cancel;                 // atomico (CANCELAR/CANCELADO)
  RedeJob *rj;
  int ok, recusa;             // recusa: 1 = so http(s), 2 = orcamento de rede
  RedeResposta resp;
};

static void jobSoltar(Job *j) {
  if (!j) return;
  rede_resposta_limpar(&j->resp);
  devolver(&redeUso, j->reserva);
  rede_job_soltar(j->rj);
  free(j->url); free(j->metodo); free(j->corpo); free(j->cabs); free(j->vet); free(j);
}
static void caixaSoltar(Caixa *cx) {
  pthread_mutex_destroy(&cx->m); pthread_cond_destroy(&cx->c); free(cx);
}

static void entregar(Job *j) {
  Caixa *cx = j->cx;
  int ultimo;
  pthread_mutex_lock(&cx->m);
  cx->refs--;
  if (cx->morta) jobSoltar(j);
  else { j->prox = cx->prontos; cx->prontos = j; pthread_cond_signal(&cx->c); }
  ultimo = cx->morta && cx->refs == 0;
  pthread_mutex_unlock(&cx->m);
  if (ultimo) caixaSoltar(cx);
}

#define CANCELAR(j) __atomic_store_n(&(j)->cancel, 1, __ATOMIC_RELEASE)
#define CANCELADO(j) __atomic_load_n(&(j)->cancel, __ATOMIC_ACQUIRE)
static int jobParar(void *u) { return CANCELADO((Job *)u) != 0; }

static void rodarJob(Job *j) {
  RedePedido q;
  memset(&q, 0, sizeof q);
  if (!CANCELADO(j)) {
    q.metodo = j->metodo; q.url = j->url; q.cabecalhos = (const char *const *)j->vet;
    q.corpo = j->corpo; q.n_corpo = j->nCorpo; q.prazo_ms = j->prazoMs;
    q.seguir = j->seguir; q.max_bytes = j->max; q.job = j->rj;
    q.parar = jobParar; q.parar_usuario = j;
    j->ok = plugrede_pedir(&q, &j->resp);
  }
  entregar(j);
}

// POOL GLOBAL de fios de rede. A fila global e limitada indiretamente: cada
// runtime poe no maximo PJ_FETCH_VOO jobs nela (o resto espera no runtime).
static pthread_mutex_t poolM = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t poolC = PTHREAD_COND_INITIALIZER;
static Job *poolIni, *poolFim;
static int poolFios, poolFalhou;

static void *fioRede(void *u) {
  (void)u;
  for (;;) {
    Job *j;
    pthread_mutex_lock(&poolM);
    while (!poolIni) pthread_cond_wait(&poolC, &poolM);
    j = poolIni; poolIni = j->prox; if (!poolIni) poolFim = NULL;
    pthread_mutex_unlock(&poolM);
    j->prox = NULL;
    rodarJob(j);
  }
  return NULL;
}

static void despachar(Job *j) {
  int inline_ = 0;
  pthread_mutex_lock(&j->cx->m);
  j->cx->refs++;
  pthread_mutex_unlock(&j->cx->m);
  pthread_mutex_lock(&poolM);
  while (poolFios < PJ_FIOS_REDE && !poolFalhou) {
    pthread_t f; pthread_attr_t a;
    pthread_attr_init(&a);
    pthread_attr_setstacksize(&a, 512 * 1024);
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);
    if (pthread_create(&f, &a, fioRede, NULL) == 0) poolFios++;
    else poolFalhou = 1;
    pthread_attr_destroy(&a);
  }
  if (!poolFios) inline_ = 1;
  else {
    j->prox = NULL;
    if (poolFim) poolFim->prox = j; else poolIni = j;
    poolFim = j;
    pthread_cond_signal(&poolC);
  }
  pthread_mutex_unlock(&poolM);
  if (inline_) rodarJob(j);
}

// ------------------------------------------------------------ execucao

typedef struct { int id; unsigned long quando, intervalo; JSValue fn; } Tm;
typedef struct { int id; JSValue cb; Job *job; int voando; } Fx;

typedef struct {
  const PjPedido *p;
  PjResultado *r;
  JSRuntime *rt;
  JSContext *ctx;
  JSClassID classeDoc;
  Medidor med;
  Caixa *cx;
  Tm *tm; int nTm, capTm;
  Fx *fx; int nFx, capFx;
  int voo, esperando;
  int proxId, logs;
  unsigned long prazo;
  size_t dom, domMax;
  int interrompido;
  JSValue cripto;
} Ex;

static int deveParar(Ex *e) {
  const PjPedido *p = e->p;
  if (p->cancelado && __atomic_load_n(p->cancelado, __ATOMIC_ACQUIRE)) return 1;
  if (p->parar && p->parar(p->pararU)) return 1;
  if (p->job && rede_job_estado(p->job) != REDE_OK) return 1;
  return 0;
}

static int interromper(JSRuntime *rt, void *op) {
  Ex *e = op;
  (void)rt;
  if (agoraMs() > e->prazo || deveParar(e)) { e->interrompido = 1; return 1; }
  return 0;
}

static Ex *exDe(JSContext *ctx) { return JS_GetContextOpaque(ctx); }

static void logar(Ex *e, int nivel, const char *msg) {
  char linha[260];
  size_t k;
  if (nivel == 0 && !e->p->verLog) return;
  if (!e->p->verLog && e->logs >= PJ_LOG_MAX) return;
  e->logs++;
  snprintf(linha, sizeof linha, "%s", msg ? msg : "");
  for (k = 0; linha[k]; k++) if ((unsigned char)linha[k] < ' ') linha[k] = ' ';
  printf("[plugin] %s %s: %s\n", e->p->idScraper ? e->p->idScraper : "?",
         nivel >= 2 ? "erro" : nivel ? "aviso" : "log", linha);
  fflush(stdout);
}

static void logarExcecao(Ex *e, const char *onde) {
  JSValue x = JS_GetException(e->ctx);
  const char *s = JS_ToCString(e->ctx, x);
  char m[240];
  snprintf(m, sizeof m, "%s: %s", onde, s ? s : "?");
  if (!e->r->erro[0]) snprintf(e->r->erro, sizeof e->r->erro, "%s", m);
  logar(e, 2, m);
  if (s) JS_FreeCString(e->ctx, s);
  JS_FreeValue(e->ctx, x);
}

// --- console, tempo, texto

static JSValue nLog(JSContext *ctx, JSValueConst t, int argc, JSValueConst *argv) {
  int nivel = 0;
  const char *s;
  (void)t;
  if (argc < 2) return JS_UNDEFINED;
  JS_ToInt32(ctx, &nivel, argv[0]);
  s = JS_ToCString(ctx, argv[1]);
  logar(exDe(ctx), nivel, s);
  if (s) JS_FreeCString(ctx, s);
  return JS_UNDEFINED;
}

static JSValue nTimer(JSContext *ctx, JSValueConst t, int argc, JSValueConst *argv) {
  Ex *e = exDe(ctx);
  double ms = 0;
  int rep = 0;
  (void)t;
  if (argc < 1 || !JS_IsFunction(ctx, argv[0])) return JS_NewInt32(ctx, 0);
  if (argc > 1) JS_ToFloat64(ctx, &ms, argv[1]);
  if (argc > 2) rep = JS_ToBool(ctx, argv[2]);
  if (ms < 0 || ms != ms) ms = 0;
  if (ms > 600000) ms = 600000;
  if (rep && ms < 10) ms = 10;
  if (e->nTm >= PJ_TIMERS_MAX) return JS_ThrowRangeError(ctx, "Plugin timer quota exceeded");
  if (e->nTm == e->capTm) {
    int cap = e->capTm ? e->capTm * 2 : 16;
    Tm *x = realloc(e->tm, sizeof(Tm) * (size_t)cap);
    if (!x) return JS_ThrowOutOfMemory(ctx);
    e->tm = x; e->capTm = cap;
  }
  e->tm[e->nTm].id = ++e->proxId;
  e->tm[e->nTm].quando = agoraMs() + (unsigned long)ms;
  e->tm[e->nTm].intervalo = rep ? (unsigned long)ms : 0;
  e->tm[e->nTm].fn = JS_DupValue(ctx, argv[0]);
  return JS_NewInt32(ctx, e->tm[e->nTm++].id);
}

static JSValue nUntimer(JSContext *ctx, JSValueConst t, int argc, JSValueConst *argv) {
  Ex *e = exDe(ctx);
  int id = 0, k;
  (void)t;
  if (argc < 1) return JS_UNDEFINED;
  JS_ToInt32(ctx, &id, argv[0]);
  for (k = 0; k < e->nTm; k++)
    if (e->tm[k].id == id) {
      JS_FreeValue(ctx, e->tm[k].fn);
      e->tm[k] = e->tm[--e->nTm];
      break;
    }
  return JS_UNDEFINED;
}

static JSValue nUtf8(JSContext *ctx, JSValueConst t, int argc, JSValueConst *argv) {
  size_t n = 0;
  uint8_t *b;
  (void)t;
  if (argc < 1) return JS_NewString(ctx, "");
  b = JS_GetArrayBuffer(ctx, &n, argv[0]);
  if (!b) return JS_NewString(ctx, "");
  return JS_NewStringLen(ctx, (const char *)b, n);
}

// CryptoJS sob demanda, num escopo sem exports/module/define: o UMD dele cai
// no ramo "root.CryptoJS = ..." com root = o objeto `h`.
static JSValue nCripto(JSContext *ctx, JSValueConst t, int argc, JSValueConst *argv) {
  Ex *e = exDe(ctx);
  static const char ini[] = "(function(){var exports=void 0,module=void 0,define=void 0;var h={};(function(){\n";
  static const char fim[] = "\n}).call(h);return h.CryptoJS;})()";
  char *src;
  size_t n;
  JSValue v;
  (void)t; (void)argc; (void)argv;
  if (!JS_IsUndefined(e->cripto)) return JS_DupValue(ctx, e->cripto);
  n = sizeof ini - 1 + sizeof PJ_CRIPTO - 1 + sizeof fim - 1;
  src = malloc(n + 1);
  if (!src) return JS_ThrowOutOfMemory(ctx);
  memcpy(src, ini, sizeof ini - 1);
  memcpy(src + sizeof ini - 1, PJ_CRIPTO, sizeof PJ_CRIPTO - 1);
  memcpy(src + sizeof ini - 1 + sizeof PJ_CRIPTO - 1, fim, sizeof fim);
  v = JS_Eval(ctx, src, n, "crypto-js.js", JS_EVAL_TYPE_GLOBAL);
  free(src);
  if (JS_IsException(v)) return v;
  e->cripto = JS_DupValue(ctx, v);
  e->r->criptoUsado = 1;
  return v;
}

// --- fetch

// Cabecalho que o N01 recusaria (Host, framing) ou que nao faz sentido vindo do
// scraper. Os outros passam; rede_pedir confere a sintaxe.
static int cabProibido(const char *l) {
  static const char *const NOMES[] = { "host:", "content-length:", "transfer-encoding:",
                                       "connection:", "keep-alive:", "upgrade:", "te:", NULL };
  int k;
  for (k = 0; NOMES[k]; k++) if (!strncasecmp(l, NOMES[k], strlen(NOMES[k]))) return 1;
  return 0;
}

// Poe na fila do pool os que cabem: vaga de voo e reserva no orcamento de rede.
static void bombear(Ex *e) {
  int k;
  for (k = 0; k < e->nFx && e->voo < PJ_FETCH_VOO; k++) {
    Job *j = e->fx[k].job;
    size_t want;
    if (e->fx[k].voando || !j) continue;
    want = j->max;
    if (!REDE_RESERVAR(want)) {
      // Sem o teto inteiro: tenta um teto menor (o corpo maior falha por
      // limite); sem nem o minimo, espera alguem devolver — ou falha se este
      // runtime nao tem nada em voo para devolver.
      size_t livre = __atomic_load_n(&redeTeto, __ATOMIC_RELAXED) - pj_orcamento_rede_uso();
      if (livre >= PJ_REDE_MIN_RESERVA && livre < want && REDE_RESERVAR(livre)) want = livre;
      // OUTRO PLUGIN segurando o orcamento tambem e "espera", e nao recusa
      // (2.0.0: "orcamento de rede dos plugins esgotado" ~2400 vezes em 4
      // pessoas, em fetch de poucos KB ao TMDB). Cada fetch reserva o teto do
      // corpo (5 MB) e o orcamento da TV e 16-24 MB: com varios plugins em
      // paralelo, quem chegava sem nada em voo era recusado na hora mesmo com
      // a rede quase vazia. Quem segura devolve ao terminar ou no prazo dele,
      // e este laco tenta de novo a cada 50 ms (esperar); o prazo do plugin
      // continua valendo.
      else if (e->voo > 0 || pj_orcamento_rede_uso() > 0) return;
      else {
        j->recusa = 2;
        e->fx[k].voando = 1; e->voo++; e->esperando--;
        pthread_mutex_lock(&e->cx->m); e->cx->refs++; pthread_mutex_unlock(&e->cx->m);
        entregar(j);
        continue;
      }
    }
    j->reserva = want; j->max = want;
    e->fx[k].voando = 1; e->voo++; e->esperando--;
    despachar(j);
  }
}

static JSValue nFetch(JSContext *ctx, JSValueConst t, int argc, JSValueConst *argv) {
  Ex *e = exDe(ctx);
  Job *j;
  const char *s;
  size_t n;
  (void)t;
  if (argc < 6 || !JS_IsFunction(ctx, argv[5])) return JS_ThrowTypeError(ctx, "fetch: argumentos");
  if (e->r->fetches >= PJ_FETCH_TOTAL || e->esperando >= PJ_FETCH_FILA) {
    e->r->fetchesRecusados++;
    return JS_ThrowRangeError(ctx, "Plugin fetch quota exceeded");
  }
  j = calloc(1, sizeof *j);
  if (!j) return JS_ThrowOutOfMemory(ctx);
  j->cx = e->cx;
  j->id = ++e->proxId;
  s = JS_ToCString(ctx, argv[0]); j->url = strdup(s ? s : ""); if (s) JS_FreeCString(ctx, s);
  s = JS_ToCString(ctx, argv[1]); j->metodo = strdup(s ? s : "GET"); if (s) JS_FreeCString(ctx, s);
  if (!j->url || !j->metodo) { jobSoltar(j); return JS_ThrowOutOfMemory(ctx); }
  { char *m; for (m = j->metodo; *m; m++) if (*m >= 'a' && *m <= 'z') *m = (char)(*m - 32); }
  s = JS_ToCStringLen(ctx, &n, argv[2]);
  if (s && n && n < REDE_CAB_PADRAO) {
    int linhas = 1, k = 0;
    char *p;
    j->cabs = malloc(n + 1);
    if (j->cabs) { memcpy(j->cabs, s, n); j->cabs[n] = 0; }
    for (p = j->cabs; p && *p; p++) if (*p == '\n') linhas++;
    if (linhas > 64) linhas = 64;
    j->vet = calloc((size_t)linhas + 1, sizeof(char *));
    for (p = j->cabs; j->vet && p && *p; ) {
      char *f = strchr(p, '\n');
      if (f) *f = 0;
      if (*p && k < linhas && !cabProibido(p)) j->vet[k++] = p;
      if (!f) break;
      p = f + 1;
    }
  }
  if (s) JS_FreeCString(ctx, s);
  if (JS_IsString(argv[3])) {
    s = JS_ToCStringLen(ctx, &n, argv[3]);
    if (s && n <= REDE_CORPO_PADRAO) { j->corpo = malloc(n + 1); if (j->corpo) { memcpy(j->corpo, s, n); j->corpo[n] = 0; j->nCorpo = n; } }
    if (s) JS_FreeCString(ctx, s);
  } else if (JS_IsArrayBuffer(argv[3])) {
    uint8_t *b = JS_GetArrayBuffer(ctx, &n, argv[3]);
    if (b && n <= REDE_CORPO_PADRAO) { j->corpo = malloc(n + 1); if (j->corpo) { memcpy(j->corpo, b, n); j->corpo[n] = 0; j->nCorpo = n; } }
  }
  if (!strcmp(j->metodo, "GET") || !strcmp(j->metodo, "HEAD")) { free(j->corpo); j->corpo = NULL; j->nCorpo = 0; }
  j->seguir = JS_ToBool(ctx, argv[4]);
  j->prazoMs = (unsigned)(e->p->redeSegundos > 0 ? e->p->redeSegundos : PJ_REDE_PADRAO_S) * 1000u;
  // O prazo do fetch nunca passa do prazo que sobra da execucao.
  { unsigned long agora = agoraMs();
    unsigned long resta = e->prazo > agora ? e->prazo - agora : 1;
    if (resta < j->prazoMs) j->prazoMs = (unsigned)resta; }
  j->max = (size_t)(e->p->redeMaxBytes > 0 ? e->p->redeMaxBytes : PJ_REDE_MAX_CORPO);
  if (j->max > REDE_CORPO_MAXIMO) j->max = REDE_CORPO_MAXIMO;
  if (e->p->grupo) j->rj = rede_job_criar(e->p->grupo);
  if (strncmp(j->url, "http://", 7) && strncmp(j->url, "https://", 8)) j->recusa = 1;
  if (e->nFx == e->capFx) {
    int cap = e->capFx ? e->capFx * 2 : 16;
    Fx *x = realloc(e->fx, sizeof(Fx) * (size_t)cap);
    if (!x) { jobSoltar(j); return JS_ThrowOutOfMemory(ctx); }
    e->fx = x; e->capFx = cap;
  }
  e->fx[e->nFx].id = j->id;
  e->fx[e->nFx].cb = JS_DupValue(ctx, argv[5]);
  e->fx[e->nFx].job = j;
  e->fx[e->nFx].voando = 0;
  e->nFx++;
  e->r->fetches++;
  if (j->recusa) {
    // So http(s): file:, data: e afins nao saem daqui. Entrega sem rede.
    e->fx[e->nFx - 1].voando = 1; e->voo++;
    pthread_mutex_lock(&e->cx->m); e->cx->refs++; pthread_mutex_unlock(&e->cx->m);
    entregar(j);
  } else {
    e->esperando++;
    bombear(e);
  }
  return JS_NewInt32(ctx, j->id);
}

static JSValue nCancel(JSContext *ctx, JSValueConst t, int argc, JSValueConst *argv) {
  Ex *e = exDe(ctx);
  int id = 0, k;
  (void)t;
  if (argc < 1) return JS_UNDEFINED;
  JS_ToInt32(ctx, &id, argv[0]);
  for (k = 0; k < e->nFx; k++)
    if (e->fx[k].id == id && e->fx[k].job) {
      Job *j = e->fx[k].job;
      CANCELAR(j);
      rede_job_cancelar(j->rj);
      if (!e->fx[k].voando) {
        // Ainda nao saiu: entrega como abortado sem passar pelo pool.
        e->fx[k].voando = 1; e->voo++; e->esperando--;
        pthread_mutex_lock(&e->cx->m); e->cx->refs++; pthread_mutex_unlock(&e->cx->m);
        entregar(j);
      }
      break;
    }
  return JS_UNDEFINED;
}

static const char *textoStatus(int s) {
  switch (s) {
  case 200: return "OK"; case 201: return "Created"; case 204: return "No Content";
  case 206: return "Partial Content"; case 301: return "Moved Permanently"; case 302: return "Found";
  case 303: return "See Other"; case 304: return "Not Modified"; case 307: return "Temporary Redirect";
  case 308: return "Permanent Redirect"; case 400: return "Bad Request"; case 401: return "Unauthorized";
  case 403: return "Forbidden"; case 404: return "Not Found"; case 429: return "Too Many Requests";
  case 500: return "Internal Server Error"; case 502: return "Bad Gateway"; case 503: return "Service Unavailable";
  }
  return "";
}

// "HTTP/1.1 200\r\nNome: v\r\n" (N01) -> "nome: v\n" (base.js).
static char *cabsParaJs(const char *c, size_t n) {
  char *o = malloc(n + 1), *w = o;
  size_t i = 0;
  if (!o) return NULL;
  while (i < n) {
    size_t a = i, k;
    const char *dois;
    while (i < n && c[i] != '\n') i++;
    k = i;
    if (k > a && c[k - 1] == '\r') k--;
    if (i < n) i++;
    if (k <= a || (k - a >= 5 && !strncasecmp(c + a, "HTTP/", 5))) continue;
    dois = memchr(c + a, ':', k - a);
    if (!dois) continue;
    for (; c + a < dois; a++) *w++ = (char)((c[a] >= 'A' && c[a] <= 'Z') ? c[a] + 32 : c[a]);
    memcpy(w, c + a, k - a); w += k - a;
    *w++ = '\n';
  }
  *w = 0;
  return o;
}

static const char *textoErro(const Job *j) {
  if (j->recusa == 1) return "so http(s)";
  if (j->recusa == 2) return "orcamento de rede dos plugins esgotado";
  if (CANCELADO(j)) return "abortado";
  switch (j->resp.erro) {
  case REDE_PRAZO: return "prazo";
  case REDE_LIMITE_CORPO: return "resposta maior que o teto";
  case REDE_LIMITE_CABECALHOS: return "cabecalhos maiores que o teto";
  case REDE_REDIRECT: return "redirect recusado";
  case REDE_CANCELADO: case REDE_GERACAO: return "abortado";
  case REDE_INDISPONIVEL: return "rede indisponivel";
  case REDE_ENTRADA: return "pedido invalido";
  case REDE_MEMORIA: return "sem memoria";
  default: return "rede";
  }
}

// Entrega os fetches que voltaram. 1 se entregou algum.
static int entregues(Ex *e) {
  Job *lista, *inv = NULL;
  int algum = 0;
  pthread_mutex_lock(&e->cx->m);
  lista = e->cx->prontos; e->cx->prontos = NULL;
  pthread_mutex_unlock(&e->cx->m);
  while (lista) { Job *n = lista->prox; lista->prox = inv; inv = lista; lista = n; }
  while (inv) {
    Job *j = inv;
    int k;
    JSValue cb = JS_UNDEFINED;
    inv = j->prox;
    for (k = 0; k < e->nFx; k++)
      if (e->fx[k].job == j) { cb = e->fx[k].cb; e->fx[k] = e->fx[--e->nFx]; e->voo--; break; }
    // A rede volta a ter vaga antes do callback, que pode pedir mais.
    devolver(&redeUso, j->reserva); j->reserva = 0;
    if (!JS_IsUndefined(cb) && !e->interrompido) {
      JSValue args[2], ret;
      algum = 1;
      if (j->ok && !j->recusa && !CANCELADO(j)) {
        JSValue o = JS_NewObject(e->ctx);
        char *cabs = cabsParaJs(j->resp.cabecalhos ? j->resp.cabecalhos : "", j->resp.n_cabecalhos);
        const char *fim = j->resp.final[0] ? j->resp.final : j->url;
        JS_SetPropertyStr(e->ctx, o, "status", JS_NewInt32(e->ctx, j->resp.status));
        JS_SetPropertyStr(e->ctx, o, "statusText", JS_NewString(e->ctx, textoStatus(j->resp.status)));
        JS_SetPropertyStr(e->ctx, o, "url", JS_NewString(e->ctx, fim));
        JS_SetPropertyStr(e->ctx, o, "redirected", JS_NewBool(e->ctx, strcmp(fim, j->url) != 0));
        JS_SetPropertyStr(e->ctx, o, "headers", JS_NewString(e->ctx, cabs ? cabs : ""));
        free(cabs);
        JS_SetPropertyStr(e->ctx, o, "body", JS_NewStringLen(e->ctx, j->resp.corpo ? j->resp.corpo : "", j->resp.n_corpo));
        if ((long)j->resp.n_corpo <= PJ_BYTES_COPIA)
          JS_SetPropertyStr(e->ctx, o, "bytes", JS_NewArrayBufferCopy(e->ctx, (const uint8_t *)(j->resp.corpo ? j->resp.corpo : ""), j->resp.n_corpo));
        e->r->bytesRede += (long)j->resp.n_corpo;
        if (e->p->verLog) {
          char m[200], seg[120];
          snprintf(m, sizeof m, "fetch %s %s -> %d (%zu bytes, %u ms)", j->metodo,
                   rede_url_publica(j->url, seg, sizeof seg), j->resp.status, j->resp.n_corpo, j->resp.ms);
          logar(e, 0, m);
        }
        args[0] = o; args[1] = JS_UNDEFINED;
      } else {
        const char *m = textoErro(j);
        e->r->fetchesFalhos++;
        args[0] = JS_NULL; args[1] = JS_NewString(e->ctx, m);
        { char seg[120], l[220];
          snprintf(l, sizeof l, "fetch %s falhou: %s", rede_url_publica(j->url, seg, sizeof seg), m);
          logar(e, CANCELADO(j) ? 0 : 1, l); }
      }
      // Solta o corpo nativo antes de entrar no JS: o pico nao dobra.
      rede_resposta_limpar(&j->resp);
      ret = JS_Call(e->ctx, cb, JS_UNDEFINED, 2, args);
      if (JS_IsException(ret)) logarExcecao(e, "callback do fetch");
      JS_FreeValue(e->ctx, ret);
      JS_FreeValue(e->ctx, args[0]); JS_FreeValue(e->ctx, args[1]);
    }
    if (!JS_IsUndefined(cb)) JS_FreeValue(e->ctx, cb);
    jobSoltar(j);
  }
  if (algum || e->esperando) bombear(e);
  return algum;
}

// --- html (htmlq)

static void docFinal(JSRuntime *rt, JSValueConst v) {
  Ex *e = JS_GetRuntimeOpaque(rt);
  HqDoc *d = JS_GetOpaque(v, e->classeDoc);
  size_t m;
  if (!d) return;
  m = hq_memoria(d);
  e->dom -= m;
  devolver(&heapUso, m);
  hq_soltar(d);
}

static HqDoc *docArg(JSContext *ctx, JSValueConst v) {
  return JS_GetOpaque(v, exDe(ctx)->classeDoc);
}
static int intArg(JSContext *ctx, JSValueConst v) { int i = -1; JS_ToInt32(ctx, &i, v); return i; }

static JSValue nhLoad(JSContext *ctx, JSValueConst t, int argc, JSValueConst *argv) {
  Ex *e = exDe(ctx);
  const char *s;
  size_t n = 0, cabe, reserva, m;
  HqDoc *d;
  JSValue o;
  (void)t;
  s = argc > 0 ? JS_ToCStringLen(ctx, &n, argv[0]) : NULL;
  // RESERVA ANTES DE MONTAR: o teto do documento e o que sobra da cota do
  // runtime, e essa quantia sai do orcamento global antes do primeiro byte.
  cabe = e->domMax > e->dom ? e->domMax - e->dom : 0;
  if (cabe < n + 4096) { JS_RunGC(e->rt); cabe = e->domMax > e->dom ? e->domMax - e->dom : 0; }
  reserva = n * 12 + 256 * 1024;        // fonte + nos + textos (medido ~8x, folga)
  if (reserva > cabe) reserva = cabe;
  if (reserva < n + 4096 || !HEAP_RESERVAR(reserva)) {
    if (s) JS_FreeCString(ctx, s);
    e->med.recusou = 1;
    return JS_ThrowRangeError(ctx, "Plugin DOM memory quota exceeded");
  }
  d = hq_carregar_max(s ? s : "", s ? n : 0, reserva);
  if (s) JS_FreeCString(ctx, s);
  if (!d) {
    devolver(&heapUso, reserva);
    e->med.recusou = 1;
    return JS_ThrowRangeError(ctx, "Plugin DOM memory quota exceeded");
  }
  m = hq_memoria(d);
  if (m > reserva) m = reserva;         // hq_carregar_max garante; defensivo
  devolver(&heapUso, reserva - m);
  e->dom += m;
  if (e->dom > e->r->domPico) e->r->domPico = e->dom;
  o = JS_NewObjectClass(ctx, (int)e->classeDoc);
  if (JS_IsException(o)) { e->dom -= m; devolver(&heapUso, m); hq_soltar(d); return o; }
  JS_SetOpaque(o, d);
  return o;
}

static JSValue arrayDe(JSContext *ctx, const int *v, int n) {
  JSValue a = JS_NewArray(ctx);
  int k;
  for (k = 0; k < n; k++) JS_SetPropertyUint32(ctx, a, (uint32_t)k, JS_NewInt32(ctx, v[k]));
  return a;
}

static JSValue nhSel(JSContext *ctx, JSValueConst t, int argc, JSValueConst *argv) {
  HqDoc *d;
  const char *s;
  int *v = NULL, n;
  JSValue a;
  (void)t;
  if (argc < 3 || !(d = docArg(ctx, argv[0]))) return JS_NewArray(ctx);
  s = JS_ToCString(ctx, argv[2]);
  n = hq_selecionar(d, intArg(ctx, argv[1]), s ? s : "", &v);
  if (s) JS_FreeCString(ctx, s);
  a = arrayDe(ctx, v, n > 0 ? n : 0);
  free(v);
  return a;
}

static JSValue nhTexto(JSContext *ctx, JSValueConst t, int argc, JSValueConst *argv) {
  HqDoc *d;
  int64_t n = 0, k;
  int *v;
  char *s;
  JSValue r;
  (void)t;
  if (argc < 2 || !(d = docArg(ctx, argv[0]))) return JS_NewString(ctx, "");
  if (JS_GetLength(ctx, argv[1], &n) < 0 || n <= 0) return JS_NewString(ctx, "");
  if (n > hq_nos(d)) n = hq_nos(d);
  v = malloc(sizeof(int) * (size_t)(n ? n : 1));
  if (!v) return JS_ThrowOutOfMemory(ctx);
  for (k = 0; k < n; k++) { JSValue x = JS_GetPropertyUint32(ctx, argv[1], (uint32_t)k); v[k] = intArg(ctx, x); JS_FreeValue(ctx, x); }
  s = hq_texto(d, v, (int)n);
  free(v);
  r = JS_NewString(ctx, s ? s : "");
  free(s);
  return r;
}

static JSValue nhHtml(JSContext *ctx, JSValueConst t, int argc, JSValueConst *argv) {
  HqDoc *d;
  char *s;
  JSValue r;
  (void)t;
  if (argc < 3 || !(d = docArg(ctx, argv[0]))) return JS_NewString(ctx, "");
  s = hq_html(d, intArg(ctx, argv[1]), JS_ToBool(ctx, argv[2]));
  r = JS_NewString(ctx, s ? s : "");
  free(s);
  return r;
}

static JSValue nhAttr(JSContext *ctx, JSValueConst t, int argc, JSValueConst *argv) {
  HqDoc *d;
  const char *nome, *v;
  (void)t;
  if (argc < 3 || !(d = docArg(ctx, argv[0]))) return JS_NULL;
  nome = JS_ToCString(ctx, argv[2]);
  v = hq_attr(d, intArg(ctx, argv[1]), nome ? nome : "");
  if (nome) JS_FreeCString(ctx, nome);
  return v && *v ? JS_NewString(ctx, v) : JS_NULL;
}

static JSValue nhAttrs(JSContext *ctx, JSValueConst t, int argc, JSValueConst *argv) {
  // hq nao expoe a lista crua; os nomes comuns cobrem quem le .attribs.
  static const char *const NOMES[] = {"href","src","class","id","title","alt","name","value",
    "type","rel","content","property","data-src","data-href","data-url","data-id","data-link",
    "data-season","data-episode","onclick","action","method","style","target","download",NULL};
  HqDoc *d;
  JSValue o = JS_NewObject(ctx);
  int i, k;
  (void)t;
  if (argc < 2 || !(d = docArg(ctx, argv[0]))) return o;
  i = intArg(ctx, argv[1]);
  for (k = 0; NOMES[k]; k++) {
    const char *v = hq_attr(d, i, NOMES[k]);
    if (v) JS_SetPropertyStr(ctx, o, NOMES[k], JS_NewString(ctx, v));
  }
  return o;
}

static JSValue nhTag(JSContext *ctx, JSValueConst t, int argc, JSValueConst *argv) {
  HqDoc *d;
  (void)t;
  if (argc < 2 || !(d = docArg(ctx, argv[0]))) return JS_NewString(ctx, "");
  return JS_NewString(ctx, hq_tag(d, intArg(ctx, argv[1])));
}

static JSValue nhRel(JSContext *ctx, JSValueConst t, int argc, JSValueConst *argv, int magic) {
  HqDoc *d;
  int i;
  (void)t;
  if (argc < 2 || !(d = docArg(ctx, argv[0]))) return JS_NewInt32(ctx, -1);
  i = intArg(ctx, argv[1]);
  return JS_NewInt32(ctx, magic == 0 ? hq_pai(d, i) : magic == 1 ? hq_prox(d, i) : hq_ant(d, i));
}

static JSValue nhFilhos(JSContext *ctx, JSValueConst t, int argc, JSValueConst *argv) {
  HqDoc *d;
  int *v = NULL, n;
  JSValue a;
  (void)t;
  if (argc < 2 || !(d = docArg(ctx, argv[0]))) return JS_NewArray(ctx);
  n = hq_filhos(d, intArg(ctx, argv[1]), &v);
  a = arrayDe(ctx, v, n);
  free(v);
  return a;
}

static JSValue nhCasa(JSContext *ctx, JSValueConst t, int argc, JSValueConst *argv) {
  HqDoc *d;
  const char *s;
  int r;
  (void)t;
  if (argc < 3 || !(d = docArg(ctx, argv[0]))) return JS_FALSE;
  s = JS_ToCString(ctx, argv[2]);
  r = hq_casa(d, intArg(ctx, argv[1]), s ? s : "");
  if (s) JS_FreeCString(ctx, s);
  return JS_NewBool(ctx, r == 1);
}

static const JSCFunctionListEntry FUNCS_HTML[] = {
  JS_CFUNC_DEF("load", 1, nhLoad),
  JS_CFUNC_DEF("sel", 3, nhSel),
  JS_CFUNC_DEF("texto", 2, nhTexto),
  JS_CFUNC_DEF("html", 3, nhHtml),
  JS_CFUNC_DEF("attr", 3, nhAttr),
  JS_CFUNC_DEF("attrs", 2, nhAttrs),
  JS_CFUNC_DEF("tag", 2, nhTag),
  JS_CFUNC_MAGIC_DEF("pai", 2, nhRel, 0),
  JS_CFUNC_MAGIC_DEF("prox", 2, nhRel, 1),
  JS_CFUNC_MAGIC_DEF("ant", 2, nhRel, 2),
  JS_CFUNC_DEF("filhos", 2, nhFilhos),
  JS_CFUNC_DEF("casa", 3, nhCasa),
};

static const JSCFunctionListEntry FUNCS_NV[] = {
  JS_CFUNC_DEF("log", 2, nLog),
  JS_CFUNC_DEF("timer", 3, nTimer),
  JS_CFUNC_DEF("untimer", 1, nUntimer),
  JS_CFUNC_DEF("fetch", 6, nFetch),
  JS_CFUNC_DEF("cancel", 1, nCancel),
  JS_CFUNC_DEF("utf8", 1, nUtf8),
  JS_CFUNC_DEF("cripto", 0, nCripto),
};

// ------------------------------------------------------------ laco

static void jsonTexto(char *dst, size_t tam, const char *s) {
  size_t k = 0;
  if (tam < 3) { if (tam) dst[0] = 0; return; }
  dst[k++] = '"';
  for (; s && *s && k + 8 < tam; s++) {
    unsigned char c = (unsigned char)*s;
    if (c == '"' || c == '\\') { dst[k++] = '\\'; dst[k++] = (char)c; }
    else if (c < 0x20) k += (size_t)snprintf(dst + k, tam - k, "\\u%04x", c);
    else dst[k++] = (char)c;
  }
  dst[k++] = '"';
  dst[k] = 0;
}

static int dispararTimers(Ex *e) {
  unsigned long agora = agoraMs();
  int k, algum = 0;
  for (k = 0; k < e->nTm; k++) {
    if (e->tm[k].quando > agora) continue;
    { JSValue fn = JS_DupValue(e->ctx, e->tm[k].fn), ret;
      if (e->tm[k].intervalo) e->tm[k].quando = agora + e->tm[k].intervalo;
      else { JS_FreeValue(e->ctx, e->tm[k].fn); e->tm[k] = e->tm[--e->nTm]; k--; }
      ret = JS_Call(e->ctx, fn, JS_UNDEFINED, 0, NULL);
      if (JS_IsException(ret)) logarExcecao(e, "timer");
      JS_FreeValue(e->ctx, ret);
      JS_FreeValue(e->ctx, fn);
      algum = 1; }
    break;   // um por volta: o timer pode ter criado/limpado outros
  }
  return algum;
}

static void rodarJobs(Ex *e) {
  JSContext *c2;
  int r;
  while ((r = JS_ExecutePendingJob(e->rt, &c2)) != 0) {
    if (r < 0) logarExcecao(e, "promessa");
    if (e->interrompido) break;
  }
}

// Espera rede/timer sem segurar trava alem da caixa; acorda a cada 50 ms para
// olhar cancelamento e geracao.
static void esperar(Ex *e) {
  unsigned long agora = agoraMs(), ate = e->prazo;
  int k;
  struct timeval tv;
  struct timespec ts;
  for (k = 0; k < e->nTm; k++) if (e->tm[k].quando < ate) ate = e->tm[k].quando;
  if (ate > agora + 50) ate = agora + 50;
  if (ate <= agora) return;
  gettimeofday(&tv, NULL);
  { unsigned long long ns = (unsigned long long)tv.tv_usec * 1000ULL + (unsigned long long)(ate - agora) * 1000000ULL;
    ts.tv_sec = tv.tv_sec + (time_t)(ns / 1000000000ULL);
    ts.tv_nsec = (long)(ns % 1000000000ULL); }
  pthread_mutex_lock(&e->cx->m);
  if (!e->cx->prontos) pthread_cond_timedwait(&e->cx->c, &e->cx->m, &ts);
  pthread_mutex_unlock(&e->cx->m);
}

int pj_executar(const PjPedido *p, PjResultado *r) {
  static const JSMallocFunctions MF = { mCalloc, mMalloc, mFree, mRealloc, mUsavel };
  Ex e;
  JSValue g, nv, h, v, prom = JS_UNDEFINED;
  JSClassDef def;
  unsigned long t0 = agoraMs();
  char *src = NULL;
  int nRes = 0, k;

  memset(r, 0, sizeof *r);
  memset(&e, 0, sizeof e);
  e.p = p; e.r = r; e.cripto = JS_UNDEFINED;
  e.prazo = t0 + (unsigned long)(p->prazoMs > 0 ? p->prazoMs : PJ_PRAZO_PADRAO_MS);
  e.domMax = (size_t)(p->domMax > 0 ? p->domMax : PJ_DOM_PADRAO);
  rede_job_reter(p->job);
  if (deveParar(&e)) { r->cancelado = 1; snprintf(r->erro, sizeof r->erro, "cancelado"); rede_job_soltar(p->job); return 0; }
  e.cx = calloc(1, sizeof *e.cx);
  if (!e.cx) { snprintf(r->erro, sizeof r->erro, "sem memoria"); rede_job_soltar(p->job); return 0; }
  pthread_mutex_init(&e.cx->m, NULL);
  pthread_cond_init(&e.cx->c, NULL);
  e.cx->refs = 1;

  e.rt = JS_NewRuntime2(&MF, &e.med);
  if (!e.rt) { snprintf(r->erro, sizeof r->erro, "sem runtime (orcamento)"); r->semMemoria = 1; goto fim; }
  JS_SetRuntimeOpaque(e.rt, &e);
  JS_SetMemoryLimit(e.rt, (size_t)(p->memoriaMax > 0 ? p->memoriaMax : PJ_MEM_PADRAO));
  JS_SetMaxStackSize(e.rt, PJ_PILHA_JS);
  JS_SetInterruptHandler(e.rt, interromper, &e);
  JS_NewClassID(e.rt, &e.classeDoc);
  memset(&def, 0, sizeof def);
  def.class_name = "NuvioDoc";
  def.finalizer = docFinal;
  JS_NewClass(e.rt, e.classeDoc, &def);
  e.ctx = JS_NewContext(e.rt);
  if (!e.ctx) { snprintf(r->erro, sizeof r->erro, "sem contexto (orcamento)"); r->semMemoria = 1; goto fim; }
  JS_SetContextOpaque(e.ctx, &e);

  g = JS_GetGlobalObject(e.ctx);
  nv = JS_NewObject(e.ctx);
  JS_SetPropertyFunctionList(e.ctx, nv, FUNCS_NV, (int)(sizeof FUNCS_NV / sizeof FUNCS_NV[0]));
  h = JS_NewObject(e.ctx);
  JS_SetPropertyFunctionList(e.ctx, h, FUNCS_HTML, (int)(sizeof FUNCS_HTML / sizeof FUNCS_HTML[0]));
  JS_SetPropertyStr(e.ctx, nv, "html", h);
  JS_SetPropertyStr(e.ctx, nv, "id", JS_NewString(e.ctx, p->idScraper ? p->idScraper : ""));
  JS_SetPropertyStr(e.ctx, nv, "ajustes", JS_NewString(e.ctx, p->ajustesJson ? p->ajustesJson : "{}"));
  JS_SetPropertyStr(e.ctx, nv, "tmdb", JS_NewString(e.ctx, p->tmdbChave ? p->tmdbChave : ""));
  JS_SetPropertyStr(e.ctx, g, "__nv", nv);
  JS_FreeValue(e.ctx, g);

  v = JS_Eval(e.ctx, PJ_BASE, sizeof PJ_BASE - 1, "nuvio-base.js", JS_EVAL_TYPE_GLOBAL);
  if (JS_IsException(v)) { logarExcecao(&e, "ambiente"); goto fim; }
  JS_FreeValue(e.ctx, v);

  { static const char ini[] = "var module = { exports: {} }; var exports = module.exports;"
                              "(function (module, exports) {\n";
    static const char fimS[] = "\n;if (typeof getStreams === 'function' && module.exports && !module.exports.getStreams)"
                               " module.exports.getStreams = getStreams;\n})(module, exports);"
                               "globalThis.__ex = module.exports;";
    size_t n = sizeof ini - 1 + p->nCodigo + sizeof fimS - 1;
    src = malloc(n + 1);
    if (!src) { snprintf(r->erro, sizeof r->erro, "sem memoria"); goto fim; }
    memcpy(src, ini, sizeof ini - 1);
    memcpy(src + sizeof ini - 1, p->codigo, p->nCodigo);
    memcpy(src + sizeof ini - 1 + p->nCodigo, fimS, sizeof fimS);
    v = JS_Eval(e.ctx, src, n, p->arquivo ? p->arquivo : "plugin.js", JS_EVAL_TYPE_GLOBAL);
    free(src); src = NULL;
    if (JS_IsException(v)) { logarExcecao(&e, "codigo do scraper"); goto fim; }
    JS_FreeValue(e.ctx, v); }
  r->msCompilar = agoraMs() - t0;

  { char id[200], nome[200], tmdb[80], tipo[40], chamada[1200];
    char temp[16], ep[16];
    jsonTexto(id, sizeof id, p->idScraper ? p->idScraper : "");
    jsonTexto(nome, sizeof nome, p->nomeScraper ? p->nomeScraper : "");
    jsonTexto(tmdb, sizeof tmdb, p->tmdbId ? p->tmdbId : "");
    jsonTexto(tipo, sizeof tipo, p->tipo ? p->tipo : "movie");
    if (p->temporada > 0) snprintf(temp, sizeof temp, "%d", p->temporada); else snprintf(temp, sizeof temp, "null");
    if (p->episodio > 0) snprintf(ep, sizeof ep, "%d", p->episodio); else snprintf(ep, sizeof ep, "null");
    snprintf(chamada, sizeof chamada,
             "(async function(){var ex=globalThis.__ex||{};"
             "var fn=ex.getStreams||(ex.default&&ex.default.getStreams)||globalThis.getStreams;"
             "if(typeof fn!=='function')throw new Error('getStreams not found');"
             "var v=await fn(%s,%s,%s,%s);"
             "return __nuvioMapear(v,%s,%s,%d);})()",
             tmdb, tipo, temp, ep, nome, id, p->maxResultados > 0 ? p->maxResultados : 150);
    prom = JS_Eval(e.ctx, chamada, strlen(chamada), "nuvio-chamada.js", JS_EVAL_TYPE_GLOBAL);
    if (JS_IsException(prom)) { logarExcecao(&e, "getStreams"); prom = JS_UNDEFINED; goto fim; } }

  for (;;) {
    JSPromiseStateEnum st;
    rodarJobs(&e);
    st = JS_PromiseState(e.ctx, prom);
    if (st == JS_PROMISE_FULFILLED) {
      JSValue res = JS_PromiseResult(e.ctx, prom);
      const char *s = JS_ToCString(e.ctx, res);
      if (s) { r->json = strdup(s); JS_FreeCString(e.ctx, s); }
      JS_FreeValue(e.ctx, res);
      break;
    }
    if (st == JS_PROMISE_REJECTED) {
      JSValue res = JS_PromiseResult(e.ctx, prom);
      const char *s = JS_ToCString(e.ctx, res);
      if (e.interrompido) r->estourou = 1;
      snprintf(r->erro, sizeof r->erro, "getStreams: %s", s ? s : "?");
      logar(&e, 2, r->erro);
      if (s) JS_FreeCString(e.ctx, s);
      JS_FreeValue(e.ctx, res);
      break;
    }
    if (e.interrompido || agoraMs() > e.prazo || deveParar(&e)) {
      r->estourou = 1;
      snprintf(r->erro, sizeof r->erro, deveParar(&e) ? "cancelado" : "prazo de %d ms",
               p->prazoMs > 0 ? p->prazoMs : PJ_PRAZO_PADRAO_MS);
      logar(&e, 1, r->erro);
      break;
    }
    if (entregues(&e)) continue;
    if (dispararTimers(&e)) continue;
    if (!e.nFx && !e.nTm) {
      snprintf(r->erro, sizeof r->erro, "getStreams nunca resolveu");
      logar(&e, 1, r->erro);
      break;
    }
    esperar(&e);
  }

fim:
  if (e.interrompido) r->estourou = 1;
  // O teto do runtime e conferido pelo proprio QuickJS (antes do alocador).
  if (e.med.recusou || strstr(r->erro, "out of memory")) r->semMemoria = 1;
  if (src) free(src);
  if (e.cx) {
    // Os fetches em voo sao cancelados e abandonados: o fio que voltar solta o
    // proprio pedido (entregar, com a caixa morta). Os que nem sairam sao
    // soltos aqui.
    for (k = 0; k < e.nFx; k++) {
      Job *j = e.fx[k].job;
      if (!j) continue;
      CANCELAR(j);
      rede_job_cancelar(j->rj);
      if (!e.fx[k].voando) { jobSoltar(j); e.fx[k].job = NULL; }
    }
  }
  if (e.ctx) {
    for (k = 0; k < e.nFx; k++) JS_FreeValue(e.ctx, e.fx[k].cb);
    for (k = 0; k < e.nTm; k++) JS_FreeValue(e.ctx, e.tm[k].fn);
    JS_FreeValue(e.ctx, e.cripto);
    JS_FreeValue(e.ctx, prom);
  }
  if (e.cx) {
    Job *l;
    int ultimo;
    pthread_mutex_lock(&e.cx->m);
    e.cx->morta = 1;
    l = e.cx->prontos; e.cx->prontos = NULL;
    e.cx->refs--;
    ultimo = e.cx->refs == 0;
    pthread_mutex_unlock(&e.cx->m);
    while (l) { Job *n = l->prox; jobSoltar(l); l = n; }
    if (ultimo) caixaSoltar(e.cx);
  }
  free(e.fx); free(e.tm);
  r->memPico = e.med.pico + r->domPico;
  if (e.ctx) JS_FreeContext(e.ctx);
  if (e.rt) JS_FreeRuntime(e.rt);
  r->ms = agoraMs() - t0;
  // GERACAO NA ENTREGA: trocar de perfil/conta depois de o scraper terminar
  // ainda descarta o resultado (o N01 nao retira o que ja foi devolvido).
  // `*cancelado == 2` e CORTE (para de esperar, mas quem ja terminou fica).
  if ((p->cancelado && __atomic_load_n(p->cancelado, __ATOMIC_ACQUIRE) == 1) || (p->parar && p->parar(p->pararU)) ||
      (p->job && rede_job_estado(p->job) != REDE_OK)) {
    r->cancelado = 1; free(r->json); r->json = NULL;
  }
  rede_job_soltar(p->job);
  if (r->json) {
    const char *q = r->json;
    while ((q = strstr(q, "\"url\":")) != NULL) { nRes++; q += 6; }
    r->n = nRes;
  }
  return r->n;
}

void pj_resultado_soltar(PjResultado *r) {
  if (!r) return;
  free(r->json);
  r->json = NULL;
}
