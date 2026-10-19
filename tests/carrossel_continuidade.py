#!/usr/bin/env python3
"""Compara o fundo dos lados do limiar capturado por NV_CAR_LIMIAR=1.

Uso: python3 tests/carrossel_continuidade.py /tmp/captura [outras capturas]
Sem dependencias; ignora a borda que ainda move uma fracao de pixel.
"""
import collections
import pathlib
import struct
import sys


def bmp(path):
    data = path.read_bytes()
    offset = struct.unpack_from('<I', data, 10)[0]
    width, height = struct.unpack_from('<ii', data, 18)
    bits = struct.unpack_from('<H', data, 28)[0]
    assert data[:2] == b'BM' and bits == 32 and height > 0
    return data, offset, width, height


for directory in sys.argv[1:]:
    a, off, width, height = bmp(pathlib.Path(directory) / 'c-limiar-4.bmp')
    b, otheroff, otherwidth, otherheight = bmp(pathlib.Path(directory) / 'c-limiar-5.bmp')
    assert (off, width, height) == (otheroff, otherwidth, otherheight)
    histogram = collections.Counter()
    for y in range(8, height - 8):
        for x in range(8, width - 8):
            pos = off + (y * width + x) * 4
            for channel in range(3):
                histogram[abs(a[pos + channel] - b[pos + channel])] += 1
    count = sum(histogram.values())
    mean = sum(delta * n for delta, n in histogram.items()) / count
    cumulative = 0
    for delta, n in sorted(histogram.items()):
        cumulative += n
        if cumulative >= count * .95:
            p95 = delta
            break
    print(f'{directory}: mean={mean:.3f}/255 p95={p95} max={max(histogram)}')
    # O grao de 8 bits e a geometria subpixel podem variar em 1-2 niveis.
    # O bug original produzia p95=16-45, mesmo sem nenhum texto na captura.
    assert mean <= 1.0 and p95 <= 3, 'salto do fundo no limiar do carrossel'
