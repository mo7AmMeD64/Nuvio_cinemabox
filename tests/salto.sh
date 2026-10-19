#!/bin/bash
set -eu
cd "$(dirname "$0")/.."
out="${TMPDIR:-/tmp}/nuvio-salto-test"
cc tests/salto.c -o "$out" -O1 -g -lm && "$out"
