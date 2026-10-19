#!/usr/bin/env python3
"""Regera deploy/app/fonts/NotoNaskhArabic-Subset.ttf, a fonte arabe embarcada.

POR QUE EXISTE. text.c manda arabe para uma fonte de RESERVA. Na LG, no Android
e no Mac ela vem do sistema; na Samsung nao ha fonte arabe nenhuma e o texto
saia em quadradinhos (#253, #258). Ver docs/fontes/README.md.

O QUE TEM DE TER. Nao ha HarfBuzz: src/bidi.c faz a juncao trocando cada letra
pela FORMA DE APRESENTACAO (U+FE70-FEFF, e U+FB50-FDFF para persa/urdu). O
recorte precisa desses dois blocos, nao so do bloco base U+0600-06FF.

ENTRADA. Noto Naskh Arabic Regular (OFL 1.1), de github.com/notofonts/arabic:
    fonts/NotoNaskhArabic/hinted/ttf/NotoNaskhArabic-Regular.ttf
SAIDA. Sem hinting e sem tabelas de layout (ninguem as le aqui).

Uso:  python3 tools/fonte-arabe.py /caminho/NotoNaskhArabic-Regular.ttf
Precisa de fontTools (pip install fonttools).
"""
import pathlib, subprocess, sys

RAIZ = pathlib.Path(__file__).resolve().parent.parent
DESTINO = RAIZ / "deploy" / "app" / "fonts" / "NotoNaskhArabic-Subset.ttf"
FAIXAS = ("U+0020-007F,U+00A0,U+0600-06FF,U+0750-077F,U+FB50-FDFF,U+FE70-FEFF,"
          "U+200C-200F,U+2010-2027")

def main():
    if len(sys.argv) != 2:
        raise SystemExit(__doc__)
    subprocess.check_call([sys.executable, "-m", "fontTools.subset", sys.argv[1],
                           "--unicodes=%s" % FAIXAS, "--output-file=%s" % DESTINO,
                           "--no-hinting", "--desubroutinize", "--layout-features=",
                           "--notdef-outline", "--drop-tables+=DSIG"])
    print("%s: %d bytes" % (DESTINO, DESTINO.stat().st_size))

if __name__ == "__main__":
    main()
