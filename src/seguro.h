// MODO SEGURO: o app desfaz sozinho uma mudanca de ajuste que o derrubou.
//
// O PROBLEMA. Ajustes como "Fileiras da home" em 40, "Itens por fileira" em 24,
// a interface em 4K ou a de vidro gastam memoria e GPU que uma TV de 1 GB pode
// nao ter. Quando o app cai por causa disso, a pessoa reabre e ele cai DE NOVO,
// porque o valor que o derrubou esta gravado no arquivo de ajustes: um laco que
// so se desfaz apagando dados ou reinstalando. O dono pediu que os avisos de
// memoria valham alguma coisa — "se o app fechar, ele volta sozinho ao valor
// anterior".
//
// COMO FUNCIONA. Tres pecas, e este modulo so tem a segunda e a terceira; quem
// sabe quais ajustes sao arriscados e como restaura-los e ajustes.c.
//
//   1. DIARIO. Ao mudar um ajuste ARRISCADO (ajustes.c, RISCOS), ajustes.c
//      chama seguro_mudou(chave, anterior, novo). A mudanca entra no diario
//      (seguro.txt, dados_gravar: atomico, so neste aparelho) como PROVISORIA.
//   2. CONFIRMACAO. Uma mudanca provisoria vira CONFIRMADA quando o app ficou
//      SEG_CONFIRMA_S (3 min) aberto depois dela, ou quando saiu limpo
//      (seguro_encerrar). Confirmada, nunca mais e desfeita por este modulo.
//   3. ARRANQUE. seguro_iniciar recebe o veredito de avisos.c sobre a sessao
//      anterior (nao se despediu = caiu; no Tizen isso ja considera a marca de
//      despedida do localStorage, entao "a TV escondeu o app" nao conta como
//      queda). Se caiu e ha mudanca PROVISORIA, devolve cada uma ao valor
//      anterior pelo gancho `aplicar` e a marca REVERTIDA. A decisao e daqui;
//      o aviso ao usuario e de ajustes.c, que conhece os rotulos.
//
// LACO DE QUEDAS. Sem mudanca provisoria a culpa nao e de um ajuste que se saiba
// apontar (pode ser o catalogo, a rede, a TV). Duas quedas seguidas antes de a
// sessao completar SEG_ESTAVEL_S (1 min) => a proxima sessao roda no PERFIL
// SEGURO: ajustes.c desliga em memoria o que pesa (vidro, 4K, tema imersivo,
// trailers, mais fileiras/itens) SEM tocar no arquivo. Se mesmo assim a sessao
// no perfil seguro cai depressa, os valores seguros passam a ser os gravados
// (SEG_PERSISTIR_SEGURO) e a pessoa e avisada; o que ela tinha fica no aviso.
//
// "ESTAVEL" E UMA GRAVACAO DURAVEL, nao uma batida. O tempo de vida da sessao
// anterior nao pode vir de um arquivo que o Tizen talvez nao tenha descarregado
// para o IndexedDB (dados_gravar_leve), senao toda queda pareceria rapida. Entao
// o diario grava com dados_gravar no momento em que a sessao passa de 60 s
// (estavel=1, rapidas=0) e no de cada confirmacao; a batida leve so alimenta o
// "t=Ns" do log.
#ifndef NV_SEGURO_H
#define NV_SEGURO_H

#define SEG_CONFIRMA_S   180   // provisoria -> confirmada
#define SEG_ESTAVEL_S     60   // sessao que passa disto nao conta como queda rapida
#define SEG_LACO_N         2   // quedas rapidas seguidas que ligam o perfil seguro
#define SEG_MAX_MUD       12
#define SEG_CHAVE         32

enum { SEG_PROVISORIA = 0, SEG_CONFIRMADA = 1, SEG_REVERTIDA = 2 };

typedef struct {
  char  chave[SEG_CHAVE];   // id estavel do ajuste, sem espacos (ver RISCOS)
  int   ant, novo;          // valores no formato do proprio ajuste
  int   estado;             // SEG_*
  int   sessao;             // numero da sessao em que mudou
  long  quando;             // time(NULL): so para a pessoa e o log
  unsigned desde;           // segundos de sessao em que mudou
} SegMud;

// Gancho de restauracao (ajustes.c). Devolve 1 se desfez, 0 se nao coube: o
// ajuste ja tinha outro valor que nao `novo` (a pessoa ja o corrigiu, ou outra
// reversao anterior o tocou) — nesse caso a mudanca so e marcada como resolvida.
typedef int (*SegAplicar)(const char *chave, int novo, int ant);

enum { SEG_NORMAL = 0, SEG_PERFIL_SEGURO, SEG_PERSISTIR_SEGURO };

typedef struct {
  int      modo;                       // SEG_NORMAL / SEG_PERFIL_SEGURO / SEG_PERSISTIR_SEGURO
  int      caiu;                       // a sessao anterior nao se despediu
  int      nRevertidas;
  SegMud   revertidas[SEG_MAX_MUD];    // o que foi desfeito NESTE arranque
  unsigned ultimoT;                    // segundos de vida da sessao anterior no ultimo sinal
  int      rapidas;                    // quedas rapidas seguidas, ja contando esta
  int      sessao;                     // numero desta sessao
} SegDecisao;

// No arranque, DEPOIS de os ajustes serem lidos e de avisos_iniciar ter dado o
// veredito. `agora` = time(NULL). Nunca devolve NULL.
const SegDecisao *seguro_iniciar(int anteriorCaiu, long agora, SegAplicar aplicar);

// Ajuste arriscado mudou. Se `novo` volta ao `ant` original de uma provisoria
// da mesma chave, a entrada some (a pessoa desfez sozinha).
void seguro_mudou(const char *chave, int ant, int novo, long agora, unsigned uptimeS);

// O ajuste mudou SEM ficar mais arriscado (a pessoa baixou 30 para 20, ou voltou
// ao valor de antes). Se ha uma mudanca em prova para a chave: `arriscado` = 0
// (o valor novo ja e seguro) ou `novo` igual ao anterior original a encerra;
// senao ela passa a valer `novo` (reverter 20 para o original, e nao 30).
void seguro_ajustou(const char *chave, int novo, int arriscado);

// Chamar por quadro (barato): a cada ~30 s de sessao grava o sinal leve; aos
// 60 s marca a sessao estavel; confirma o que passou de 3 min.
void seguro_batida(unsigned uptimeS);

// Saida limpa: confirma tudo, fecha a sessao no diario.
void seguro_encerrar(void);

// 1 se esta sessao roda no perfil seguro (ajustes.c consulta a cada leitura).
int  seguro_perfil_ativo(void);

// Avisos de primeira vez ("Mais fileiras usam mais memoria..."): uma mascara de
// bits no diario, por aparelho. Visto = a folha ja foi mostrada e aceita.
enum { SEG_AVISO_FILEIRAS = 1, SEG_AVISO_ITENS = 2 };
int  seguro_aviso_visto(int bit);
void seguro_aviso_marcar(int bit);

// Leitura (tela de Ajustes, testes).
int  seguro_n_mud(void);
const SegMud *seguro_mud(int i);
int  seguro_n_provisorias(void);

#define SEG_ARQ "seguro.txt"

#endif
