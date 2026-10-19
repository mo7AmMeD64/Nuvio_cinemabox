#!/bin/bash
# Publica o site da TV VIDAA (worker nuvio-tv, servidor/tv) e confere no ar.
#
# So este script publica o nuvio-tv. O wrangler troca os assets inteiros a cada
# deploy: publicar com build/vidaa-site vazio ou pela metade tira a pagina da TV
# do ar, que foi o que derrubou /tv/ duas vezes (issue #135). Por isso a
# conferencia vem ANTES do deploy, e o curl depois.
#
# Antes: tools/tizen.sh --vidaa, tools/tizen.sh --vidaa --um-fio, tools/vidaa-site.sh
set -e
cd "$(dirname "$0")/.."

SITE="build/vidaa-site/tv"
URL="${NUVIO_TV_URL:-https://nuvio-tv.henriquef29.workers.dev}"

V=$(cat "$SITE/versao.txt" 2>/dev/null || true)
if [ -z "$V" ]; then
  echo "vidaa-publicar.sh: $SITE/versao.txt ausente — rode tools/vidaa-site.sh" >&2
  exit 1
fi
for modo in mt st; do
  for f in index.html index.js index.wasm index.data fontes.js fontes.data; do
    if [ ! -s "$SITE/$V/$modo/$f" ]; then
      echo "vidaa-publicar.sh: falta $SITE/$V/$modo/$f — site incompleto, nada publicado" >&2
      exit 1
    fi
  done
done

(cd servidor/tv && npx wrangler deploy)

NO_AR=$(curl -fsS "$URL/tv/versao.txt" || true)
if [ "$NO_AR" != "$V" ]; then
  echo "vidaa-publicar.sh: ERRO — $URL/tv/versao.txt devolveu '$NO_AR', esperado '$V'" >&2
  exit 1
fi
for modo in mt st; do
  curl -fsS -o /dev/null "$URL/tv/$V/$modo/index.wasm" || {
    echo "vidaa-publicar.sh: ERRO — $URL/tv/$V/$modo/index.wasm nao responde" >&2
    exit 1
  }
done
echo "vidaa-publicar.sh: $V no ar em $URL/tv/"
