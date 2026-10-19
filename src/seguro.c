// Modo seguro: o diario de mudancas arriscadas e a decisao de arranque. A regra
// e o cabecalho (seguro.h); aqui ficam so o formato do arquivo e a maquina de
// estados.
//
// FORMATO de seguro.txt (texto, uma linha por registro, gravado inteiro e
// atomicamente por dados_gravar):
//   nvseguro 1
//   c <sessao> <rapidas> <aberta> <estavel> <perfilSeguro> <uptime> <avisos>
//   m <sessao> <quando> <estado> <ant> <novo> <desde> <chave>
// `c` e o estado da sessao mais recente; `m` uma mudanca. Linha que nao casa e
// ignorada: um arquivo de outra versao nao pode impedir o app de abrir.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "seguro.h"
#include "dados.h"

static struct {
  int sessao, rapidas, aberta, estavel, perfilSeguro;
  unsigned uptime, avisos;
} S;
static SegMud mud[SEG_MAX_MUD];
static int nMud;
static SegDecisao dec;
static int iniciado;
static unsigned ultimaBatida;

static void salvar(int leve) {
  char buf[SEG_MAX_MUD * 96 + 128];
  size_t u = 0;
  int i;
  u += (size_t)snprintf(buf + u, sizeof buf - u, "nvseguro 1\n");
  u += (size_t)snprintf(buf + u, sizeof buf - u, "c %d %d %d %d %d %u %u\n",
                        S.sessao, S.rapidas, S.aberta, S.estavel,
                        S.perfilSeguro, S.uptime, S.avisos);
  for (i = 0; i < nMud && u + 96 < sizeof buf; i++)
    u += (size_t)snprintf(buf + u, sizeof buf - u, "m %d %ld %d %d %d %u %s\n",
                          mud[i].sessao, mud[i].quando, mud[i].estado,
                          mud[i].ant, mud[i].novo, mud[i].desde, mud[i].chave);
  if (leve) dados_gravar_leve(SEG_ARQ, buf);
  else      dados_gravar(SEG_ARQ, buf);
}

static void carregar(void) {
  char *t, *p;
  memset(&S, 0, sizeof S);
  memset(mud, 0, sizeof mud);
  nMud = 0;
  t = dados_ler(SEG_ARQ);
  if (!t) return;
  for (p = t; *p; ) {
    char *fim = strchr(p, '\n');
    if (fim) *fim = 0;
    if (p[0] == 'c' && p[1] == ' ') {
      sscanf(p + 2, "%d %d %d %d %d %u %u", &S.sessao, &S.rapidas, &S.aberta,
             &S.estavel, &S.perfilSeguro, &S.uptime, &S.avisos);
    } else if (p[0] == 'm' && p[1] == ' ' && nMud < SEG_MAX_MUD) {
      SegMud m;
      char chave[SEG_CHAVE];
      memset(&m, 0, sizeof m);
      if (sscanf(p + 2, "%d %ld %d %d %d %u %31s", &m.sessao, &m.quando,
                 &m.estado, &m.ant, &m.novo, &m.desde, chave) == 7 &&
          m.estado >= SEG_PROVISORIA && m.estado <= SEG_REVERTIDA) {
        snprintf(m.chave, sizeof m.chave, "%s", chave);
        mud[nMud++] = m;
      }
    }
    if (!fim) break;
    p = fim + 1;
  }
  free(t);
}

static void confirmarProvisorias(void) {
  int i;
  for (i = 0; i < nMud; i++)
    if (mud[i].estado == SEG_PROVISORIA) mud[i].estado = SEG_CONFIRMADA;
}

const SegDecisao *seguro_iniciar(int anteriorCaiu, long agora, SegAplicar aplicar) {
  int i, prov = 0, perfilAnterior;
  (void)agora;
  carregar();
  memset(&dec, 0, sizeof dec);
  dec.caiu = anteriorCaiu ? 1 : 0;
  dec.ultimoT = S.uptime;
  perfilAnterior = S.perfilSeguro;

  if (anteriorCaiu) {
    // QUEDA RAPIDA = a sessao estava aberta no diario e nunca chegou a 60 s.
    // Sem `aberta` (primeiro arranque desta versao, arquivo apagado) nao ha
    // prova de quanto ela durou, e na duvida nao conta.
    int rapida = S.aberta && !S.estavel;
    S.rapidas = rapida ? S.rapidas + 1 : 0;
    for (i = 0; i < nMud; i++) {
      if (mud[i].estado != SEG_PROVISORIA) continue;
      prov++;
      if (!aplicar || aplicar(mud[i].chave, mud[i].novo, mud[i].ant)) {
        if (dec.nRevertidas < SEG_MAX_MUD) dec.revertidas[dec.nRevertidas++] = mud[i];
        printf("[seguro] revertido %s %d -> %d (sessao anterior caiu t=%us)\n",
               mud[i].chave, mud[i].novo, mud[i].ant, dec.ultimoT);
      } else {
        printf("[seguro] %s ja nao vale %d (sessao anterior caiu t=%us): nada a desfazer\n",
               mud[i].chave, mud[i].novo, dec.ultimoT);
      }
      mud[i].estado = SEG_REVERTIDA;
    }
    dec.rapidas = S.rapidas;
    if (prov > 0) {
      // Havia um culpado apontavel e ele foi desfeito: a proxima sessao e a
      // prova. Zerar aqui evita acender o perfil seguro por cima da correcao.
      S.rapidas = 0;
    } else if (S.rapidas >= SEG_LACO_N) {
      // Laco de quedas sem culpado. Se a sessao anterior JA era no perfil
      // seguro e caiu depressa de novo, o perfil seguro nao bastou como
      // remedio temporario: grava-se de vez, e a pessoa e avisada.
      dec.modo = perfilAnterior ? SEG_PERSISTIR_SEGURO : SEG_PERFIL_SEGURO;
      if (dec.modo == SEG_PERSISTIR_SEGURO) S.rapidas = 0;
    }
  } else {
    // Saiu limpo (ou a TV escondeu o app): o que estava em prova passou.
    confirmarProvisorias();
    S.rapidas = 0;
    dec.rapidas = 0;
  }

  S.sessao++;
  dec.sessao = S.sessao;
  S.aberta = 1;
  S.estavel = 0;
  S.uptime = 0;
  S.perfilSeguro = dec.modo == SEG_PERFIL_SEGURO;
  iniciado = 1;
  ultimaBatida = 0;
  salvar(0);
  printf("[seguro] sessao %d: anterior %s (t=%us), %d mudanca(s) em prova, quedas rapidas %d -> %s\n",
         S.sessao, anteriorCaiu ? "caiu" : "saiu limpo", dec.ultimoT, prov, dec.rapidas,
         dec.modo == SEG_PERFIL_SEGURO ? "PERFIL SEGURO nesta sessao"
         : dec.modo == SEG_PERSISTIR_SEGURO ? "valores seguros gravados"
         : "normal");
  fflush(stdout);
  return &dec;
}

