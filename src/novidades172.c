// Cartao de NOVIDADES DA 1.7.2: a cara nova, entrar com e-mail e as fontes que
// chegam conforme cada addon responde.
//
// O MOLDE E O DA 1.4.8 E DA 1.7 (novidades170.c): a esquerda uma PREVIA VIVA,
// a direita o titulo e uma lista CURTA, no rodape duas pilulas. Tres cenas
// trocam sozinhas; cima/baixo levam o foco entre a previa e os botoes.
//
// A PALETA E A DO LOGO NOVO, e nao a cor de destaque do app: o cartao
// apresenta a marca. Ameixa no fundo, creme no texto, laranja no primario,
// amarelo e vermelho nos detalhes, e as listras do login atras de tudo. Os
// botoes sao desenhados aqui (botoes.h pinta com a cor viva da pessoa).
//
// As cenas usam a arte de verdade da abertura e do login (art/marcas/
// abertura.jpg e login-fundo.jpg) e o mesmo QR de login.c; a escala e a da
// previa, nao a de tela cheia, pelo mesmo motivo da 1.7: chamar login.c daqui
// mexeria no estado da tela de entrada.
//
// CUSTO: texto so por txt_linha/txt_bloco (textura por estilo, cor e texto;
// cor nunca muda por quadro, so o alfa). As duas artes sao pedidas numa
// largura so. O QR e uma textura gerada uma vez.
//
// A MARCA E "novidades-172-ui.txt" (N172_ARQ).
#include "novidades172.h"
#include "ajustes.h"
#include "anim.h"
#include "dados.h"
#include "gfx.h"
#include "idioma.h"
#include "layout.h"
#include "ponteiro.h"
#include "qr.h"
#include "tex_cache.h"
#include "text.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N172_ARQ          "novidades-172-ui.txt"
#define N172_W            1720.0f
#define N172_H            1000.0f
#define N172_X            ((NV_TELA_W - N172_W) * 0.5f)
#define N172_Y            ((NV_TELA_H - N172_H) * 0.5f)
#define N172_PAD            52.0f
#define N172_RAIO           32.0f
#define N172_BT_H           72.0f
#define N172_BT_GAP         16.0f
#define N172_PV_W          760.0f
#define N172_PV_H          (N172_H - 2.0f * N172_PAD - N172_BT_H - 28.0f)
#define N172_PV_RAIO        26.0f
#define N172_COL_GAP        64.0f
#define N172_TXT_X        (N172_X + N172_PAD + N172_PV_W + N172_COL_GAP)
#define N172_TXT_W        (N172_X + N172_W - N172_PAD - N172_TXT_X)
#define N172_ICONE          40.0f
#define N172_ABRIR_MS      280.0f
#define N172_FECHAR_MS     160.0f
#define N172_TRANSICAO_S     0.55f
#define N172_ARTE_W       1280.0f

// A paleta do logo (medida no logo.png do lancamento).
#define AMEIXA_R 0.106f
#define AMEIXA_G 0.039f
#define AMEIXA_B 0.098f
#define CREME_R  0.996f
#define CREME_G  0.902f
#define CREME_B  0.769f
#define LARANJA_R 0.949f
#define LARANJA_G 0.486f
#define LARANJA_B 0.118f
#define AMARELO_R 0.980f
#define AMARELO_G 0.659f
#define AMARELO_B 0.157f
#define VERMELHO_R 0.839f
#define VERMELHO_G 0.204f
#define VERMELHO_B 0.165f
// Os mesmos, em bytes, para o texto (txt_linha recebe int).
#define CREME_I    254, 230, 196
#define CREME2_I   214, 190, 170   // creme apagado: a linha de descricao
#define AMEIXA_I    34,  12,  30
#define LARANJA_I  246, 140,  52

enum { B_DEPOIS = 0, B_OK = 1, B_N };
enum { ID_NADA = 0, ID_VISUAL, ID_EMAIL, ID_FONTES, ID_SEEKR, ID_AJUSTES, ID_RETOMAR, ID_CONSERTOS };

static int   aberto, decidido, foco = B_OK, naPrevia;
static int   cena, cenaAntiga;
static float entrada, transicao = 1.0f, relogioCena, tempoAntiga;
static char  dirArte[512] = "deploy/app/art";
static char  arqAbertura[600], arqListras[600];
static GLuint texQr;

// ------------------------------------------------------------------ apoio
static float rr(float px, GfxRect r) { float m = r.w < r.h ? r.w : r.h; return m > 0.0f ? px / m : 0.0f; }
static float passo(float t, float ini, float dur) {
  return anim_suave(anim_clamp((t - ini) / dur, 0.0f, 1.0f));
}

