#!/bin/bash
# Ponte do IME do .wgt (tools/tizen-shell.html <-> src/entrada_texto.c).
set -eu
cd "$(dirname "$0")/.."
EMSDK_DIR="${EMSDK_DIR:-$HOME/emsdk}"
source "$EMSDK_DIR/emsdk_env.sh" >/dev/null 2>&1
export EMCC="${EMCC:-$(command -v emcc)}"
node tests/entrada_texto_wgt.cjs
