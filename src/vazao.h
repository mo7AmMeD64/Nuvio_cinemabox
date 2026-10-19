// TESTE DE VELOCIDADE: a CONTA, sem rede, sem SDL e sem arquivo.
//
// O pedido do dono: "medir a velocidade dos stream e addon para poder dizer o
// tamanho maximo e otimo dos titulos para tocar sem parar". A medida em si
// (baixar um trecho de fontes de verdade e contar bytes por segundo) mora em
// rede.c (rede_medir_vazao) e em diagnostico.c; aqui fica so o que transforma
// as amostras por segundo em "ate 12 GB por filme de 2 h", para
// tests/vazao.sh prender a regra sem aparelho.
//
// A REGRA, e por que cada numero:
//   - OTIMO  = p20 x 0,75. O p20 e o "segundo ruim" tipico da conexao: 80% dos
//     segundos medidos foram mais rapidos que ele. Um arquivo cuja taxa media
//     cabe em 75% disso toca sem parar mesmo quando a rede cai para o pior
//     trecho comum — a folga de 25% e o que o pico de bitrate de uma cena
//     pesada (remux chega a 2x a media) come antes de o buffer esvaziar.
//   - MAXIMO = mediana x 0,9. Cabe na velocidade tipica, mas nos trechos
//     ruins o buffer esvazia e o player pode pausar para carregar.
//   - GB = Mbps x segundos / 8 / 1000 (GB decimal, que e como addon e debrid
//     escrevem o tamanho do arquivo). Filme de 2 h = 7200 s; episodio de
//     45 min = 2700 s.
#ifndef NV_VAZAO_H
#define NV_VAZAO_H
#include <stddef.h>

// Amostras por fonte (1 por segundo) e no total (3 fontes).
#define VAZAO_SEG_MAX     16
#define VAZAO_FONTES_MAX  3
#define VAZAO_AMOSTRAS_MAX (VAZAO_SEG_MAX * VAZAO_FONTES_MAX)

#define VAZAO_FILME_S    7200   // 2 h
#define VAZAO_EPISODIO_S 2700   // 45 min

typedef struct {
  int n;            // amostras usadas
  int medianaKbps;
  int p20Kbps;
  int otimoKbps;    // p20 x 0,75
  int maximoKbps;   // mediana x 0,9
} VazaoResumo;

// Mediana e p20 (posicao mais proxima: o valor na posicao ceil(0,2 n) da
// lista ordenada) das amostras em kbps. Amostra negativa e ignorada; zero
// CONTA — um segundo sem byte nenhum e exatamente o trecho ruim que o otimo
// precisa enxergar. Devolve 0 (e zera `r`) sem amostra valida.
int vazao_resumir(const int *kbps, int n, VazaoResumo *r);

// GB decimais de `kbps` sustentados por `segundos`.
double vazao_gb(int kbps, int segundos);

// "38" / "4,5" / "0,8": uma casa abaixo de 10, inteiro dali para cima.
// `sep` e o separador decimal do idioma (',' ou '.').
void vazao_fmt_mbps(char *dst, size_t n, int kbps, char sep);
void vazao_fmt_gb(char *dst, size_t n, double gb, char sep);

// Endereco de AVISO e nao de conteudo, o mesmo criterio da verificacao de
// fonte (streams.c) mais os clipes de erro que o player descarta pela duracao:
// slate do AIOStreams/ElfHosted, downloading.mp4 do Debridio e os clipes por
// codigo de erro do static.debridio.com. Medir um desses mediria o servidor de
// aviso, nao a fonte.
int vazao_url_aviso(const char *url);

// "https://a.b:443" de "https://a.b:443/x?y" — a chave de "hosts diferentes".
// So o esquema e o host; o caminho (onde vai a chave do debrid) nunca sai
// daqui. Devolve 0 sem "://".
int vazao_host(const char *url, char *dst, size_t n);

// Dica de uma linha para a taxa ideal, em portugues (a tela passa por i18n).
const char *vazao_dica(int otimoKbps);


// ---------------------------------------------------------------------------
// CICLO COMPLETO E "POR ADD-ON": a regra, sem rede.
//
// O teste rapido mede ate 3 fontes de hosts diferentes. O dono perguntou se
// "so testa 3 fontes" e pediu um modo que teste TODAS (ou uma por add-on) para
// comparar. O que mora aqui e o que da para prender sem aparelho: quais fontes
// entram, em que ordem, uma por vez, com cancelamento e orcamento de tempo, e
// como o resultado vira ranking e "toca ou nao toca" para o bitrate da fonte.
// Quem baixa de verdade (rede_medir_vazao) entra pelo callback `medir`.
//
// UMA POR VEZ, sempre: medir duas juntas divide a banda e mede a divisao, e
// muitos provedores de debrid limitam as conexoes simultaneas por conta.
#define VAZ_CICLO_MAX        40      // fontes medidas num ciclo completo
#ifndef VAZ_CICLO_JANELA_S
#define VAZ_CICLO_JANELA_S   5       // corpo por fonte (o rapido usa 8 s)
#endif
#define VAZ_CICLO_ESPERA_MS  6000UL  // prazo do 1o byte de cada fonte
#define VAZ_CICLO_ORCAMENTO_MS (8UL * 60UL * 1000UL)
#define VAZ_CICLO_DEBRID_MAX 16      // fontes "nao testada (debrid)" listadas
#define VAZ_CICLO_LISTA_MAX  (VAZ_CICLO_MAX + VAZ_CICLO_DEBRID_MAX)

