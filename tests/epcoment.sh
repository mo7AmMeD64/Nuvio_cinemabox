#!/bin/bash
# Real detail/menu UI, deterministic Trakt responses; no network.
set -euo pipefail
cd "$(dirname "$0")/.."
export NUVIO_DADOS
NUVIO_DADOS=$(mktemp -d "${TMPDIR:-/tmp}/nuvio-epcoment.XXXXXX")
trap 'rm -rf "$NUVIO_DADOS"' EXIT
sources=()
for source in src/*.c src/dts/*.c; do
  case "$source" in src/main.c|src/detail.c|src/extras.c) continue;; esac
  sources+=("$source")
done
flags=(-O1 -g -Isrc -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 -Wno-deprecated-declarations -Wno-macro-redefined)
cc "${flags[@]}" -Drede_baixar_com=epcoment_baixar -Dtrakt_cabecalhos=epcoment_cabecalhos -c src/extras.c -o "$NUVIO_DADOS/extras.o"
cc "${flags[@]}" -DNV_TEST_EPCOMENT "${sources[@]}" "$NUVIO_DADOS/extras.o" tests/menuep.c -o "$NUVIO_DADOS/teste" \
  -L/opt/homebrew/lib -lSDL2 -lSDL2_image -lSDL2_ttf -lz -framework OpenGL
mkdir -p /Volumes/ExternalSSD/tmp/rev/epcoment
"$NUVIO_DADOS/teste" /Volumes/ExternalSSD/tmp/rev/epcoment/panel
