#!/bin/bash
# Captura de texto arabe do TMDB pelos caminhos comuns de desenho (ver o .c).
# Nao entra na suite: precisa de janela GL e de olho humano para julgar.
#
#   NUVIO_SEM_RESERVA_DE_SISTEMA=1 bash tests/arabe_texto_shot.sh /tmp/arabe.bmp
set -eu
cd "$(dirname "$0")/.."
NUVIO_DADOS=$(mktemp -d "${TMPDIR:-/tmp}/nuvio-arabe-shot.XXXXXX")
export NUVIO_DADOS
trap 'rm -rf "$NUVIO_DADOS"' EXIT
sources=()
for source in src/*.c src/dts/*.c; do
  if [ "$source" != src/main.c ]; then sources+=("$source"); fi
done
bin="$NUVIO_DADOS/shot"
cc "${sources[@]}" tests/arabe_texto_shot.c -Isrc -o "$bin" \
  -O1 -g -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 \
  -L/opt/homebrew/lib -lSDL2 -lSDL2_image -lSDL2_ttf -lz -framework OpenGL \
  -Wno-deprecated-declarations -Wno-macro-redefined
"$bin" "$@"
