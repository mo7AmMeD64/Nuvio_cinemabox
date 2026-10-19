#!/bin/bash
# Spotlight + teclado da TV (entrada_texto.h) pelo caminho SDL, sem TV. Ver o .c.
set -euo pipefail
cd "$(dirname "$0")/.."
tmp="$(mktemp -d "${TMPDIR:-/tmp}/nuvio-spot-tv-XXXXXX")"
trap 'rm -rf "$tmp"' EXIT
NUVIO_DADOS="$tmp/dados"; mkdir -p "$NUVIO_DADOS"; export NUVIO_DADOS
sources=()
for source in src/*.c src/dts/*.c; do
  [ "$source" = "src/main.c" ] && continue
  sources+=("$source")
done
cc "${sources[@]}" tests/spotlight_tv_texto.c -Isrc -o "$tmp/t" -O1 -g -DNV_TEXTO_SDL_TESTE \
  -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 -L/opt/homebrew/lib \
  -lSDL2 -lSDL2_image -lSDL2_ttf -lz -framework OpenGL \
  -Wno-deprecated-declarations -Wno-macro-redefined
"$tmp/t" | grep -E '^(PASS|Assert)|\[texto\]' | tail -20
