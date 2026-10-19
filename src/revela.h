// Tres micro-animacoes de card, sem estado global e sem alocacao: cada tela
// guarda os proprios registros (um por lugar na grade) e pergunta a cada
// quadro. Todas obedecem a anim_politica_reduzida (anim.h): com animacoes
// reduzidas elas devolvem o estado final no mesmo quadro.
//
//   revela_arte   a ARTE CHEGANDO. O card que esperou com o esqueleto na tela
//                 recebe a textura e aparece por ~220 ms de esvanecimento, em
//                 vez de trocar o cinza pela foto num quadro so (o "pop-in").
//                 O card cuja arte ja estava no cache NUNCA esvanece: so quem
//                 foi visto esperando ganha a entrada, entao rolar uma fileira
//                 ja carregada continua instantaneo.
//
//   revela_varre  a LUZ DO FOCO. O especular do GFX_CARD (a faixa diagonal
//                 clara do cartaz em foco) entra deslizando do canto inferior
//                 esquerdo ate o lugar de repouso, uma vez por foco. Com a
//                 tecla presa (passos a menos de NV_VARRE_REPETE_MS) a luz ja
//                 nasce assentada: uma varredura por card atravessado seria
//                 ruido. Custo: um uniforme a mais no desenho que ja existe.
//
//   revela_entra  a FILEIRA CHEGANDO. Cada card sobe alguns pixels e ganha
//                 opacidade com um atraso proporcional a coluna VISIVEL (o
//                 "stagger"). Nao ha passada extra: e o mesmo desenho com a
//                 opacidade de grupo e o y deslocados. Nas GRADES (Ver
//                 tudo, Biblioteca, Busca) o atraso soma coluna e fileira
//                 visiveis (revela_onda_atraso): a "onda" da primeira vez.
//
//   revela_progresso  a BARRA ANDANDO. A barra de progresso (Continuar
//                 assistindo, episodio no detalhe) cresce do valor que ja
//                 mostrava ate o novo, em vez de saltar. Um valor visto pela
//                 primeira vez na sessao nasce parado.
#ifndef NV_REVELA_H
#define NV_REVELA_H
#include "anim.h"
#include <SDL2/SDL.h>

#define NV_REVELA_MS        220.0f
#define NV_VARRE_MS         520.0f
#define NV_VARRE_REPETE_MS  180u
// Distancia de partida da faixa, na unidade do shader (projecao na diagonal
// do card; o card inteiro cobre ~+-0.68). 1.0 nasce fora do canto.
#define NV_VARRE_INICIO     1.0f
#define NV_ENTRA_MS         300.0f
#define NV_ENTRA_PASSO_MS    38.0f   // atraso entre colunas visiveis
#define NV_ENTRA_FIL_MS      70.0f   // atraso entre fileiras visiveis
#define NV_ENTRA_MAX_COL        7    // depois disso o atraso para de crescer
#define NV_ENTRA_DY          22.0f   // quanto o card sobe ate assentar

// Saida rapida e cauda longa, sem passar do ponto: o "ease-out" exponencial
// que tvOS e Google TV usam para o que ENTRA. Cubica, e nao expf, porque tem
// fim exato em t = 1.
static inline float revela_saida(float t) {
  t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
  float u = 1.0f - t;
  return 1.0f - u * u * u;
}

typedef struct {
  Uint32 chegou;           // quando a textura apareceu depois da espera
  unsigned char esperou;   // 1 = este lugar ja mostrou o esqueleto
} RevelaArte;

// Chame com `temArte` = a textura existe neste quadro. Devolve a opacidade
// da arte (1 = assentada). Enquanto for < 1, desenhe o esqueleto por baixo.
static inline float revela_arte(RevelaArte *r, int temArte, Uint32 agora) {
  if (!temArte) { r->esperou = 1; r->chegou = 0; return 0.0f; }
  if (!r->esperou) return 1.0f;
  if (anim_politica_reduzida) { r->esperou = 0; return 1.0f; }
  if (!r->chegou) r->chegou = agora ? agora : 1u;
  float t = (float)(agora - r->chegou) / NV_REVELA_MS;
  if (t >= 1.0f) { r->esperou = 0; r->chegou = 0; return 1.0f; }
  return revela_saida(t);
}

typedef struct {
  int alvo;                // identidade do foco atual (-1 = nenhum)
  Uint32 desde;            // quando a varredura comecou (0 = assentada)
  Uint32 ultimoPasso;
} RevelaVarre;

// `alvo` identifica o lugar em foco (ex.: fileira*64+coluna). Devolve o
// deslocamento da faixa para gfx_varre_atual: NV_VARRE_INICIO no primeiro
// quadro, 0 assentada.
static inline float revela_varre(RevelaVarre *v, int alvo, Uint32 agora) {
  if (alvo != v->alvo) {
    int repete = v->ultimoPasso && agora - v->ultimoPasso < NV_VARRE_REPETE_MS;
    v->alvo = alvo;
    v->ultimoPasso = agora;
    v->desde = (repete || alvo < 0 || anim_politica_reduzida) ? 0u : (agora ? agora : 1u);
  }
  if (!v->desde) return 0.0f;
  if (anim_politica_reduzida) { v->desde = 0; return 0.0f; }
  float t = (float)(agora - v->desde) / NV_VARRE_MS;
  if (t >= 1.0f) { v->desde = 0; return 0.0f; }
  return NV_VARRE_INICIO * (1.0f - revela_saida(t));
}

