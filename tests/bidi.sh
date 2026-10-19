#!/bin/bash
# Arabic shaping + RTL ordering for subtitles (src/bidi.c). See tests/bidi.c.
#   SANITIZE=1 for ASan/UBSan.
set -euo pipefail
cd "$(dirname "$0")/.."
work=$(mktemp -d "${TMPDIR:-/tmp}/nuvio-bidi.XXXXXX")
trap 'rm -rf "$work"' EXIT
flags=()
if [ "${SANITIZE:-0}" = 1 ]; then flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=all); fi
cc ${flags[@]+"${flags[@]}"} -Isrc -O1 -g -Wall -Wextra tests/bidi.c src/bidi.c -o "$work/test"
"$work/test"