static void montarCaminhos(void) {
  snprintf(arqAbertura, sizeof arqAbertura, "%s/marcas/abertura.jpg", dirArte);
  snprintf(arqListras, sizeof arqListras, "%s/marcas/login-fundo.jpg", dirArte);
}
static void pedirArtes(void) {
  tex_obter_larg(arqAbertura, N172_ARTE_W);
  tex_obter_larg(arqListras, N172_ARTE_W);
}

// Arte em "cover" num retangulo arredondado; ameixa enquanto nao chega.
static void cobrir(const char *arq, GfxRect r, float raioPx, float a) {
  GLuint t;
  if (a <= 0.003f) return;
  t = tex_obter_larg(arq, N172_ARTE_W);
  if (!t) { gfx_cor(r, rr(raioPx, r), AMEIXA_R, AMEIXA_G, AMEIXA_B, a); return; }
  gfx_tex_aspect_atual = tex_aspecto(arq);
  if (gfx_tex_aspect_atual <= 0.0f) gfx_tex_aspect_atual = 16.0f / 9.0f;
  gfx_card_forcar_cover_atual = 1.0f;
  gfx_rect(r, t, GFX_CARD, 0, 0, 0, rr(raioPx, r), 0, 0, 0, a);
  gfx_card_forcar_cover_atual = 0.0f;
  gfx_tex_aspect_atual = 0.0f;
}

// O QR da cena de entrada: o mesmo simbolo da tela de login, uma textura so.
static void gerarQr(void) {
  Qr q;
  int lado, x, y, mg = 3;
  unsigned char *px;
  if (texQr || !qr_gerar(&q, "https://nuvio.tv/tv-login")) return;
  lado = q.lado + 2 * mg;
  px = (unsigned char *)malloc((size_t)lado * lado * 3);
  if (!px) return;
  memset(px, 255, (size_t)lado * lado * 3);
  for (y = 0; y < q.lado; y++)
    for (x = 0; x < q.lado; x++)
      if (qr_modulo(&q, x, y)) {
        size_t i = ((size_t)(y + mg) * lado + (x + mg)) * 3;
        px[i] = px[i + 1] = px[i + 2] = 0;
      }
  glGenTextures(1, &texQr);
  glBindTexture(GL_TEXTURE_2D, texQr);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, lado, lado, 0, GL_RGB, GL_UNSIGNED_BYTE, px);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  gfx_tex_esquecer(0);
  free(px);
}

// A PILULA NA COR DO LOGO. Primario: laranja com texto ameixa; com foco ganha
// o anel creme e clareia. Secundario: vidro creme; com foco vira creme cheio.
static float pilulaLargura(const char *rot, const char *icone) {
  return (float)txt_largura(TXT_BODY, rot) + 2.0f * 34.0f + (icone ? 38.0f : 0.0f);
}
static void pilula(GfxRect r, const char *rot, const char *icone, float foco, int primario, float a) {
  TxtLinha claro = txt_linha(TXT_BODY, rot, CREME_I, 255);
  TxtLinha escuro = txt_linha(TXT_BODY, rot, AMEIXA_I, 255);
  int textoEscuro = primario || foco > 0.5f;
  TxtLinha l = textoEscuro ? escuro : claro;
  float raio = 0.5f, x;
  if (primario) {
    float k = 0.86f + 0.14f * foco;
    gfx_cor(r, raio, LARANJA_R * k + (1.0f - k) * 0.3f, LARANJA_G * k, LARANJA_B * k, a);
  } else {
    gfx_cor(r, raio, CREME_R, CREME_G, CREME_B, (0.10f + 0.82f * foco) * a);
  }
  if (foco > 0.0f) {
    GfxRect anel = { r.x - 5.0f, r.y - 5.0f, r.w + 10.0f, r.h + 10.0f };
    gfx_rect(anel, 0, GFX_ANEL, 0, 3.0f / anel.h, 0, 0.5f, CREME_R, CREME_G, CREME_B, foco * a);
  }
  x = r.x + (r.w - (float)l.w - (icone ? 38.0f : 0.0f)) * 0.5f;
  if (icone) {
    float c = textoEscuro ? 0.13f : CREME_R;
    gfx_icone((GfxRect){ x, r.y + (r.h - 28.0f) * 0.5f, 28.0f, 28.0f }, icone,
              c, textoEscuro ? 0.05f : CREME_G, textoEscuro ? 0.12f : CREME_B, a);
    x += 38.0f;
  }
  txt_desenhar_alpha(l, x, r.y + (r.h - (float)l.h) * 0.5f, a);
}

// As tres listras do logo (laranja, amarelo, vermelho), em pilulas finas.
static void listras(float x, float y, float w, float a) {
  float h = 6.0f, gap = 4.0f;
  gfx_cor((GfxRect){ x, y, w, h }, 0.5f, LARANJA_R, LARANJA_G, LARANJA_B, a);
  gfx_cor((GfxRect){ x, y + h + gap, w * 0.82f, h }, 0.5f, AMARELO_R, AMARELO_G, AMARELO_B, a);
  gfx_cor((GfxRect){ x, y + 2.0f * (h + gap), w * 0.64f, h }, 0.5f, VERMELHO_R, VERMELHO_G, VERMELHO_B, a);
}