// Progresso 0..1 da entrada de um card: `inicio` e quando a fileira comecou a
// entrar (0 = ja entrou), `ordem` a posicao visivel (coluna + fileiras).
static inline float revela_entra(Uint32 inicio, float atrasoMs, Uint32 agora) {
  if (!inicio || anim_politica_reduzida) return 1.0f;
  // Com sinal: `inicio` pode estar no futuro (atraso da fileira).
  float t = ((float)(Sint32)(agora - inicio) - atrasoMs) / NV_ENTRA_MS;
  if (t <= 0.0f) return 0.0f;
  return revela_saida(t);
}

// ONDA DE GRADE: atraso de um card pela coluna e pela fileira VISIVEIS.
// As duas param de crescer (NV_ENTRA_MAX_COL, NV_ONDA_MAX_FIL) para a onda
// nunca demorar mais que ~0,8 s, mesmo numa grade de 5x4.
#define NV_ONDA_MAX_FIL 4
static inline float revela_onda_atraso(int coluna, int fileira) {
  if (coluna < 0) coluna = 0;
  if (coluna > NV_ENTRA_MAX_COL) coluna = NV_ENTRA_MAX_COL;
  if (fileira < 0) fileira = 0;
  if (fileira > NV_ONDA_MAX_FIL) fileira = NV_ONDA_MAX_FIL;
  return coluna * NV_ENTRA_PASSO_MS + fileira * NV_ENTRA_FIL_MS;
}
// A onda que comecou em `inicio` ja assentou (todo card, ate o ultimo
// atrasado)? Quem chama zera o inicio e os cards passam a perguntar de graca.
static inline int revela_onda_fim(Uint32 inicio, Uint32 agora) {
  return !inicio || anim_politica_reduzida ||
         (Sint32)(agora - inicio) >
           (Sint32)(NV_ENTRA_MS + revela_onda_atraso(NV_ENTRA_MAX_COL, NV_ONDA_MAX_FIL) + 40.0f);
}

// --- BARRA DE PROGRESSO ANIMADA ----------------------------------------------
//
// Uma tabela pequena por arquivo (a funcao e static inline, entao cada tela
// guarda o que ELA mostrou): chave imdb+temporada+episodio, o valor de onde a
// barra partiu, o alvo e quando comecou. A chave que nao esta na tabela entra
// ja assentada — o primeiro desenho da sessao nao anima; e o valor que MUDA
// (a volta do player) que anda. Cheia, a tabela despeja o menos usado.
#define NV_PROGRESSO_MS  520.0f
#define NV_PROGRESSO_N       48

typedef struct {
  char chave[48];
  float de, para;          // em 0..100, como CatItem.progresso
  Uint32 desde;            // 0 = assentada em `para`
  Uint32 uso;
} RevelaProgresso;

// Valor exibido de uma entrada ao instante `agora` (pura; testada sozinha).
static inline float revela_progresso_valor(const RevelaProgresso *p, Uint32 agora) {
  if (!p->desde || anim_politica_reduzida) return p->para;
  float t = (float)(Sint32)(agora - p->desde) / NV_PROGRESSO_MS;
  if (t >= 1.0f) return p->para;
  if (t <= 0.0f) return p->de;
  return p->de + (p->para - p->de) * revela_saida(t);
}

// Passo sobre uma tabela dada: devolve o valor a desenhar para `alvo`.
static inline float revela_progresso_em(RevelaProgresso *tab, int n,
                                        const char *imdb, int temporada,
                                        int episodio, float alvo, Uint32 agora) {
  char chave[48];
  int i, livre = 0;
  if (!imdb || !imdb[0]) return alvo;
  SDL_snprintf(chave, sizeof chave, "%s:%d:%d", imdb, temporada, episodio);
  for (i = 0; i < n; i++) {
    RevelaProgresso *p = &tab[i];
    if (p->chave[0] && !SDL_strcmp(p->chave, chave)) {
      p->uso = agora ? agora : 1u;
      if (alvo != p->para) {
        // Parte de onde a barra ESTA, e nao do alvo velho: um valor novo no
        // meio da animacao continua sem salto.
        p->de = revela_progresso_valor(p, agora);
        p->para = alvo;
        p->desde = anim_politica_reduzida ? 0u : (agora ? agora : 1u);
      }
      if (p->desde && (anim_politica_reduzida ||
                       (float)(Sint32)(agora - p->desde) >= NV_PROGRESSO_MS))
        p->desde = 0;
      return revela_progresso_valor(p, agora);
    }
    if (!p->chave[0]) livre = i;
    else if (tab[livre].chave[0] && p->uso < tab[livre].uso) livre = i;
  }
  // Primeira vez: entra assentada, sem animar.
  SDL_snprintf(tab[livre].chave, sizeof tab[livre].chave, "%s", chave);
  tab[livre].de = tab[livre].para = alvo;
  tab[livre].desde = 0;
  tab[livre].uso = agora ? agora : 1u;
  return alvo;
}

// A tabela desta tela (uma por arquivo que inclui o cabecalho).
static inline float revela_progresso(const char *imdb, int temporada, int episodio,
                                     float alvo, Uint32 agora) {
  static RevelaProgresso tab[NV_PROGRESSO_N];
  return revela_progresso_em(tab, NV_PROGRESSO_N, imdb, temporada, episodio,
                             alvo, agora);
}

#endif
