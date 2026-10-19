#!/bin/bash
# Every spot the release bump touches says the same version (default 2.0.7).
#   bash tests/versao_207.sh [versao]
set -u
cd "$(dirname "$0")/.."
v=${1:-2.0.7}; n=$(echo "$v" | tr -d .)
rc=0
chk() { grep -q -- "$2" "$1" && echo "PASS: $1" || { echo "FAIL: $1 lacks $2"; rc=1; }; }
chk deploy/app/appinfo.json "\"version\": \"$v\""
chk tools/tizen-config.xml "version=\"$v\""
chk src/novidades/$n.inc "\.versao = \"$v\""
chk src/novidades_cartao.c "novidades/$n.inc"
exit $rc
