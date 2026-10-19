#!/bin/bash
export TMPDIR="${TMPDIR:-/tmp}"
TMPDIR="${TMPDIR%/}"
# OK na Live TV abre o mini guia (TCL sem tecla colorida). Com um argumento,
# grava <arg>-faixa.png e <arg>-guia.png sobre o quadro de fixture.
#
#   bash tests/guiaoverlay_ok.sh [/tmp/x/shot]
set -eu
cd "$(dirname "$0")/.."

NUVIO_DADOS=$(mktemp -d ${TMPDIR:-/tmp}/nuvio-guiaoverlay-ok.XXXXXX)
export NUVIO_DADOS
trap 'rm -rf "$NUVIO_DADOS"' EXIT

python3 - <<'PY'
from pathlib import Path
source = Path("android/app/src/main/java/space/nuvio/nativelegacy/NuvioActivity.kt").read_text()
assert "KeyEvent.KEYCODE_GUIDE, KeyEvent.KEYCODE_MENU -> KeyEvent.KEYCODE_F10" in source
assert "KeyEvent.KEYCODE_DPAD_CENTER, KeyEvent.KEYCODE_NUMPAD_ENTER -> KeyEvent.KEYCODE_ENTER" in source
PY

# Use a checked-in still frame for repeatable captures.
export NUVIO_SHOT_ARTE="${NUVIO_SHOT_ARTE:-tests/amostra.jpg}"
if [ "$#" -gt 0 ]; then mkdir -p "$(dirname "$1")"; fi

sources=()
for source in src/*.c src/dts/*.c; do
  case "$source" in src/main.c|src/guia.c) continue;; esac
  sources+=("$source")
done
# src/guia.c e INCLUIDO pelo teste: desenharCard e a lista de canais sao
# estaticos, e semear por dentro e o unico jeito de fotografar sem addon no ar.
cc "${sources[@]}" tests/guiaoverlay_ok.c -Isrc -o ${TMPDIR:-/tmp}/nuvio-guiaoverlay-ok \
  -O1 -g \
  -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 \
  -L/opt/homebrew/lib -lSDL2 -lSDL2_image -lSDL2_ttf -lz -framework OpenGL \
  -Wall -Wextra -Wno-deprecated-declarations -Wno-macro-redefined
if [ "$#" -eq 0 ]; then set -- "$NUVIO_DADOS/shot"; fi
${TMPDIR:-/tmp}/nuvio-guiaoverlay-ok "$@"
