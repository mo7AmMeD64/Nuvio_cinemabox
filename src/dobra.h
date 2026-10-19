#ifndef NV_DOBRA_H
#define NV_DOBRA_H

// LETRA ESTILIZADA -> LETRA COMUM (#144). Os formatadores de nome de fonte
// (AIOStreams e similares) escrevem "RᴇLᴇAꜱᴇ", "𝗕𝗹𝘂𝗥𝗮𝘆", "Ⓓⓤⓑ" e "ＨＤＲ" com
// caracteres que so PARECEM letras: versalete (U+1D00...), alfanumericos
// matematicos (U+1D400...), letras circuladas e largura cheia. A Inter nao tem
// nenhum deles; na TV cada um saia como o retangulo do .notdef ou puxava a
// linha inteira para a fonte de reserva.
//
// Devolve o ASCII equivalente, ou 0 quando `cp` nao e letra/digito estilizado.
// Versalete vira MAIUSCULA: e como ele se le. Pura, testada em tests/dobra.sh.
char nv_dobra_estilizada(unsigned long cp);

#endif