void seguro_mudou(const char *chave, int ant, int novo, long agora, unsigned uptimeS) {
  int i;
  if (!iniciado || !chave || !chave[0] || ant == novo) return;
  for (i = 0; i < nMud; i++)
    if (mud[i].estado == SEG_PROVISORIA && !strcmp(mud[i].chave, chave)) {
      // Varias mexidas no mesmo ajuste na mesma prova: o "anterior" que vale e
      // o PRIMEIRO, o que ainda nao tinha derrubado nada.
      if (novo == mud[i].ant) {
        memmove(&mud[i], &mud[i + 1], sizeof mud[0] * (size_t)(nMud - i - 1));
        nMud--;
      } else {
        mud[i].novo = novo; mud[i].quando = agora; mud[i].desde = uptimeS;
      }
      salvar(0);
      return;
    }
  if (nMud >= SEG_MAX_MUD) {
    // Cheio: sai a mais antiga que ja foi resolvida; se todas estao em prova,
    // a mais antiga mesmo (nao ha tantos ajustes arriscados quanto o teto).
    int velha = 0;
    for (i = 0; i < nMud; i++)
      if (mud[i].estado != SEG_PROVISORIA) { velha = i; break; }
    memmove(&mud[velha], &mud[velha + 1], sizeof mud[0] * (size_t)(nMud - velha - 1));
    nMud--;
  }
  memset(&mud[nMud], 0, sizeof mud[0]);
  snprintf(mud[nMud].chave, sizeof mud[nMud].chave, "%s", chave);
  mud[nMud].ant = ant; mud[nMud].novo = novo;
  mud[nMud].estado = SEG_PROVISORIA;
  mud[nMud].sessao = S.sessao; mud[nMud].quando = agora; mud[nMud].desde = uptimeS;
  nMud++;
  salvar(0);
  printf("[seguro] em prova %s %d -> %d (confirma em %d s ou na saida limpa)\n",
         chave, ant, novo, SEG_CONFIRMA_S);
  fflush(stdout);
}

void seguro_ajustou(const char *chave, int novo, int arriscado) {
  int i;
  if (!iniciado || !chave) return;
  for (i = 0; i < nMud; i++)
    if (mud[i].estado == SEG_PROVISORIA && !strcmp(mud[i].chave, chave)) {
      if (!arriscado || novo == mud[i].ant) {
        memmove(&mud[i], &mud[i + 1], sizeof mud[0] * (size_t)(nMud - i - 1));
        nMud--;
      } else mud[i].novo = novo;
      salvar(0);
      return;
    }
}

void seguro_batida(unsigned uptimeS) {
  int i, duravel = 0;
  if (!iniciado) return;
  S.uptime = uptimeS;
  if (!S.estavel && uptimeS >= SEG_ESTAVEL_S) {
    S.estavel = 1; S.rapidas = 0; duravel = 1;
  }
  for (i = 0; i < nMud; i++)
    if (mud[i].estado == SEG_PROVISORIA &&
        uptimeS >= mud[i].desde + SEG_CONFIRMA_S) {
      mud[i].estado = SEG_CONFIRMADA;
      duravel = 1;
      printf("[seguro] confirmado %s %d (aberto %u s depois da mudanca)\n",
             mud[i].chave, mud[i].novo, uptimeS - mud[i].desde);
      fflush(stdout);
    }
  if (duravel) { salvar(0); ultimaBatida = uptimeS; }
  else if (uptimeS >= ultimaBatida + 30) { salvar(1); ultimaBatida = uptimeS; }
}

void seguro_encerrar(void) {
  if (!iniciado) return;
  confirmarProvisorias();
  S.aberta = 0;
  S.rapidas = 0;
  salvar(0);
  iniciado = 0;
}

int seguro_perfil_ativo(void) { return iniciado && S.perfilSeguro; }

int seguro_aviso_visto(int bit) { return (S.avisos & (unsigned)bit) != 0; }
void seguro_aviso_marcar(int bit) {
  if (S.avisos & (unsigned)bit) return;
  S.avisos |= (unsigned)bit;
  if (iniciado) salvar(0);
}

int seguro_n_mud(void) { return nMud; }
const SegMud *seguro_mud(int i) { return i >= 0 && i < nMud ? &mud[i] : NULL; }
int seguro_n_provisorias(void) {
  int i, k = 0;
  for (i = 0; i < nMud; i++) if (mud[i].estado == SEG_PROVISORIA) k++;
  return k;
}
