#!/bin/bash
# Icones do PLAYER no Glass UI (mockup aprovado em 03/10, player-mockup.html).
#
# Os SVG de deploy/app/art/icones/player/ sao os do mockup: desenhos do Lucide
# (lucide.dev, licenca ISC — deploy/app/art/icones/lucide/LICENSE) mais tres
# do proprio mockup (play-f e pause-f cheios, ratio). Traco 2 (2,4 no check e
# nas setas, como no mockup) em grade 24.
#
# Rasteriza cada um em deploy/app/art/icones/pl_<nome>.png, 128x128, BRANCO
# sobre transparente com a forma no ALFA — o mesmo caminho de
# tools/icones-lucide.sh (gfx_icone tinge pelo alfa). O prefixo pl_ separa
# estes dos de outras telas.
#
# Uso: bash tools/icones-player.sh      Precisa de: sips (macOS), magick.
set -e
cd "$(dirname "$0")/.."
DIR="deploy/app/art/icones"
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
for f in "$DIR"/player/*.svg; do
  n=$(basename "$f" .svg)
  sed -e 's/currentColor/#ffffff/g' -e 's/width="24"/width="512"/' \
      -e 's/height="24"/height="512"/' "$f" > "$TMP/a.svg"
  sips -s format png "$TMP/a.svg" --out "$TMP/a.png" >/dev/null
  magick "$TMP/a.png" -alpha extract -filter Box -resize 128x128 "$TMP/m.png"
  magick -size 128x128 xc:white "$TMP/m.png" -alpha off -compose CopyOpacity \
    -composite -strip -define png:color-type=6 "$DIR/pl_$n.png"
done
USADOS=$(grep -ho '"pl_[a-z0-9-]*"' src/*.c | tr -d '"' | sed 's/^pl_//' | sort -u)
for n in $USADOS; do
  [ -f "$DIR/player/$n.svg" ] || { echo "icones-player: src/ usa pl_$n sem SVG" >&2; exit 1; }
done
echo "icones-player: $(ls "$DIR"/player/*.svg | wc -l | tr -d ' ') icones em $DIR/pl_*.png"
