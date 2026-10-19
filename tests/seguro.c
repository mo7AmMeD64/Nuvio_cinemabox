// Modo seguro (src/seguro.c): diario de mudancas arriscadas, reversao no arranque
// depois de uma queda, confirmacao por tempo ou por saida limpa, e o laco de
// quedas que liga o perfil seguro. So a logica: os arquivos sao uma tabela em
// memoria no lugar de dados.c, e `caiu` (o veredito de avisos.c) e um parametro.
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/seguro.h"

// --- dados.h falso: um unico arquivo em memoria ------------------------------
static char arq[4096]; static int temArq;
char *dados_ler(const char *n) { (void)n; return temArq ? strdup(arq) : NULL; }
int dados_gravar(const char *n, const char *c) { (void)n; snprintf(arq, sizeof arq, "%s", c); temArq = 1; return 1; }
int dados_gravar_leve(const char *n, const char *c) { return dados_gravar(n, c); }

// --- o "ajuste": um valor por chave, como ajustes.c faz -----------------------
static int fileiras = 7, itens = 0, vidro = 1;
static int aplicados;
static int aplicar(const char *chave, int novo, int ant) {
  int *v = !strcmp(chave, "filLimite") ? &fileiras
         : !strcmp(chave, "itensFileira") ? &itens
         : !strcmp(chave, "vidro") ? &vidro : NULL;
  if (!v || *v != novo) return 0;
  *v = ant; aplicados++;
  return 1;
}
static void reinicia_disco(void) { temArq = 0; arq[0] = 0; fileiras = 7; itens = 0; vidro = 1; aplicados = 0; }
#define OK(c) do { if (!(c)) { fprintf(stderr, "FALHOU: %s (linha %d)\n", #c, __LINE__); exit(1); } } while (0)

// Um "processo": arranca com o veredito de queda, devolve a decisao.
static const SegDecisao *arranca(int caiu) { return seguro_iniciar(caiu, 1000, aplicar); }
// A mudanca que a tela de Ajustes faria: aplica o valor e notifica o modulo.
static void muda_fileiras(int novo, unsigned t) { int ant = fileiras; fileiras = novo; seguro_mudou("filLimite", ant, novo, 1000, t); }

