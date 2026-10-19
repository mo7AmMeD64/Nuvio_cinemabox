#!/bin/bash
# Icones do MENU LATERAL (layouts Moderna e Padrao), de traco fino.
#
# POR QUE EXISTE: os menu_*.png sao os glifos CHEIOS do sidebar do app web (a
# casa com porta, a engrenagem macica). O mockup aprovado pelo dono ("Glass UI
# — ilha", tela 1, 02/10/2026) troca por desenhos de LINHA no estilo do Lucide
# — traco 1,9 em grade 24, ponta e junta redondas — e o dono cobrou a build
# por ter ficado "bem menos polida" que o mockup. Os SVG de
# deploy/app/art/icones/menu-traco/ sao os PATHS DO PROPRIO MOCKUP, copiados
# sem editar (nao os do Lucide upstream: o compasso e a biblioteca de la tem
# outro desenho, e o aprovado foi o do mockup).
#
# Nome com prefixo mt_ (menu-traco) e nao menu_*: o layout Dinamica (barra da
# Apple TV, menu.c) continua com os menu_*.png de sempre, e trocar o arquivo
# trocaria as duas barras de uma vez.
#
# Rasterizacao: a mesma de tools/icones-lucide.sh (512 pelo sips, reduzido a
# 128 com filtro box, forma no ALFA, branco), para o peso do traco sair limpo.
#
# Uso: bash tools/icones-menu.sh     Precisa de: sips (macOS), magick.
set -e
cd "$(dirname "$0")/.."
DIR="deploy/app/art/icones"
SVG="$DIR/menu-traco"
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
for f in "$SVG"/*.svg; do
  n=$(basename "$f" .svg)
  sed -e 's/currentColor/#ffffff/g' -e 's/width="24"/width="512"/' \
      -e 's/height="24"/height="512"/' "$f" > "$TMP/a.svg"
  sips -s format png "$TMP/a.svg" --out "$TMP/a.png" >/dev/null
  magick "$TMP/a.png" -alpha extract -filter Box -resize 128x128 "$TMP/m.png"
  magick -size 128x128 xc:white "$TMP/m.png" -alpha off -compose CopyOpacity \
    -composite -strip -define png:color-type=6 "$DIR/mt_$n.png"
done
echo "icones-menu: $(ls "$SVG"/*.svg | wc -l | tr -d ' ') icones em $DIR/mt_*.png"
