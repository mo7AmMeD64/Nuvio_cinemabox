#!/bin/bash
export TMPDIR="${TMPDIR:-/tmp}"
TMPDIR="${TMPDIR%/}"
# txt_largura == largura rasterizada; blocos so rasterizam as linhas finais.
#   bash tests/text_largura.sh
set -eu
cd "$(dirname "$0")/.."
sources=()
for source in src/*.c src/dts/*.c; do [ "$source" != src/main.c ] && sources+=("$source"); done
cc "${sources[@]}" tests/text_largura.c -Isrc -o ${TMPDIR:-/tmp}/nuvio-text-largura \
  -O1 -g -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 \
  -L/opt/homebrew/lib -lSDL2 -lSDL2_image -lSDL2_ttf -lz -framework OpenGL \
  -Wno-deprecated-declarations -Wno-macro-redefined
NUVIO_DADOS=$(mktemp -d) ${TMPDIR:-/tmp}/nuvio-text-largura
