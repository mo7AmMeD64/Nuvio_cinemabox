#ifndef NUVIO_AJUSTES_UX_H
#define NUVIO_AJUSTES_UX_H

typedef struct {
  int op;
  char titulo[160];
  char caminho[160];
  char valor[160];
  int avancado;
  int bloqueado;
  char icone[40];     /* o icone da secao (aj_*) */
  char ajuda[300];    /* a frase da opcao, para o melhor resultado */
} AjusteBuscaResultado;

/* A previa da opcao no melhor resultado da busca: a arte do titulo com o
 * valor atual por cima (Spotlight no modo Ajustes). */
void ajustes_previa_busca(int op, float x, float y, float w, float h, float a);

/* Arte local usada nas amostras ilustrativas, sem rede. */
void ajustes_recursos(const char *dirArte);

/* Consulta apenas o catalogo local de Ajustes, sem rede ou valores privados. */
int ajustes_buscar(const char *consulta, AjusteBuscaResultado *resultados, int capacidade);
void ajustes_abrir_opcao(int op);
/* Consumido: 1 inicia busca; 2 retorna à consulta anterior. */
int ajustes_pediu_busca(void);

#endif
