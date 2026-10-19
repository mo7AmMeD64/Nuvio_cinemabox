#!/usr/bin/env python3
"""Rebuild deploy/app/art/selos/ from the two Xperience badge packs.

  padrao.json   default pack  (kingsize, white monochrome art, tinted by the app)
  colorido.json colored pack  (elite, full-colour art, used when "Selos
                               coloridos" is on)

The JSONs are the packs exactly as published (groups + filters, patterns and
CDN imageURLs untouched) minus the filters whose regex cannot be used (see
SKIP below). Each filter's art is downloaded from its imageURL, scaled down to
fit 400x128 px (the row draws it at ~24 px) and stored as
selos/<flattened-url-path>.webp. src/selospacote.c derives the same local name
from the imageURL, so there is no second index to keep in sync.

  python3 tools/selos-xperience.py [--padrao URL_OR_FILE] [--colorido URL_OR_FILE]

Needs cwebp and magick (ImageMagick) on the PATH. Network only for the packs
and the images; the app never touches the CDN unless SELOS_IMAGENS_NA_REDE is
set in src/selospacote.h.
"""
import argparse, json, os, subprocess, sys, tempfile, urllib.request

PADRAO = "https://xperience-app.com/badges/557fe8ef-5819-4fb2-934c-8171f1829d42.json"
COLORIDO = "https://xperience-app.com/badges/eda281ff-3dfa-4b6a-95c1-4f77cfabd739.json"
RAIZ = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "deploy", "app", "art", "selos")
# Patterns that hide state in invisible Unicode markers (the Xperience app tags
# release titles with zero-width sequences; nothing the TV sees carries them).
SKIP = {"gq-hc-hdrip", "black-and-white", "true-hue-edition"}

def ler(fonte):
    if os.path.exists(fonte):
        return json.load(open(fonte))
    req = urllib.request.Request(fonte, headers={"User-Agent": "Mozilla/5.0"})
    return json.load(urllib.request.urlopen(req, timeout=30))

def plano(url):  # keep in sync with nomeLocal() in src/selospacote.c
    i = url.find("/badges/")
    return url[i + 8:].replace("/", "_") if i >= 0 else url.rsplit("/", 1)[-1]

def imagem(url, destino):
    req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"})
    bruto = urllib.request.urlopen(req, timeout=30).read()
    with tempfile.TemporaryDirectory() as t:
        a, b = os.path.join(t, "a.webp"), os.path.join(t, "b.png")
        open(a, "wb").write(bruto)
        subprocess.run(["magick", a, "-resize", "400x128>", b], check=True)
        subprocess.run(["cwebp", "-quiet", "-q", "88", "-alpha_q", "100", "-m", "6", b, "-o", destino], check=True)

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--padrao", default=PADRAO)
    ap.add_argument("--colorido", default=COLORIDO)
    a = ap.parse_args()
    os.makedirs(RAIZ, exist_ok=True)
    total = 0
    for nome, fonte in (("padrao", a.padrao), ("colorido", a.colorido)):
        d = ler(fonte)
        d["filters"] = [f for f in d["filters"] if f["id"] not in SKIP]
        for f in d["filters"]:
            dest = os.path.join(RAIZ, plano(f["imageURL"]))
            if not os.path.exists(dest):
                imagem(f["imageURL"], dest)
            total += os.path.getsize(dest)
        json.dump(d, open(os.path.join(RAIZ, nome + ".json"), "w"), ensure_ascii=False, separators=(",", ":"))
        print(nome, len(d["filters"]), "filters")
    print("art bytes:", total)

if __name__ == "__main__":
    sys.exit(main())