// ------------------------------------------------------------------- cenas
// Cada cena desenha dentro da previa (x, y) .. (x + PV_W, y + PV_H), com o
// relogio dela em `t` segundos.

// A CARA NOVA: a abertura, o logo se dissolvendo e o login sobre as listras.
#define CA_DUR 7.0f
static void miniLogin(float x, float y, float a) {
  float cx = x + N172_PV_W * 0.5f;
  TxtLinha t = txt_linha(TXT_HEADLINE, i18n("Entrar na sua conta"), 255, 255, 255, 255);
  txt_desenhar_alpha(t, cx - (float)t.w * 0.5f, y + 210.0f, a);
  gerarQr();
  { float q = 230.0f;
    GfxRect m = { cx - q * 0.5f - 14.0f, y + 282.0f, q + 28.0f, q + 28.0f };
    gfx_cor(m, rr(18.0f, m), 1, 1, 1, a);
    if (texQr) {
      gfx_tex_aspect_atual = 0.0f;
      gfx_rect((GfxRect){ m.x + 14.0f, m.y + 14.0f, q, q }, texQr, GFX_SNAP, 0, 0, 0, 0.0f, 0, 0, 0, a);
    } }
  { const char *rot = i18n("Entrar com e-mail e senha");
    float w = (float)txt_largura(TXT_CAPTION, rot) + 56.0f;
    GfxRect p = { cx - w * 0.5f, y + 600.0f, w, 52.0f };
    TxtLinha l = txt_linha(TXT_CAPTION, rot, AMEIXA_I, 255);
    gfx_cor(p, 0.5f, CREME_R, CREME_G, CREME_B, 0.94f * a);
    txt_desenhar_alpha(l, p.x + 28.0f, p.y + (p.h - (float)l.h) * 0.5f, a); }
}
static void cenaVisual(float x, float y, float t, float a) {
  GfxRect pv = { x, y, N172_PV_W, N172_PV_H };
  float some = passo(t, 1.8f, 1.1f), entra = passo(t, 2.6f, 0.7f);
  // A abertura em cover numa previa mais estreita que 16:9: o logo fica no meio.
  cobrir(arqListras, pv, N172_PV_RAIO, a);
  cobrir(arqAbertura, pv, N172_PV_RAIO, a * (1.0f - some));
  if (entra > 0.0f) miniLogin(x, y + (1.0f - entra) * 14.0f, a * entra);
}

// ENTRAR COM E-MAIL: o e-mail sendo digitado, a senha, e o Entrar aceso.
#define EM_DUR 7.0f
static const char *const EM_EMAIL = "voce@gmail.com";
static void campo(float x, float y, float w, const char *rotulo, const char *valor,
                  int cursor, float a) {
  TxtLinha r = txt_linha(TXT_CAPTION2, rotulo, CREME2_I, 255);
  GfxRect c = { x, y + 30.0f, w, 64.0f };
  txt_desenhar_alpha(r, x + 4.0f, y, a);
  gfx_cor(c, rr(16.0f, c), AMEIXA_R, AMEIXA_G, AMEIXA_B, 0.78f * a);
  gfx_rect(c, 0, GFX_ANEL, 0, 1.5f / c.h, 0, rr(16.0f, c), CREME_R, CREME_G, CREME_B,
           (cursor ? 0.70f : 0.18f) * a);
  if (valor && valor[0]) {
    TxtLinha v = txt_linha(TXT_BODY, valor, CREME_I, 255);
    txt_desenhar_alpha(v, c.x + 22.0f, c.y + (c.h - (float)v.h) * 0.5f, a);
    if (cursor)
      gfx_cor((GfxRect){ c.x + 24.0f + (float)v.w, c.y + 18.0f, 3.0f, c.h - 36.0f }, 0.5f,
              LARANJA_R, LARANJA_G, LARANJA_B, a);
  } else if (cursor) {
    gfx_cor((GfxRect){ c.x + 22.0f, c.y + 18.0f, 3.0f, c.h - 36.0f }, 0.5f,
            LARANJA_R, LARANJA_G, LARANJA_B, a);
  }
}
static void cenaEmail(float x, float y, float t, float a) {
  GfxRect pv = { x, y, N172_PV_W, N172_PV_H };
  float fx = x + 90.0f, fw = N172_PV_W - 180.0f;
  int nEmail = (int)((t - 0.7f) * 8.0f), nSenha = (int)((t - 3.0f) * 6.0f), i;
  char email[32], senha[16];
  int piscaE = t < 2.9f && fmodf(t, 0.9f) < 0.55f;
  int piscaS = t >= 2.9f && t < 4.6f && fmodf(t, 0.9f) < 0.55f;
  float entrar = passo(t, 4.7f, 0.4f);
  cobrir(arqListras, pv, N172_PV_RAIO, a);
  gfx_cor(pv, rr(N172_PV_RAIO, pv), AMEIXA_R, AMEIXA_G, AMEIXA_B, 0.42f * a);
  if (nEmail < 0) nEmail = 0;
  if (nEmail > (int)strlen(EM_EMAIL)) nEmail = (int)strlen(EM_EMAIL);
  snprintf(email, sizeof email, "%.*s", nEmail, EM_EMAIL);
  if (nSenha < 0) nSenha = 0;
  if (nSenha > 8) nSenha = 8;
  for (i = 0; i < nSenha; i++) senha[i] = '*';
  senha[nSenha] = 0;
  { TxtLinha tt = txt_linha(TXT_HEADLINE, i18n("Entrar com e-mail e senha"), 255, 255, 255, 255);
    txt_desenhar_alpha(tt, x + (N172_PV_W - (float)tt.w) * 0.5f, y + 180.0f, a); }
  campo(fx, y + 280.0f, fw, i18n("E-mail"), email, piscaE || (t < 2.9f && nEmail < (int)strlen(EM_EMAIL)), a);
  campo(fx, y + 410.0f, fw, i18n("Senha"), senha, piscaS, a);
  { const char *rot = i18n("Entrar");
    float w = pilulaLargura(rot, NULL);
    GfxRect b = { x + (N172_PV_W - w) * 0.5f, y + 570.0f, w, 64.0f };
    pilula(b, rot, NULL, entrar, 1, a); }
}

