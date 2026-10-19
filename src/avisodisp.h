// AVISOS DISPENSADOS — o "nao quero mais ver isto" de cada aviso.
//
// Pedido do dono (06/10/2026): "muito alerta sem dispensar; o lembrete de
// serie so sai se eu entrar na serie". Ate aqui o unico jeito de tirar um
// aviso era abrir o alvo dele (a pagina do titulo, Salvos, a atualizacao) ou
// abrir a aba Avisos inteira, que marcava TUDO como lido — e lido nao e
// dispensado: a linha continuava na lista e o aviso do dono voltava a cada
// arranque. Este modulo guarda o que a pessoa DISPENSOU.
//
// TRES ESTADOS, e eles nao se misturam:
//   - lido (avisos-vistos.txt, avisos.c): o ponto de "novo" apaga; a linha fica;
//   - adiado ("Depois", so nesta sessao, so na RAM): o cartao sai da pilula
//     ate o app fechar; na proxima abertura volta, se ainda nao foi lido;
//   - dispensado (este arquivo): a linha sai da lista, o cartao da pilula, e
//     a MESMA chave nao volta nem depois de reiniciar.
//
// A CHAVE E O EVENTO, nao o tipo: "agenda:<imdb>:<data do episodio>",
// "update:<versao>", o id do aviso do dono, "crash:<quando>",
// "enquete:<id>". Dispensar a estreia de T2E5 nao cala T2E6 (outra data,
// outra chave), nem a versao 1.8.1 cala a 1.8.2. O lembrete da serie continua
// ligado: dispensar e sobre o aviso, nao sobre a intencao (para isso ha
// "Remover lembrete", agenda_alternar_lembrete).
//
// POR PERFIL, EM DISCO: avisos-dispensados.txt na pasta de dados, uma linha
// "<perfil> <chave>" (o formato do enquete.txt). Teto de AVD_MAX linhas; a mais
// velha sai primeiro — uma chave de evento antigo nao volta a ser emitida, entao
// perder a mais velha nao traz nada de volta na pratica.
//
// SESSAO: avisodisp_sessao_* e o conjunto em RAM do que ja foi dito/adiado
// nesta sessao (tambem por perfil). E o limite de ruido: um aviso que ja passou
// pela ilha nesta sessao nao volta a passar, a menos que seja um evento novo
// (outra chave).
//
// Thread-safe (o fio do canal de avisos.c consulta). Sem SDL, testavel sozinho
// (tests/avisodisp.c).
#ifndef NV_AVISODISP_H
#define NV_AVISODISP_H

#define AVD_ARQ   "avisos-dispensados.txt"
#define AVD_MAX   200
#define AVD_CHAVE 80

// 1 = a chave foi dispensada no perfil ativo.
int  avisodisp_tem(const char *chave);
// Dispensa no perfil ativo e grava. Repetir nao duplica.
void avisodisp_por(const char *chave);
// Nesta sessao (RAM): ja dito/adiado no perfil ativo.
int  avisodisp_sessao_tem(const char *chave);
void avisodisp_sessao_por(const char *chave);
// Para os testes: esquece a RAM (o arquivo e relido na proxima consulta).
void avisodisp_esquecer(void);

#endif
