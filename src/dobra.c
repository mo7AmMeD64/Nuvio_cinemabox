// Ver dobra.h.
#include "dobra.h"

// Versalete do IPA e do bloco fonetico, na ordem do alfabeto. O Unicode nao
// tem os 26 num bloco so: vem de U+1D00 (fonetico), U+02xx (IPA) e U+A7xx
// (latim estendido-D), e ainda falta um de verdade (X, que ninguem usa).
static const struct { unsigned short cp; char c; } VERSALETE[] = {
  { 0x1D00, 'A' }, { 0x0299, 'B' }, { 0x1D04, 'C' }, { 0x1D05, 'D' },
  { 0x1D07, 'E' }, { 0xA730, 'F' }, { 0x0262, 'G' }, { 0x029C, 'H' },
  { 0x026A, 'I' }, { 0x1D0A, 'J' }, { 0x1D0B, 'K' }, { 0x029F, 'L' },
  { 0x1D0D, 'M' }, { 0x0274, 'N' }, { 0x1D0F, 'O' }, { 0x1D18, 'P' },
  { 0x01EB, 'Q' }, { 0x0280, 'R' }, { 0xA731, 'S' }, { 0x1D1B, 'T' },
  { 0x1D1C, 'U' }, { 0x1D20, 'V' }, { 0x1D21, 'W' }, { 0x028F, 'Y' },
  { 0x1D22, 'Z' },
};

// Letras modificadoras (sobrescrito): as maiusculas do bloco fonetico e as
// minusculas do IPA/latim. Faltam as que o Unicode nao tem (q, ᴿ em alguns).
static const struct { unsigned short cp; char c; } MODIFICADORA[] = {
  { 0x1D2C, 'A' }, { 0x1D2E, 'B' }, { 0x1D30, 'D' }, { 0x1D31, 'E' },
  { 0x1D33, 'G' }, { 0x1D34, 'H' }, { 0x1D35, 'I' }, { 0x1D36, 'J' },
  { 0x1D37, 'K' }, { 0x1D38, 'L' }, { 0x1D39, 'M' }, { 0x1D3A, 'N' },
  { 0x1D3C, 'O' }, { 0x1D3E, 'P' }, { 0x1D3F, 'R' }, { 0x1D40, 'T' },
  { 0x1D41, 'U' }, { 0x2C7D, 'V' }, { 0x1D42, 'W' },
  { 0x1D43, 'a' }, { 0x1D47, 'b' }, { 0x1D9C, 'c' }, { 0x1D48, 'd' },
  { 0x1D49, 'e' }, { 0x1DA0, 'f' }, { 0x1D4D, 'g' }, { 0x02B0, 'h' },
  { 0x2071, 'i' }, { 0x02B2, 'j' }, { 0x1D4F, 'k' }, { 0x02E1, 'l' },
  { 0x1D50, 'm' }, { 0x207F, 'n' }, { 0x1D52, 'o' }, { 0x1D56, 'p' },
  { 0x02B3, 'r' }, { 0x02E2, 's' }, { 0x1D57, 't' }, { 0x1D58, 'u' },
  { 0x1D5B, 'v' }, { 0x02B7, 'w' }, { 0x02E3, 'x' }, { 0x02B8, 'y' },
  { 0x1DBB, 'z' },
};

char nv_dobra_estilizada(unsigned long cp) {
  unsigned i;
  if (cp < 0x80) return 0;
  for (i = 0; i < sizeof VERSALETE / sizeof *VERSALETE; i++)
    if (VERSALETE[i].cp == cp) return VERSALETE[i].c;
  // Sobrescrito e subscrito: ⁰ ⁴-⁹ e ₀-₉ (¹ ² ³ sao do Latin-1, que a fonte tem).
  if (cp == 0x2070) return '0';
  if (cp >= 0x2074 && cp <= 0x2079) return (char)('4' + (cp - 0x2074));
  if (cp >= 0x2080 && cp <= 0x2089) return (char)('0' + (cp - 0x2080));
  // Circuladas: Ⓐ-Ⓩ, ⓐ-ⓩ, ①-⑨.
  if (cp >= 0x24B6 && cp <= 0x24CF) return (char)('A' + (cp - 0x24B6));
  if (cp >= 0x24D0 && cp <= 0x24E9) return (char)('a' + (cp - 0x24D0));
  if (cp >= 0x2460 && cp <= 0x2468) return (char)('1' + (cp - 0x2460));
  // Largura cheia: ！ a ～ espelham o ASCII de 0x21 a 0x7E.
  if (cp >= 0xFF01 && cp <= 0xFF5E) return (char)(cp - 0xFEE0);
  // Alfanumericos matematicos: treze estilos de 52 letras (A-Z, a-z) a partir
  // de U+1D400, e cinco de 10 digitos a partir de U+1D7CE. Os "buracos" do
  // bloco (letras que ja existiam em outro lugar, como o ℎ) nao sao atribuidos
  // e nao aparecem em texto real; cair na letra certa ali e inofensivo.
  if (cp >= 0x1D400 && cp <= 0x1D6A3) {
    unsigned off = (unsigned)((cp - 0x1D400) % 52);
    return off < 26 ? (char)('A' + off) : (char)('a' + off - 26);
  }
  if (cp >= 0x1D7CE && cp <= 0x1D7FF) return (char)('0' + (cp - 0x1D7CE) % 10);
  // Letras quadradas e circuladas escuras do bloco de emoji (🄰 🅐 🅰): A-Z.
  if (cp >= 0x1F130 && cp <= 0x1F149) return (char)('A' + (cp - 0x1F130));
  if (cp >= 0x1F150 && cp <= 0x1F169) return (char)('A' + (cp - 0x1F150));
  if (cp >= 0x1F170 && cp <= 0x1F189) return (char)('A' + (cp - 0x1F170));
  // Letras modificadoras ("HD" e "HDR" escritos como sobrescrito: ᴴᴰ, ʰᵈ).
  for (i = 0; i < sizeof MODIFICADORA / sizeof *MODIFICADORA; i++)
    if (MODIFICADORA[i].cp == cp) return MODIFICADORA[i].c;
  return 0;
}