// FONTES NA HORA: as linhas entram uma a uma, conforme cada addon responde, e
// a primeira e escolhida sem esperar a ultima.
#define FO_DUR 7.5f
typedef struct { const char *addon, *qual; float chega; } FonteLin;
static const FonteLin FONTES[] = {
  { "AIOStreams",  "4K · HDR10 · 18 GB",     0.6f },
  { "Torrentio",   "4K · Dolby Vision",      1.3f },
  { "Comet",       "1080p · 6.2 GB",         2.0f },
  { "MediaFusion", "1080p · 4.1 GB",         3.1f },
  { "Jackettio",   "720p · 1.8 GB",          4.6f },
};
#define FO_N ((int)(sizeof FONTES / sizeof *FONTES))
static void cenaFontes(float x, float y, float t, float a) {
  GfxRect pv = { x, y, N172_PV_W, N172_PV_H };
  float lx = x + 56.0f, lw = N172_PV_W - 112.0f, ly = y + 200.0f, lh = 86.0f;
  float escolhe = passo(t, 2.6f, 0.45f);
  int i;
  gfx_cor(pv, rr(N172_PV_RAIO, pv), AMEIXA_R, AMEIXA_G, AMEIXA_B, a);
  cobrir(arqListras, pv, N172_PV_RAIO, 0.32f * a);
  { TxtLinha tt = txt_linha(TXT_HEADLINE, i18n("Fontes"), 255, 255, 255, 255);
    txt_desenhar_alpha(tt, lx, y + 120.0f, a); }
  for (i = 0; i < FO_N; i++) {
    float e = passo(t, FONTES[i].chega, 0.35f);
    GfxRect r = { lx + (1.0f - e) * 30.0f, ly + (float)i * (lh + 12.0f), lw, lh };
    float vivo = i == 0 ? escolhe : 0.0f;
    if (e <= 0.0f) continue;
    gfx_cor(r, rr(18.0f, r), CREME_R, CREME_G, CREME_B, 0.07f * a * e);
    if (vivo > 0.0f) {
      gfx_cor(r, rr(18.0f, r), LARANJA_R, LARANJA_G, LARANJA_B, 0.92f * vivo * a * e);
      gfx_rect((GfxRect){ r.x - 4.0f, r.y - 4.0f, r.w + 8.0f, r.h + 8.0f }, 0, GFX_ANEL, 0,
               3.0f / (r.h + 8.0f), 0, rr(22.0f, r), CREME_R, CREME_G, CREME_B, vivo * a * e);
    }
    { TxtLinha n = txt_linha(TXT_BODY, FONTES[i].addon, CREME_I, 255);
      TxtLinha nE = txt_linha(TXT_BODY, FONTES[i].addon, AMEIXA_I, 255);
      TxtLinha q = txt_linha(TXT_CAPTION, FONTES[i].qual, CREME2_I, 255);
      TxtLinha qE = txt_linha(TXT_CAPTION, FONTES[i].qual, AMEIXA_I, 255);
      txt_desenhar_alpha(n, r.x + 26.0f, r.y + 14.0f, a * e * (1.0f - vivo));
      txt_desenhar_alpha(q, r.x + 26.0f, r.y + 48.0f, a * e * (1.0f - vivo));
      txt_desenhar_alpha(nE, r.x + 26.0f, r.y + 14.0f, a * e * vivo);
      txt_desenhar_alpha(qE, r.x + 26.0f, r.y + 48.0f, a * e * vivo); }
    if (vivo > 0.0f) {
      const char *rot = i18n("Reproduzir");
      TxtLinha p = txt_linha(TXT_CAPTION, rot, AMEIXA_I, 255);
      float pw = (float)p.w + 72.0f;
      GfxRect b = { r.x + r.w - pw - 18.0f, r.y + (r.h - 50.0f) * 0.5f, pw, 50.0f };
      gfx_cor(b, 0.5f, CREME_R, CREME_G, CREME_B, vivo * a * e);
      gfx_icone((GfxRect){ b.x + 18.0f, b.y + 13.0f, 24.0f, 24.0f }, "play", 0.13f, 0.05f, 0.12f, vivo * a * e);
      txt_desenhar_alpha(p, b.x + 50.0f, b.y + (b.h - (float)p.h) * 0.5f, vivo * a * e);
    }
  }
}

