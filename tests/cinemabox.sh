#!/bin/bash
export TMPDIR="${TMPDIR:-/tmp}"
TMPDIR="${TMPDIR%/}"
set -eu
cd "$(dirname "$0")/.."
DIR=$(mktemp -d ${TMPDIR:-/tmp}/nuvio-cinemabox.XXXXXX)
trap 'rm -rf "$DIR"' EXIT
cc -O1 -g -Wall -Wextra -fsanitize=address,undefined -Isrc \
  src/cinemabox.c src/js.c tests/cinemabox.c -o "$DIR/teste" -lpthread
"$DIR/teste"
