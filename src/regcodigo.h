// CODIGO DO REGISTRO ENVIADO: seis caracteres que a pessoa informa a quem faz
// o app (Ajustes > Sobre e ajuda > Enviar registro). Vem do registro_id que o
// servidor devolve no recibo (o id da linha na tabela `registro`), por uma
// bijecao afim mod 2^30 em base32 de Crockford — a MESMA de
// servidor/recomendacoes/src/codigo.js. O suporte volta ao id com
// servidor/recomendacoes/codigo-registro.mjs. Sem I, L, O nem U: nada que se
// confunda ditando pelo telefone.
#ifndef NV_REGCODIGO_H
#define NV_REGCODIGO_H

// `id` em texto decimal (como extrairRegistroId entrega). Escreve 6 caracteres
// e o zero em `dst` (>= 7 bytes) e devolve 1; 0 se o id nao for um numero de
// 1 a 2^30-1 (dst fica vazio).
int regcodigo_de_id(const char *id, char *dst);

#endif
