#!/bin/bash
set -eu
cd "$(dirname "$0")/.."
saida="${1:-/Volumes/ExternalSSD/tmp/rev/barraatv}"
mkdir -p "$saida"
NUVIO_DADOS=$(mktemp -d "${TMPDIR:-/tmp}/nv-barraatv-data.XXXXXX"); export NUVIO_DADOS
work=$(mktemp -d "${TMPDIR:-/tmp}/nv-barraatv-build.XXXXXX")
trap 'rm -rf "$NUVIO_DADOS" "$work"' EXIT
sources=()
for source in src/*.c src/dts/*.c; do
  case "$source" in src/main.c|src/menu.c) continue;; esac
  sources+=("$source")
done
cc -O0 -g -DNV_TOUCH_UI -DNV_TESTE "${sources[@]}" tests/barraatv.c -Isrc \
  -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 -L/opt/homebrew/lib \
  -lSDL2 -lSDL2_image -lSDL2_ttf -lz -framework OpenGL \
  -Wno-deprecated-declarations -Wno-macro-redefined -o "$work/test"
"$work/test" "$saida"
