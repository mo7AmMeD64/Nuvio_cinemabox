#!/bin/bash
set -eu
cd "$(dirname "$0")/.."
work=$(mktemp -d "${TMPDIR:-/tmp}/nuvio-cwfrente.XXXXXX")
trap 'rm -rf "$work"' EXIT
cc tests/cwfrente.c -Isrc -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 \
  -o "$work/test" -O1 -g -Wall -Wno-deprecated-declarations -Wno-macro-redefined
"$work/test"
