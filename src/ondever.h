// ONDE ASSISTIR: os servicos de streaming que tem o titulo NO PAIS DA PESSOA,
// para a folha de Fontes oferecer "abrir na Netflix" ao lado das fontes dos
// addons — para quem nao tem addon de fonte, ou prefere o app oficial.
//
// TRES PECAS:
//   - o PAIS: pela rede (Cloudflare devolve o pais do IP), guardado em
//     pais.txt por uma semana. Sem rede, a regiao do idioma do TMDB ("pt-BR"
//     -> BR); sem nada, US. Nao e um ajuste: o catalogo da Netflix e o do
//     lugar onde a TV esta, nao o da lingua que a pessoa escolheu.
//   - a LISTA: /movie|tv/<id>/watch/providers do TMDB, so a regiao do pais.
//     Entra o que e assistir sem pagar a parte: assinatura (flatrate), gratis
//     (free) e com anuncio (ads). Aluguel e compra ficam de fora — "ir direto"
//     nao e comprar. Variantes do mesmo servico ("Netflix Standard with Ads")
//     viram uma linha so.
//   - O APP: o instalado na TV cujo NOME casa com o servico (LG: listApps;
//     Samsung: tizen.application no .wgt, ApplicationManager no .tpk;
//     Android: o PackageManager, por src/android.c). Nenhum
//     id fixo para ABRIR — "HBO Max" na LG e com.wbd.stream hoje e pode
//     mudar. Nao instalado: a pagina dele na loja da TV, quando o id na loja
//     e conhecido (tabela em ondever.c); senao, a frase "procure na loja".
#ifndef NV_ONDEVER_H
#define NV_ONDEVER_H

#define ONDEVER_MAX 8

typedef struct {
  char nome[64];     // "Netflix", "HBO Max" — como o TMDB escreve
  char logo[160];    // URL do logo quadrado do TMDB (w92)
  int  gratis;       // 1 = free/ads; 0 = assinatura
} OndeVer;

// O que a linha do servico faz no OK, nesta TV, agora.
enum {
  ONDE_INFO = 0,     // so mostra onde esta (plataforma nao abre outro app)
  ONDE_ABRIR,        // app instalado: abre
  ONDE_LOJA,         // nao instalado, id na loja conhecido: abre a pagina dele
  ONDE_PROCURAR,     // nao instalado, sem id: abre a loja e diz o que procurar
};

// Comeca a descobrir o pais e a lista de apps (fios proprios). Chamar uma
// vez, do fio principal, depois de dados_iniciar.
void ondever_iniciar(void);
// Codigo ISO de duas letras ("BR"). Nunca vazio.
void ondever_pais(char *out, unsigned capacity);

// Pede os servicos do titulo. Nao bloqueia. Repetir o mesmo id nao refaz.
// `imdb` aceita "tt123:1:2" (o episodio e ignorado) e "tmdb:123"; outros ids
// (canal ao vivo) nao tem servico e nao saem para a rede.
void ondever_pedir(const char *imdb, int serie, long tmdbId);
// Quantos servicos para `imdb` (0 = nenhum, ainda nao chegou, ou a lista e
// de outro titulo). Copia o i-esimo em *dst.
int  ondever_n(const char *imdb);
int  ondever_item(const char *imdb, int i, OndeVer *dst);

// Rele os apps instalados (a pessoa pode ter acabado de instalar um). Do fio
// principal; a resposta chega depois. Android usa um unico fio e publica a
// lista completa; enquanto a primeira consulta chega, o estado e ONDE_INFO.
void ondever_apps_atualizar(void);
// ONDE_* para o servico `nome`.
int  ondever_estado(const char *nome);
// Faz o que ondever_estado diz. Devolve o ONDE_* que fez.
int  ondever_abrir(const char *nome);

// Exposto para o teste: a chave que casa servico com app ("HBO Max" -> "max",
// "Disney+" -> "disneyplus", "Amazon Prime Video" -> "primevideo").
void ondever_chave(const char *nome, char *dst, unsigned tam);
int  ondever_casa(const char *servico, const char *app);
// Para o teste e para o .tpk: acrescenta um app instalado a lista.
void ondever_app_visto(const char *id, const char *nome);
void ondever_apps_limpar(void);

// Offline parser and request state; no title deep links are implied.
int ondever_extrair(const char *json, const char *country, OndeVer *out, int capacity);
enum { ONDE_SEM_PEDIDO, ONDE_BUSCANDO, ONDE_PRONTO, ONDE_FALHOU };
int ondever_status(const char *id);
#ifdef NV_SHOT_HOOKS
void ondever_shot(const char *imdb, const OndeVer *l, int n);
#endif
#endif