// ================================================================ as tabelas
typedef struct {
  const char *nome;
  void (*desenhar)(float x, float y, float t, float a);
  float duracao, estatico;   // s; o quadro das animacoes reduzidas
  int id;
} Cena;

static const Cena CENAS[] = {
  { "Cara nova",                 cenaVisual, CA_DUR, 4.0f, ID_VISUAL },
  { "Entrar com e-mail e senha", cenaEmail,  EM_DUR, 5.2f, ID_EMAIL },
  { "Fontes na hora",            cenaFontes, FO_DUR, 5.0f, ID_FONTES },
};
#define N172_NC ((int)(sizeof CENAS / sizeof *CENAS))

typedef struct { int id; const char *icone, *nome, *linha; } Item;

static const Item ITENS[] = {
  { ID_VISUAL,    "aj_palette",    "Cara nova",
    "Abertura, ícone e telas de entrada com o visual do Nuvio Legacy." },
  { ID_EMAIL,     "aj_keyboard",   "Entrar com e-mail e senha",
    "Sem celular por perto: e-mail e senha direto na TV, embaixo do QR." },
  { ID_FONTES,    "fontes",        "Fontes na hora",
    "A lista enche conforme cada addon responde e a escolha não espera o mais lento." },
  { ID_SEEKR,     "aj_images",     "Seekr",
    "Miniaturas ao percorrer a barra de tempo. Ajustes › Reprodução › Seekr." },
  { ID_AJUSTES,   "menu_settings", "Ajustes",
    "Em categorias, com busca e as opções avançadas à parte." },
  { ID_RETOMAR,   "play",          "Retomar",
    "Voltar a um filme pausado abre direto a fonte que estava tocando." },
  { ID_CONSERTOS, "check",         "Consertos",
    "Protetor de tela da LG desligado no filme, Android mais estável e perfis separados." },
};
#define N172_NI ((int)(sizeof ITENS / sizeof *ITENS))

// A cor do disco de cada linha, na ordem das listras do logo.
static void corItem(int i, float *r, float *g, float *b) {
  switch (i % 3) {
    case 0:  *r = LARANJA_R;  *g = LARANJA_G;  *b = LARANJA_B;  break;
    case 1:  *r = AMARELO_R;  *g = AMARELO_G;  *b = AMARELO_B;  break;
    default: *r = VERMELHO_R; *g = VERMELHO_G; *b = VERMELHO_B; break;
  }
}

// ------------------------------------------------------------------- estado
int novidades172_itens(void) { return N172_NI; }
int novidades172_item_largura(int i, int *limite, const char **nome) {
  float tx = N172_TXT_X + N172_ICONE + 20.0f;
  if (i < 0 || i >= N172_NI) return 0;
  if (limite) *limite = (int)(N172_TXT_X + N172_TXT_W - tx);
  if (nome) *nome = ITENS[i].nome;
  return txt_largura(TXT_CAPTION, i18n(ITENS[i].linha));
}
int novidades172_cenas(void) { return N172_NC; }
int novidades172_aberto(void) { return aberto; }

void novidades172_dir(const char *d) {
  if (d && d[0]) snprintf(dirArte, sizeof dirArte, "%s", d);
  montarCaminhos();
}

static void mudarCena(int nova) {
  if (nova < 0) nova = N172_NC - 1;
  if (nova >= N172_NC) nova = 0;
  if (nova == cena) return;
  tempoAntiga = relogioCena;
  cenaAntiga = cena;
  cena = nova;
  transicao = ajustes_animacoes_reduzidas() ? 1.0f : 0.0f;
  relogioCena = 0.0f;
}

