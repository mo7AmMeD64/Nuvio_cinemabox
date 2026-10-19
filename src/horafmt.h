// Hora DE TELA no formato escolhido em Ajustes (Aparencia > Formato do
// relogio): "18:30" ou "6:30 PM". Log, marca de arquivo e cabecalho de registro
// continuam em 24 h de proposito: quem le e gente comparando com o servidor.
#ifndef NV_HORAFMT_H
#define NV_HORAFMT_H
#include <stddef.h>
#include <time.h>

// So cabecalho (static inline): dezenas de testes compilam ilha.c, guia.c,
// menu.c... sozinhos, e um relogio.c a mais quebraria o link de todos.
// hora_tela_partes: digitos ("6:30") e sufixo ("PM", ou "" em 24 h)
// separados, porque o numeral grande da tela de descanso usa uma fonte que so
// tem digitos e ':' (text.c).
#include <stdio.h>
int ajustes_relogio_12h(void);

static inline void hora_tela_partes(char *dig, size_t nd, char *suf, size_t ns, const struct tm *t) {
  if (suf && ns) suf[0] = 0;
  if (!dig || !nd) return;
  if (!t) { snprintf(dig, nd, "--:--"); return; }
  if (!ajustes_relogio_12h()) { snprintf(dig, nd, "%02d:%02d", t->tm_hour, t->tm_min); return; }
  snprintf(dig, nd, "%d:%02d", t->tm_hour % 12 ? t->tm_hour % 12 : 12, t->tm_min);
  if (suf && ns) snprintf(suf, ns, "%s", t->tm_hour < 12 ? "AM" : "PM");
}

static inline void hora_tela(char *dst, size_t n, const struct tm *t) {
  char suf[4];
  size_t k;
  hora_tela_partes(dst, n, suf, sizeof suf, t);
  if (!dst || !n || !suf[0]) return;
  k = 0; while (dst[k]) k++;
  snprintf(dst + k, n - k, " %s", suf);
}

static inline void hora_tela_t(char *dst, size_t n, time_t t) {
  struct tm lt;
  hora_tela(dst, n, localtime_r(&t, &lt) ? &lt : NULL);
}

#endif
