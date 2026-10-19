#!/bin/bash
# #5 da operacao 1.8 (05/10/2026): arte pequena decodificada grande — ver
# tests/artemenor.c. A copia de amostra.jpg existe porque o teste precisa da
# MESMA arte nos DOIS tetos (640 antigo e 128 novo) ao mesmo tempo: no cache o
# caminho e a chave.
set -eu
cd "$(dirname "$0")/.."
copia="$(mktemp "${TMPDIR:-/tmp}/artemenor640.XXXXXX")"
cp tests/amostra.jpg "$copia"
trap 'rm -f "$copia"' EXIT
sources=()
for source in src/*.c src/dts/*.c; do
  case "$source" in src/main.c|src/tex_cache.c) continue;; esac
  sources+=("$source")
done
out="${TMPDIR:-/tmp}/nuvio-artemenor"
cc "${sources[@]}" tests/artemenor.c -Isrc -o "$out" \
  -O1 -g -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 \
  -L/opt/homebrew/lib -lSDL2 -lSDL2_image -lSDL2_ttf -lz -framework OpenGL \
  -Wno-deprecated-declarations -Wno-macro-redefined
"$out" "$copia"