void novidades172_ir(int c, float t) {
  cena = cenaAntiga = (c % N172_NC + N172_NC) % N172_NC;
  transicao = 1.0f;
  relogioCena = t;
}

static void comecar(float e) {
  aberto = decidido = 1;
  foco = B_OK;
  naPrevia = 0;
  cena = cenaAntiga = 0;
  entrada = e;
  transicao = 1.0f;
  relogioCena = tempoAntiga = 0.0f;
  if (!arqAbertura[0]) montarCaminhos();
  pedirArtes();
}

void novidades172_abrir(void) { comecar(1.0f); }

void novidades172_primeira_vez(void) {
  char *s;
  if (decidido) return;
  decidido = 1;
  s = dados_ler(N172_ARQ);
  if (s) { free(s); return; }
  comecar(0.0f);
}

static void fechar(void) {
  aberto = 0;
  dados_gravar(N172_ARQ, "1\n");
  // O QR so existe enquanto o cartao esta na tela.
  if (texQr) { gfx_tex_esquecer(texQr); glDeleteTextures(1, &texQr); texQr = 0; }
}

void novidades172_evento(const SDL_Event *e) {
  SDL_Keycode k;
  if (!aberto || !e || e->type != SDL_KEYDOWN) return;
  k = e->key.keysym.sym;
  if (k == SDLK_UP)   { naPrevia = 1; return; }
  if (k == SDLK_DOWN) { naPrevia = 0; return; }
  if (k == SDLK_LEFT) {
    if (naPrevia) mudarCena(cena - 1);
    else if (foco > 0) foco--;
    return;
  }
  if (k == SDLK_RIGHT) {
    if (naPrevia) mudarCena(cena + 1);
    else if (foco < B_N - 1) foco++;
    return;
  }
  if (k == SDLK_RETURN || k == SDLK_KP_ENTER || k == SDLK_SPACE) {
    if (naPrevia) { mudarCena(cena + 1); return; }
    fechar();
    return;
  }
  if (k == SDLK_AC_BACK || k == SDLK_ESCAPE || k == SDLK_BACKSPACE ||
      k == SDLK_DELETE || e->key.keysym.scancode == NV_SCANCODE_BACK)
    fechar();
}

void novidades172_atualizar(float dt, Uint32 agora) {
  (void)agora;
  if (!aberto && entrada < 0.002f) { entrada = 0.0f; return; }
  if (aberto) pedirArtes();
  if (ajustes_animacoes_reduzidas()) {
    entrada = aberto ? 1.0f : 0.0f;
    transicao = 1.0f;
    if (aberto) {
      relogioCena += dt;
      if (relogioCena >= CENAS[cena].duracao) mudarCena(cena + 1);
    }
    return;
  }
  entrada = anim_rampa(entrada, aberto ? 1.0f : 0.0f, dt, aberto ? N172_ABRIR_MS : N172_FECHAR_MS);
  if (aberto) {
    if (transicao < 1.0f) {
      transicao += dt / N172_TRANSICAO_S;
      if (transicao > 1.0f) transicao = 1.0f;
      tempoAntiga += dt;
    }
    relogioCena += dt;
    if (relogioCena >= CENAS[cena].duracao) mudarCena(cena + 1);
  }
}

// ------------------------------------------------------------------ desenho
static void desenhaCena(int c, float x, float y, float t, float a) {
  if (a <= 0.003f) return;
  CENAS[c].desenhar(x, y, ajustes_animacoes_reduzidas() ? CENAS[c].estatico : t, a);
}

// O selo com o nome da cena e os tracos do ciclo, no alto da previa.
static GfxRect seloRect;
static void selo(float x, float y, float a) {
  const char *nome = i18n(CENAS[cena].nome);
  TxtLinha l = txt_linha(TXT_CAPTION, nome, CREME_I, 255);
  TxtLinha lf = txt_linha(TXT_CAPTION, nome, AMEIXA_I, 255);
  GfxRect s = { x + 24.0f, y + 24.0f, (float)l.w + 36.0f, 44.0f };
  float tA = anim_suave(transicao);
  int i;
  if (naPrevia) gfx_cor(s, 0.5f, LARANJA_R, LARANJA_G, LARANJA_B, a);
  else gfx_cor(s, 0.5f, AMEIXA_R, AMEIXA_G, AMEIXA_B, 0.72f * a);
  txt_desenhar_alpha(naPrevia ? lf : l, s.x + 18.0f, s.y + (s.h - (float)l.h) * 0.5f, a * tA);
  seloRect = s;
  for (i = 0; i < N172_NC; i++) {
    float tw = 26.0f, gap = 8.0f;
    float tx = x + N172_PV_W - 24.0f - (float)(N172_NC - i) * (tw + gap) + gap;
    GfxRect tr = { tx, y + 44.0f, tw, 4.0f };
    gfx_cor(tr, 0.5f, CREME_R, CREME_G, CREME_B, 0.28f * a);
    if (i < cena) gfx_cor(tr, 0.5f, CREME_R, CREME_G, CREME_B, 0.70f * a);
    if (i == cena) {
      float p = anim_clamp(relogioCena / CENAS[cena].duracao, 0.0f, 1.0f);
      tr.w *= p;
      if (tr.w > 1.0f) gfx_cor(tr, 0.5f, LARANJA_R, LARANJA_G, LARANJA_B, a);
    }
  }
}

