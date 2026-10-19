#ifndef NV_RELOGIOFIM_H
#define NV_RELOGIOFIM_H
// "TERMINA AS HH:MM" DO PLAYER E DA TELA DE PAUSA (issue #213).
//
// O relato era "Ends at 1", sempre "1", com o relogio de cima certo. O
// strftime estava certo; o BUFFER nao: o player montava a frase em fim[32], e
// a traducao russa "Заканчивается в %s" ja ocupa 30 bytes em UTF-8 (cirilico e
// 2 bytes por letra). Sobravam 1 byte de hora e o NUL: "Заканчивается в 1" —
// e o "1" e o primeiro digito de qualquer hora entre 10:00 e 19:59. O grego
// ("Τελειώνει στις %s") cortava do mesmo jeito. A pausa usava fim[40] e
// passava raspando. tests/relogiofim.c varre os 30 idiomas.
//
// A hora fica em 24 h ("%H:%M"), como o relogio de cima: o defeito era o
// corte, nao o formato. RELOGIO_FIM_MAX cabe tambem uma hora de 12 h
// ("11:59 PM") na traducao mais longa, se algum dia o formato mudar.
#include "idioma.h"
#include <stdio.h>
#include <time.h>

#define RELOGIO_FIM_MAX 128

// `fim` com RELOGIO_FIM_MAX bytes. `falta` em segundos (negativo vale 0).
static inline void relogio_fim(char *fim, size_t tam, time_t agora, double falta) {
  time_t t2 = agora + (time_t)(falta > 0.0 ? falta : 0.0);
  struct tm lf;
  char h2[16];
  localtime_r(&t2, &lf);
  strftime(h2, sizeof h2, "%H:%M", &lf);
  // i18n NO FORMATO: frase montada nao casa com chave (issue #12). O "a" com
  // crase vai escrito como \xc3\xa0, com o literal partido em dois.
  snprintf(fim, tam, i18n("Termina \xc3\xa0" "s %s"), h2);
}

// A MESMA FRASE NA PILULA DA ILHA DO PLAYER (plrilha.c): "termina as 22:41",
// em minuscula depois da hora, como no mockup do Glass UI. Mesmo buffer.
static inline void relogio_fim_ilha(char *fim, size_t tam, time_t agora, double falta) {
  time_t t2 = agora + (time_t)(falta > 0.0 ? falta : 0.0);
  struct tm lf;
  char h2[16];
  localtime_r(&t2, &lf);
  strftime(h2, sizeof h2, "%H:%M", &lf);
  snprintf(fim, tam, i18n("termina \xc3\xa0" "s %s"), h2);
}

#endif
