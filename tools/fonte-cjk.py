#!/usr/bin/env python3
"""Regera deploy/app/fonts/DroidSansFallback-Subset.ttf, a fonte CJK embarcada.

POR QUE EXISTE. text.c manda japones e chines para uma fonte de RESERVA. Na LG e
no Mac ela vem do sistema; no WASM da Samsung nao ha fonte de sistema nenhuma, e
sem este arquivo ja/zh saem em quadradinhos. Ver docs/fontes/README.md.

ENTRADA. A Droid Sans Fallback 2.54 (Apache 2.0, Google 2006) que a propria TV LG
traz em /usr/share/fonts/DroidSansFallback.ttf:
    scp root@<tv>:/usr/share/fonts/DroidSansFallback.ttf /tmp/
SAIDA. ~7.900 caracteres, sem hinting, ~1,2 MB:
  - ASCII, Latin-1, pontuacao geral, setas, pontuacao CJK, kana, largura total;
  - todo caractere usado nas tabelas ja/zhcn/zhtw (fica coberto por construcao);
  - GB2312 nivel 1 e 2 parcial (hanzi simplificados de uso corrente), JIS X 0208
    nivel 1 (kanji do dia a dia) e Big5 de uso corrente (hanzi tradicionais):
    o que titulos e sinopses vindos do TMDB mais usam, para nao voltar tofu
    quando a interface esta em ingles e o filme e coreano ou chines.

Uso:  python3 tools/fonte-cjk.py /tmp/DroidSansFallback.ttf
Precisa de fontTools (pip install fonttools).
"""
import pathlib, subprocess, sys

RAIZ = pathlib.Path(__file__).resolve().parent.parent
sys.path.insert(0, str(RAIZ / "tools"))
import idiomas as I  # noqa: E402  (reusa o parser das tabelas)

def faixa(a, b):
    return {chr(c) for c in range(a, b + 1)}

def decodifica(pares1, pares2, codec):
    out = set()
    for b1 in pares1:
        for b2 in pares2:
            try: out.add(bytes([b1, b2]).decode(codec))
            except UnicodeDecodeError: pass
    return out

def main():
    if len(sys.argv) != 2:
        raise SystemExit(__doc__)
    origem = pathlib.Path(sys.argv[1])
    usados = set()
    for cod in ("ja", "zhcn", "zhtw"):
        for _n, _k, val in I.ler_irma(cod):
            usados |= set(I.decodificar(val).decode("utf-8"))
    gb = decodifica(range(0xB0, 0xD8), range(0xA1, 0xFF), "gb2312")
    jis = decodifica(range(0x88, 0x99), range(0x40, 0xFD), "shift_jis")
    b5 = decodifica(range(0xA4, 0xC7), list(range(0x40, 0x7F)) + list(range(0xA1, 0xFF)), "big5")
    base = (faixa(0x20, 0x7E) | faixa(0xA0, 0xFF) | faixa(0x2010, 0x2027) | faixa(0x2190, 0x2193) |
            faixa(0x3000, 0x303F) | faixa(0x3040, 0x30FF) | faixa(0xFF00, 0xFFEF) | set("★▶✓•"))
    todos = usados | base | gb | jis | b5
    texto = pathlib.Path("/tmp/nuvio-cjk-caracteres.txt")
    texto.write_text("".join(sorted(todos)), encoding="utf-8")
    destino = I.FONTES / I.FONTE_CJK
    subprocess.check_call([sys.executable, "-m", "fontTools.subset", str(origem),
                           "--text-file=%s" % texto, "--output-file=%s" % destino,
                           "--no-hinting", "--desubroutinize", "--layout-features=",
                           "--notdef-outline", "--drop-tables+=DSIG"])
    print("%s: %d caracteres pedidos, %d bytes" % (destino, len(todos), destino.stat().st_size))

if __name__ == "__main__":
    main()