// Uma linha da lista. `vivo` 0..1: a cena na previa fala desta linha.
static void desenhaItem(int i, float y, float vivo, int maxL, float a) {
  float tx = N172_TXT_X + N172_ICONE + 20.0f, tw = N172_TXT_X + N172_TXT_W - tx;
  GfxRect d = { N172_TXT_X, y + 4.0f, N172_ICONE, N172_ICONE };
  GfxRect ic = { d.x + 9.0f, d.y + 9.0f, 22.0f, 22.0f };
  float cr, cg, cb;
  corItem(i, &cr, &cg, &cb);
  gfx_cor(d, 0.5f, cr, cg, cb, (0.20f + 0.80f * vivo) * a);
  gfx_icone(ic, ITENS[i].icone, CREME_R + (0.13f - CREME_R) * vivo, CREME_G + (0.05f - CREME_G) * vivo,
            CREME_B + (0.12f - CREME_B) * vivo, a);
  { TxtLinha n = txt_linha_corta(TXT_BODY, i18n(ITENS[i].nome), CREME_I, 255, tw);
    txt_desenhar_alpha(n, tx, y, a); }
  txt_bloco_corta(TXT_CAPTION, i18n(ITENS[i].linha), CREME2_I, tx, y + 33.0f, tw, 27.0f, a, maxL);
}

// Mede a lista e distribui o espaco: quem nao cabe numa linha ganha a segunda
// enquanto houver altura; o vao entre as linhas e o que sobrar.
static float linhaH[16];
static int linhaMax[16];
static void medirLista(float alto, float *gap) {
  float tx = N172_TXT_X + N172_ICONE + 20.0f, tw = N172_TXT_X + N172_TXT_W - tx;
  float soma = 60.0f * (float)N172_NI, folga;
  int i;
  folga = alto - soma - 8.0f * (float)(N172_NI - 1);
  for (i = 0; i < N172_NI; i++) {
    int larga = txt_largura(TXT_CAPTION, i18n(ITENS[i].linha)) > (int)tw;
    linhaMax[i] = 1;
    linhaH[i] = 60.0f;
    if (larga && folga >= 27.0f) {
      linhaMax[i] = 2; linhaH[i] += 27.0f; soma += 27.0f; folga -= 27.0f;
    }
  }
  *gap = N172_NI > 1 ? anim_clamp((alto - soma) / (float)(N172_NI - 1), 8.0f, 34.0f) : 0.0f;
}

static void ponteiroFoco(int b, int nada) { (void)nada; foco = b; naPrevia = 0; }
static void ponteiroOk(int b, int nada) { (void)b; (void)nada; fechar(); }
static void focarSelo(int nada, int nada2) { (void)nada; (void)nada2; naPrevia = 1; }
static void ponteiroSelo(int nada, int nada2) { (void)nada; (void)nada2; naPrevia = 1; mudarCena(cena + 1); }