int main(void) {
  const SegDecisao *d;

  // 1. mudou -> caiu -> proximo arranque desfaz.
  reinicia_disco();
  d = arranca(0);                       // primeiro arranque: sem diario
  OK(d->modo == SEG_NORMAL && d->nRevertidas == 0);
  muda_fileiras(30, 20);
  OK(seguro_n_provisorias() == 1);
  seguro_batida(40);                    // 20 s depois: ainda em prova
  OK(seguro_n_provisorias() == 1);
  d = arranca(1);                       // caiu sem se despedir
  OK(d->nRevertidas == 1 && !strcmp(d->revertidas[0].chave, "filLimite"));
  OK(d->revertidas[0].novo == 30 && d->revertidas[0].ant == 7);
  OK(fileiras == 7);
  OK(d->modo == SEG_NORMAL);            // culpado apontado => nada de perfil seguro
  OK(seguro_mud(0)->estado == SEG_REVERTIDA);
  // O aviso e uma vez so: outro arranque limpo nao repete a reversao.
  d = arranca(0);
  OK(d->nRevertidas == 0 && fileiras == 7);

  // 2. mudou -> 3 min -> confirmada -> caiu: NAO desfaz.
  reinicia_disco();
  arranca(0);
  muda_fileiras(24, 10);
  seguro_batida(10 + SEG_CONFIRMA_S - 1);
  OK(seguro_n_provisorias() == 1);
  seguro_batida(10 + SEG_CONFIRMA_S);
  OK(seguro_n_provisorias() == 0 && seguro_mud(0)->estado == SEG_CONFIRMADA);
  d = arranca(1);
  OK(d->nRevertidas == 0 && fileiras == 24 && aplicados == 0);

  // 3. saida limpa confirma, mesmo antes dos 3 min.
  reinicia_disco();
  arranca(0);
  muda_fileiras(24, 5);
  seguro_encerrar();
  d = arranca(0);
  OK(d->nRevertidas == 0 && fileiras == 24 && seguro_mud(0)->estado == SEG_CONFIRMADA);
  // ...e uma queda depois disso nao volta atras no que ja passou.
  d = arranca(1);
  OK(d->nRevertidas == 0 && fileiras == 24);

  // 4. varias mexidas no mesmo ajuste: vale o anterior ORIGINAL; voltar ao
  //    original limpa a entrada.
  reinicia_disco();
  arranca(0);
  muda_fileiras(20, 5); muda_fileiras(30, 6);
  OK(seguro_n_mud() == 1 && seguro_mud(0)->ant == 7 && seguro_mud(0)->novo == 30);
  muda_fileiras(7, 7);
  OK(seguro_n_mud() == 0);
  muda_fileiras(30, 8);
  d = arranca(1);
  OK(d->nRevertidas == 1 && fileiras == 7);

  // 5. dois ajustes em prova: os dois voltam.
  reinicia_disco();
  arranca(0);
  muda_fileiras(30, 5);
  { int a = itens; itens = 2; seguro_mudou("itensFileira", a, 2, 1000, 6); }
  { int a = vidro; vidro = 0; seguro_mudou("vidro", a, 0, 1000, 7); }
  d = arranca(1);
  OK(d->nRevertidas == 3 && fileiras == 7 && itens == 0 && vidro == 1);

  // 6. o valor ja mudou por fora (a pessoa corrigiu antes de cair): nao inventa.
  reinicia_disco();
  arranca(0);
  muda_fileiras(30, 5);
  fileiras = 10;                        // outro caminho gravou 10
  d = arranca(1);
  OK(d->nRevertidas == 0 && fileiras == 10);

  // 7. LACO DE QUEDAS sem culpado: 2 quedas rapidas seguidas => perfil seguro
  //    SO nesta sessao; sessao estavel no perfil seguro => volta ao normal.
  reinicia_disco();
  arranca(0);                           // s1: cai aos 10 s
  d = arranca(1);                       // s2 (1a queda rapida)
  OK(d->modo == SEG_NORMAL && d->rapidas == 1);
  d = arranca(1);                       // s3 (2a queda rapida)
  OK(d->modo == SEG_PERFIL_SEGURO && seguro_perfil_ativo());
  seguro_batida(75);                    // a sessao segura aguenta
  d = arranca(1);                       // cai depois de estavel: nao e queda rapida
  OK(d->modo == SEG_NORMAL && !seguro_perfil_ativo());
  d = arranca(0);
  OK(d->modo == SEG_NORMAL);

  // 8. o perfil seguro tambem cai depressa: os valores seguros passam a valer.
  reinicia_disco();
  arranca(0); arranca(1);
  d = arranca(1);
  OK(d->modo == SEG_PERFIL_SEGURO);
  d = arranca(1);                       // caiu de novo, ainda no perfil seguro
  OK(d->modo == SEG_PERSISTIR_SEGURO && !seguro_perfil_ativo());
  d = arranca(1);                       // um contador zerado: recomeca do 1
  OK(d->modo == SEG_NORMAL && d->rapidas == 1);

  // 9. saida limpa zera o laco.
  reinicia_disco();
  arranca(0); arranca(1);               // 1 queda rapida
  seguro_encerrar();
  arranca(0);
  d = arranca(1);
  OK(d->modo == SEG_NORMAL && d->rapidas == 1);

  // 10. queda sem diario (primeiro arranque desta versao) nao conta como rapida.
  reinicia_disco();
  d = arranca(1); OK(d->rapidas == 0);
  d = arranca(1); OK(d->rapidas == 1);  // agora o diario prova que foi rapida

  // 11. o que a TV escondeu (caiu=0 mesmo com o diario aberto) e confirmado.
  reinicia_disco();
  arranca(0);
  muda_fileiras(30, 5);
  d = arranca(0);                       // avisos.c disse "se despediu" (Tizen)
  OK(d->nRevertidas == 0 && fileiras == 30 && seguro_mud(0)->estado == SEG_CONFIRMADA);

  // 12. avisos de primeira vez sobrevivem ao reinicio.
  reinicia_disco();
  arranca(0);
  OK(!seguro_aviso_visto(SEG_AVISO_FILEIRAS));
  seguro_aviso_marcar(SEG_AVISO_FILEIRAS);
  arranca(0);
  OK(seguro_aviso_visto(SEG_AVISO_FILEIRAS) && !seguro_aviso_visto(SEG_AVISO_ITENS));

  // 13. arquivo corrompido nao impede o app de abrir.
  reinicia_disco();
  snprintf(arq, sizeof arq, "lixo\nm x y\nc 1\n\xff\xfe\n"); temArq = 1;
  d = arranca(1);
  OK(d->nRevertidas == 0);

  printf("seguro: tudo ok\n");
  return 0;
}
