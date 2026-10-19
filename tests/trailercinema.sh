#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
${CC:-cc} -std=c11 -Isrc tests/trailercinema.c -o "$tmp/trailercinema" -lm
"$tmp/trailercinema"