void novidades172_desenhar(Uint32 agora) {
  float a = anim_suave(entrada), dy, y0;
  GfxRect card;
  (void)agora;
  if (entrada < 0.002f) return;
  if (aberto) ponteiro_camada();
  gfx_cor((GfxRect){ 0, 0, NV_TELA_W, NV_TELA_H }, 0.0f, 0, 0, 0, 0.78f * entrada);
  dy = (1.0f - a) * 34.0f;
  y0 = N172_Y + dy;
  card = (GfxRect){ N172_X, y0, N172_W, N172_H };
  // O cartao: ameixa, as listras do login bem baixas e o brilho laranja que
  // vem do canto da previa.
  gfx_cor(card, N172_RAIO / N172_H, AMEIXA_R, AMEIXA_G, AMEIXA_B, 0.97f * a);
  cobrir(arqListras, card, N172_RAIO, 0.24f * a);
  gfx_rect(card, 0, GFX_ANEL, 0, 1.4f / N172_H, 0, N172_RAIO / N172_H, CREME_R, CREME_G, CREME_B, 0.14f * a);
  gfx_luz_canto(card, N172_RAIO / N172_H, N172_PAD + N172_PV_W * 0.5f, -120.0f, 820.0f,
                LARANJA_R, LARANJA_G, LARANJA_B, 0.10f * a);
  gfx_recorte(card.x, card.y, card.w, card.h);

  // ----- a previa
  { float px = N172_X + N172_PAD, py = y0 + N172_PAD;
    float t = anim_suave(transicao);
    GfxRect pv = { px, py, N172_PV_W, N172_PV_H };
    gfx_recorte(pv.x, pv.y, pv.w, pv.h);
    gfx_cor(pv, rr(N172_PV_RAIO, pv), AMEIXA_R, AMEIXA_G, AMEIXA_B, a);
    if (transicao < 1.0f && cenaAntiga != cena)
      desenhaCena(cenaAntiga, px - t * 24.0f, py, tempoAntiga, a * (1.0f - anim_clamp(t / 0.5f, 0.0f, 1.0f)));
    desenhaCena(cena, px + (1.0f - t) * 24.0f, py, relogioCena,
                a * (cenaAntiga != cena ? anim_clamp((t - 0.38f) / 0.62f, 0.0f, 1.0f) : t));
    gfx_rect(pv, 0, GFX_ANEL, 0, 1.4f / pv.h, 0, rr(N172_PV_RAIO, pv), CREME_R, CREME_G, CREME_B, 0.16f * a);
    selo(px, py, a);
    if (aberto)
      ponteiro_alvo(seloRect.x, seloRect.y, seloRect.w, seloRect.h, focarSelo, ponteiroSelo, 0, 0);
    gfx_recorte(card.x, card.y, card.w, card.h); }

  // ----- a coluna da direita: marca, titulo, listras e a lista.
  { char tit[64];
    float ya = y0 + N172_PAD, gap, alto, yy;
    int i;
    snprintf(tit, sizeof tit, i18n("Novidades da %s"), N172_VERSAO);
    { TxtLinha k = txt_linha(TXT_CAPTION2, "NUVIO LEGACY", LARANJA_I, 255);
      txt_desenhar_alpha(k, N172_TXT_X, ya, a); }
    txt_bloco(TXT_TITULO2, tit, CREME_I, N172_TXT_X, ya + 30.0f, N172_TXT_W, 62.0f, a, 1);
    listras(N172_TXT_X, ya + 30.0f + 74.0f, 210.0f, a);
    yy = ya + 30.0f + 74.0f + 26.0f + 26.0f;
    alto = (y0 + N172_PAD + N172_PV_H) - yy;
    medirLista(alto, &gap);
    for (i = 0; i < N172_NI; i++) {
      float vivo = 0.0f, tA = anim_suave(transicao);
      float local = ajustes_animacoes_reduzidas() ? 1.0f
                  : anim_clamp((a - 0.04f * (float)i) * 3.0f, 0.0f, 1.0f);
      if (ITENS[i].id == CENAS[cena].id) vivo += tA;
      if (transicao < 1.0f && ITENS[i].id == CENAS[cenaAntiga].id) vivo += 1.0f - tA;
      if (vivo > 1.0f) vivo = 1.0f;
      desenhaItem(i, yy + (1.0f - local) * 12.0f, vivo, linhaMax[i], a * local);
      yy += linhaH[i] + gap;
    } }

  // ----- o rodape: a dica do D-pad a esquerda, os dois botoes a direita.
  { float yBase = y0 + N172_H - N172_PAD;
    const char *rotOk = i18n("Entendi");
    const char *rotD = i18n("Agora não");
    float wOk = pilulaLargura(rotOk, "check"), wD = pilulaLargura(rotD, NULL);
    GfxRect bOk = { N172_X + N172_W - N172_PAD - wOk, yBase - N172_BT_H, wOk, N172_BT_H };
    GfxRect bD = { bOk.x - N172_BT_GAP - wD, yBase - N172_BT_H * 0.5f - 30.0f, wD, 60.0f };
    { TxtLinha h = txt_linha_corta(TXT_CAPTION2,
                     i18n(naPrevia ? "← → Trocar a prévia  ·  ↓ Botões" : "↑ Escolher a prévia"),
                     CREME2_I, 255, bD.x - 32.0f - (N172_X + N172_PAD));
      txt_desenhar_alpha(h, N172_X + N172_PAD + 4.0f, yBase - N172_BT_H * 0.5f - (float)h.h * 0.5f,
                         a * 0.9f); }
    pilula(bD, rotD, NULL, !naPrevia && foco == B_DEPOIS ? 1.0f : 0.0f, 0, a);
    pilula(bOk, rotOk, "check", !naPrevia && foco == B_OK ? 1.0f : 0.0f, 1, a);
    if (aberto) {
      ponteiro_alvo(bD.x, bD.y, bD.w, bD.h, ponteiroFoco, ponteiroOk, B_DEPOIS, 0);
      ponteiro_alvo(bOk.x, bOk.y, bOk.w, bOk.h, ponteiroFoco, ponteiroOk, B_OK, 0);
    } }
  gfx_sem_recorte();
}