typedef enum { VCM_RAPIDO = 0, VCM_COMPLETO, VCM_ADDON } VazCicloModo;

// O que aconteceu com uma fonte. VS_DEBRID = exigiria o debrid resolver ou
// baixar o arquivo (torrent sem link, "fora de cache"): nao e tocada.
typedef enum {
  VS_PENDENTE = 0, VS_OK, VS_FALHOU, VS_AVISO, VS_HOST_REPETIDO, VS_DEBRID
} VazSit;

typedef enum { VSU_SEM_REF = 0, VSU_OK, VSU_JUSTO, VSU_NAO } VazSuf;

// Uma candidata, sem o link: so o que a selecao precisa.
typedef struct {
  int addon;              // indice do add-on de onde veio
  unsigned long chave;    // vazao_chave do link; 0 = nao deduplica
  unsigned char medivel;  // 0 = exige o debrid (nao entra)
} VazItem;

typedef struct {
  int candidatas;   // itens recebidos
  int debrid;       // fora por exigir o debrid
  int duplicadas;   // fora por repetirem o link de outra
  int foraDoLimite; // mediveis que passaram do teto
  int fila;         // quantas entraram
} VazSelecao;

// Escolhe e ORDENA quem sera medido. `it` vem na ordem da fonte automatica.
//   VCM_RAPIDO   todas as mediveis na ordem (o chamador para nas 3 de hosts
//                diferentes: o host so se conhece depois de resolver o link);
//   VCM_COMPLETO revezando entre os add-ons (1a de cada, 2a de cada...), para
//                o teto de `cap` nao deixar o ultimo add-on sem nenhuma;
//   VCM_ADDON    a melhor de cada add-on, na ordem dos add-ons.
// `fila` recebe indices de `it` (capacidade `cap`); devolve quantos.
int vazao_selecionar(VazCicloModo modo, const VazItem *it, int n, int cap,
                     int *fila, VazSelecao *s);

// Hash (FNV-1a) do link, para reconhecer o mesmo link duas vezes sem guardar
// nem imprimir o link.
unsigned long vazao_chave(const char *url);

// Host publico de um link para a TELA e o RELATORIO: so "host[:porta]", sem
// esquema, sem usuario:senha@, sem caminho, consulta ou fragmento — a chave do
// debrid vai no caminho. Devolve 0 (dst vazio) sem "://".
int vazao_host_publico(const char *url, char *dst, size_t n);

// O AGENDADOR. Percorre `fila` UMA fonte por vez chamando ops->medir, que
// devolve o VazSit e guarda o resultado onde quiser.
typedef struct {
  int (*medir)(int indice, void *u);          // -> VazSit
  int (*cancelado)(void *u);                  // != 0: para agora
  unsigned long (*agora)(void *u);            // ms
  void (*passo)(void *u, int feitos, int total);  // depois de cada fonte
  void *u;
} VazOps;

typedef struct {
  int maxMedidas;            // 0 = todas; o rapido para nas 3
  int maxTentativas;         // 0 = sem teto; o rapido tenta ate 6
  unsigned long orcamentoMs; // 0 = sem orcamento
  int orcamentoAposMedida;   // 1 = so vale depois da 1a medida (o rapido)
} VazPlano;

typedef struct {
  int tentadas, medidas, falhas, hostRepetido, restantes;
  int cancelado, semTempo;
} VazAgenda;

void vazao_agendar(const VazPlano *pl, const int *fila, int n, const VazOps *ops,
                   VazAgenda *out);

// Bitrate que a fonte pede, em kbps: o do arquivo (tamanho / duracao) quando o
// nome traz o tamanho; senao um valor tipico da altura (4K ~25 Mbps, 1440p 16,
// 1080p 8, 720p 4, menos 2,5). 0 = sem nenhuma pista.
int vazao_necessario_kbps(int altura, long tamanhoMB, int duracaoS);

// Compara com o que a fonte ENTREGOU (a mesma regra do teste rapido): o otimo
// (p20 x 0,75) cobre o bitrate -> toca sem parar; so o maximo (mediana x 0,9)
// cobre -> pode pausar; nenhum -> insuficiente. Sem bitrate: VSU_SEM_REF.
VazSuf vazao_suficiencia(const VazaoResumo *r, int necessarioKbps);

// Uma medida de fonte para o ranking.
typedef struct {
  int addon;
  int sit;                 // VazSit
  int altura, dv;
  long tamanhoMB;
  char nome[64];           // rotulo da fonte
  char host[96];           // vazao_host_publico do link FINAL
  int esperaMs;            // do pedido ao 1o byte
  int http;
  VazaoResumo r;
  int necessarioKbps;
  int suf;                 // VazSuf
} VazCicloRes;

// Ordem de exibicao: medidas por vazao (mediana) decrescente, empate pela
// menor espera; depois as que falharam; por fim as do debrid. Estavel.
void vazao_ordenar(const VazCicloRes *r, int n, int *ordem);

// Mediana das medianas das fontes VS_OK do add-on; *usadas = quantas. 0 sem nenhuma.
int vazao_mediana_addon(const VazCicloRes *r, int n, int addon, int *usadas);

#endif
