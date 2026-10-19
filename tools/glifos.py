#!/usr/bin/env python3
"""Codepoints que a Inter embarcada (Regular, Medium e Bold ao mesmo tempo)
desenha, um em hexadecimal por linha. Usado por tests/limpa.sh para conferir que
o texto de addon limpo so tem glifo que existe (#144). Reusa o leitor de cmap de
tools/idiomas.py, que nao depende de fontTools.

    python3 tools/glifos.py [prefixo-da-familia]   # padrao: InterDisplay
"""
import pathlib
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import idiomas  # noqa: E402

FONTES = pathlib.Path(__file__).resolve().parent.parent / "deploy" / "app" / "fonts"


def main():
    prefixo = sys.argv[1] if len(sys.argv) > 1 else "InterDisplay"
    conjuntos = [idiomas.cmap_ttf(f) for f in sorted(FONTES.glob(prefixo + "*.ttf"))]
    if not conjuntos:
        sys.exit("nenhuma fonte com o prefixo " + prefixo)
    for cp in sorted(set.intersection(*conjuntos)):
        print("%X" % cp)


if __name__ == "__main__":
    main()
