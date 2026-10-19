// ENQUETE NA ILHA DO RELOGIO (N3, 2.0) — o lado cliente.
//
// Mockup aprovado: mockups-20/04-enquete-ilha.html. Construida so com o que a
// ilha ja faz: aviso curto com a tecla azul que abre sozinho (como a pergunta
// "Onde o + salva?", ilhasalvar.c), o modal que cresce da pilula, ilha_acao com
// Desfazer e a bolinha de acento no relogio (ilha_ponto_enquete).
//
// FLUXO. O servico (servidor/recomendacoes/src/enquete.js) entrega UMA enquete
// ativa por conta/perfil. O convite aparece UMA vez por enquete e perfil
// (enquete.txt guarda que ja foi dito); "Agora nao" — ou o Voltar — deixa uma
// bolinha de acento no relogio ate a pessoa responder, e a AZUL nele reabre o
// convite, agora com "Dispensar" (06/10): dispensada, a bolinha apaga e aquela
// enquete nao volta para o perfil (avisodisp.h).
// Votou: o modal passa a mostrar o resultado (texto, porcentagem e trilho).
// "Nao receber mais enquetes" vai para a conta e para Ajustes (espelho local), com
// Desfazer pela ilha.
//
// NUNCA BLOQUEIA A ABERTURA: tudo que fala com o servidor roda num fio proprio,
// o primeiro pedido espera o app assentar, e trocar de perfil invalida o que
// estiver em voo (geracao). Sem NUVIO_REC_URL no pacote, nada acontece.
// FIO PRINCIPAL nas funcoes abaixo.
#ifndef NV_ENQUETE_H
#define NV_ENQUETE_H
#include <SDL2/SDL.h>

// Uma vez por quadro (ilhasinais_passo).
void enquete_passo(Uint32 agora);
// Botao de um modal de chave "enquete:..." (ilhasinais.c entrega).
void enquete_acao(const char *chave, int botao);
// app.c: o perfil trocou. Cancela o que esta em voo e busca de novo.
void enquete_perfil_trocado(void);
// Ajustes > Receber enquetes mudou (ajustes.c): leva a escolha para a conta.
// `sair` 1 = nao receber mais.
void enquete_definir_optout(int sair);
// 1 enquanto ha enquete aberta esperando resposta (a bolinha do relogio).
int  enquete_aberta(void);

#endif
