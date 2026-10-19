#include "epcoment.h"
#include "extras.h"
#include "sinopse.h"
#include "descoberta.h"
#include "text.h"
#include "idioma.h"
#include "ajustes.h"
#include "js.h"
#include "anim.h"
#include "layout.h"
#include "ponteiro.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

static char show[64];
static int temp, episodio, foco, dentro, quantidade;
static float scroll;
// Byte ranges survive texture eviction. Rewrap only on content/font/width changes.
static struct {
  char texto[16384], data[80];
  unsigned short inicio[16384], tamanho[16384];
  int n, revelado;
  float comprimentos[16384];
  float largura, escala;
  TxtFamilia familia;
} cards[EX_COMENT_MAX];

void epcoment_abrir(const char *imdb, int t, int e) {
  snprintf(show, sizeof show, "%s", imdb);
  temp = t; episodio = e; foco = dentro = 0;
  quantidade = -2; scroll = 0;
  memset(cards, 0, sizeof cards);
  extras_pedir_comentarios_ep(show, temp, episodio);
}
int epcoment_quantidade(void) {
  if (!extras_comentarios_ep_do(show, temp, episodio) || extras_comentarios_ep_carregando()) return -1;
  return extras_n_comentarios_ep();
}
void epcoment_atualizar(float dt) {
  // A previous episode's worker may still be finishing when this menu opens.
  if (!extras_comentarios_ep_do(show, temp, episodio) && !extras_comentarios_ep_carregando())
    extras_pedir_comentarios_ep(show, temp, episodio);
  scroll = ajustes_animacoes_reduzidas() ? foco * 280.0f : anim_mola(scroll, foco * 280.0f, dt, NV_MOLA_TELA);
}
int epcoment_foco(void) { return dentro ? foco : -1; }
void epcoment_menu(void) { dentro = 0; }
int epcoment_entrar(void) { if (epcoment_quantidade() <= 0) return 0; dentro = 1; return 1; }
void comentario_abrir_inteiro(int indice) {
  if (indice < 0 || indice >= epcoment_quantidade()) return;
  char data[64];
  foco = indice;
  desc_data_extenso(extras_comentario_ep_data(indice), data, sizeof data);
  sinopse_comentario(extras_comentario_ep_usuario(indice), extras_comentario_ep_texto(indice), data,
                    extras_comentario_ep_nota(indice), extras_comentario_ep_curtidas(indice),
                    extras_comentario_ep_respostas(indice), 0, indice + 1, epcoment_quantidade());
}
int epcoment_evento(const SDL_Event *e) {
  SDL_Keycode k;
  if (!dentro || e->type != SDL_KEYDOWN) return 0;
  k = e->key.keysym.sym;
  if (k == SDLK_LEFT) { epcoment_menu(); return 1; }
  if (k == SDLK_UP || k == SDLK_DOWN) {
    foco += k == SDLK_DOWN ? 1 : -1;
    if (foco >= epcoment_quantidade()) foco = epcoment_quantidade() - 1;
    if (foco < 0) foco = 0;
    return 1;
  }
  if (k == SDLK_RETURN || k == SDLK_KP_ENTER || k == SDLK_SPACE) {
    if (!e->key.repeat) {
      if (extras_comentario_ep_spoiler(foco) && !cards[foco].revelado) cards[foco].revelado = 1;
      else comentario_abrir_inteiro(foco);
    }
    return 1;
  }
  return 0; // Back belongs to the entire context menu, including fallback.
}
static void texto(TxtEstilo estilo, const char *s, float x, float y, float a) {
  txt_desenhar_alpha(txt_linha(estilo, s, 243, 242, 239, 255), x, y, a);
}
static void preparar(int i, float w) {
  const char *s = extras_comentario_ep_texto(i), *p;
  if (cards[i].n && cards[i].largura == w && cards[i].escala == gfx_escala() &&
      cards[i].familia == txt_fonte_interface() && !strcmp(cards[i].texto, s)) return;
  snprintf(cards[i].texto, sizeof cards[i].texto, "%s", s);
  cards[i].largura = w; cards[i].escala = gfx_escala(); cards[i].familia = txt_fonte_interface();
  cards[i].n = 0; p = cards[i].texto;
  while (*p && cards[i].n < 16384) {
    char buf[16384]; size_t n = 0, quebra = 0;
    const char *inicio;
    while (*p == ' ' || *p == '\r') p++;
    if (!*p) break;
    inicio = p;
    while (*p && *p != '\n') {
      size_t k = 1;
      while (p[k] && ((unsigned char)p[k] & 0xc0) == 0x80) k++;
      if (n + k >= sizeof buf) break;
      memcpy(buf + n, p, k); buf[n + k] = 0;
      if (n && txt_largura(TXT_ILHA_TEXTO, buf) > w) break;
      if (*p == ' ') quebra = n;
      n += k; p += k;
    }
    if (*p && *p != '\n' && quebra) { n = quebra; p = inicio + quebra; }
    int j = cards[i].n++;
    cards[i].inicio[j] = (unsigned short)(inicio - cards[i].texto);
    cards[i].tamanho[j] = (unsigned short)n;
    memcpy(buf, inicio, n); buf[n] = 0;
    cards[i].comprimentos[j] = txt_largura(TXT_ILHA_TEXTO, buf);
    if (*p == '\n') p++;
    if (p == inicio && *p) p++;
  }
  long long ms = js_ms_iso(extras_comentario_ep_data(i));
  int dias = ms ? (int)(((long long)time(NULL) - ms / 1000) / 86400) : 0;
  if (dias > 0) snprintf(cards[i].data, sizeof cards[i].data, i18n("há %d dias"), dias);
  else snprintf(cards[i].data, sizeof cards[i].data, "%s", i18n("Agora"));
}
static void focar(int i, int b) { (void)b; if (i >= 0 && i < epcoment_quantidade()) { foco = i; dentro = 1; } }
static void ativar(int i, int b) {
  (void)b; focar(i, 0);
  if (extras_comentario_ep_spoiler(i) && !cards[i].revelado) cards[i].revelado = 1;
  else comentario_abrir_inteiro(i);
}
void epcoment_desenhar(GfxRect r, float a) {
  ExEpNotas notas = {0}; char buf[120];
  int n = epcoment_quantidade();
  float x = r.x + 28, top = r.y + 190, w = r.w - 56, max = 1;
  gfx_cor(r, 24 / r.h, .11f, .114f, .129f, a);
  gfx_anel(r, 24 / r.h, 1, 1, 1, 1, .12f * a);
  if (n >= 0) extras_notas_ep(&notas);
  if (notas.nota) snprintf(buf, sizeof buf, idioma_ponto_decimal(ajustes_idioma()) ? "%d.%d" : "%d,%d", notas.nota / 10, notas.nota % 10);
  else snprintf(buf, sizeof buf, "%s", "—");
  texto(TXT_G52B, buf, x, r.y + 42, a);
  snprintf(buf, sizeof buf, i18n("%d votos"), notas.votos);
  texto(TXT_G18R, "Trakt", x + 28, r.y + 110, .8f * a);
  texto(TXT_G18R, "·", x + 76, r.y + 110, .8f * a);
  texto(TXT_G18R, buf, x + 92, r.y + 110, .8f * a);
  gfx_cor((GfxRect){x, r.y + 112, 20, 20}, .5f, .93f, .08f, .13f, a);
  for (int k = 5; k <= 10; k++) if (notas.distribuicao[k] > max) max = notas.distribuicao[k];
  float bx = x + 235, bw = w - 235;
  for (int k = 10; k >= 5; k--) {
    float y = r.y + 32 + (10 - k) * 22;
    snprintf(buf, sizeof buf, "%d", k);
    texto(TXT_G18R, buf, bx - 30, y - 9, .6f * a);
    gfx_cor((GfxRect){bx, y, bw, 7}, .5f, 1, 1, 1, .12f * a);
    if (notas.distribuicao[k]) gfx_cor((GfxRect){bx, y, bw * notas.distribuicao[k] / max, 7}, .5f, 1, .30f, .43f, a);
  }
  gfx_cor((GfxRect){x, top - 16, w, 1}, 0, 1, 1, 1, .12f * a);
  if (n <= 0) {
    texto(TXT_ILHA_TEXTO, n < 0 ? i18n("Carregando comentários…") : i18n("Nenhum comentário ainda."), x + 20, top + 34, .75f * a);
    return;
  }
  if (quantidade != n) { quantidade = n; if (foco >= n) foco = 0; }
  gfx_recorte(x - 4, top - 4, w + 8, r.h - 210);
  for (int i = 0; i < n; i++) {
    float y = top + i * 280 - scroll;
    float h = 280;
    if (y + h < top || y > r.y + r.h) continue;
    preparar(i, w - 44);
    int ativo = dentro && foco == i;
    float ca = a * (foco == i ? 1 : .45f);
    GfxRect card = {x, y, w, h};
    if (ativo) {
      gfx_cor(card, 18 / h, .21f, .216f, .235f, a);
      gfx_anel(card, 18 / h, 3, .94f, .94f, .95f, a);
    }
    if (y >= top && y + h <= r.y + r.h - 20)
      ponteiro_alvo(card.x, card.y, card.w, card.h, focar, ativar, i, 0);
    const char *user = extras_comentario_ep_usuario(i);
    char inicial[8] = "?"; size_t k = 1;
    if (*user) { while (user[k] && ((unsigned char)user[k] & 0xc0) == 0x80 && k < 7) k++; memcpy(inicial, user, k); inicial[k] = 0; }
    gfx_cor((GfxRect){x + 24, y + 28, 44, 44}, .5f, i % 2 ? .15f : .35f, i % 2 ? .65f : .47f, i % 2 ? .43f : .98f, ca);
    texto(TXT_G21B, inicial, x + 37, y + 36, ca);
    txt_desenhar_alpha(txt_linha_corta(TXT_G21B, user, 243, 242, 239, 255, w - 220), x + 82, y + 24, ca);
    texto(TXT_G18R, cards[i].data, x + 82, y + 53, .6f * ca);
    int nota = extras_comentario_ep_nota(i);
    if (nota) {
      snprintf(buf, sizeof buf, "♥ %d/10", nota);
      txt_desenhar_alpha(txt_linha(TXT_G21B, buf, 255, 78, 108, 255), x + w - 105, y + 40, ca);
    }
    int hidden = extras_comentario_ep_spoiler(i) && !cards[i].revelado;
    for (int j = 0; j < cards[i].n && j < 4; j++) {
      char l[16384]; int len = cards[i].tamanho[j];
      memcpy(l, cards[i].texto + cards[i].inicio[j], len); l[len] = 0;
      if (j == 3 && j + 1 < cards[i].n) snprintf(l + len, sizeof l - len, "…");
      float ty = y + 94 + j * 31;
      // Conceal every glyph with soft overlapping strokes; no spoiler texture is exposed.
      if (hidden) {
        float width = cards[i].comprimentos[j];
        for (int b = 0; b < 3; b++) gfx_cor((GfxRect){x + 24 - b, ty + 7 - b * 2, width + b * 2, 9 + b * 4}, .5f, .75f, .76f, .79f, .08f * ca);
      } else texto(TXT_ILHA_TEXTO, l, x + 24, ty, .87f * ca);
    }
    snprintf(buf, sizeof buf, "♥ %d", extras_comentario_ep_curtidas(i));
    texto(TXT_G18R, buf, x + 24, y + h - 44, .58f * ca);
    gfx_icone((GfxRect){x + 116, y + h - 42, 19, 19}, "aj_message-square", 1, 1, 1, .58f * ca);
    snprintf(buf, sizeof buf, "%d", extras_comentario_ep_respostas(i));
    texto(TXT_G18R, buf, x + 142, y + h - 44, .58f * ca);
  }
  gfx_sem_recorte();
  float track = r.h - 218, thumb = track / n;
  gfx_cor((GfxRect){r.x + r.w - 16, top, 4, track}, 2 / track, 1, 1, 1, .14f * a);
  float pos = n > 1 ? foco / (float)(n - 1) : 0;
  gfx_cor((GfxRect){r.x + r.w - 16, top + pos * (track - thumb), 4, thumb}, 2 / thumb, 1, 1, 1, .9f * a);
}
