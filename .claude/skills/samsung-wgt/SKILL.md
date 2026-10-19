---
name: samsung-wgt
description: Gerar e publicar o .wgt do Nuvio para Samsung (WebAssembly/Emscripten, Tizen 5.5+) e a variante experimental do Tizen 4 (feat/tizen4-coop). Use para qualquer build .wgt, release normal da Samsung, build de diagnóstico ou alto-cache.
---

# .wgt (Samsung, WASM)

`src/*.c` compilado com emcc para WebAssembly, dentro de um widget. Vídeo pelo
AVPlay (`src/video_tizen.c`), atrás do canvas. Detalhes e o porquê estão nos
cabeçalhos de `tools/tizen.sh` e `tools/tizen-wgt.sh`. Leia antes de mudar
flags.

## Release normal

```bash
bash tools/tizen.sh
NUVIO_WGT_NOME="NuvioTV-<versao>-tizen" bash tools/tizen-wgt.sh
```

- Sem `NUVIO_WGT_NOME` o pacote sai como `NuvioTV-native.wgt`.
- O pacote sai SEM ASSINATURA de propósito: o certificado de distribuidor só
  vale para os DUIDs gravados nele. Quem instala assina (Apps2Samsung faz isso).
- Vai na mesma release `vX.Y.Z` do `.ipk` da LG (receita completa na memória
  `receita-de-release-nuvio-native-legacy`: tag, notas em inglês com
  `## Fixed`/`## Added`, `repo.json` do Homebrew Channel).
- Conferir credenciais: `unzip -l <wgt> | grep -E '\.txt|collections'` tem de
  dar só `addons-recomendados.txt`, `badges/index.json` e `LICENSE`. A arte vai
  num `index.data`, então confira também os nomes em `filename:` no `index.js`.

## Variantes

- `bash tools/tizen.sh --alto-cache`: texturas cravadas em 300 MB.
- `bash tools/tizen.sh --leve`: diagnóstico A/B, NÃO publicar.
- Build de diagnóstico com envio automático: `NUVIO_DIAG_TOKEN` (ou
  `~/.config/nuvio/diag-token`) + `NUVIO_REC_URL` em
  `../NuvioWeb-0.3.38-beta/local.properties`. Nunca imprimir o token.

## Tizen 4 experimental (branch `feat/tizen4-coop`)

1. Mesclar o master na branch.
2. Alinhar `tools/tizen4-config.xml` `version=` ao `appinfo.json`
   (`tizen.sh --tizen4` aborta se discordar).
3. `NUVIO_TIZEN4_EXP=N bash tools/tizen.sh --tizen4`
4. `NUVIO_SAIDA=build/tizen4 NUVIO_WGT_ESTAGIO=build/wgt-stage-t4
   NUVIO_TIZEN_CONFIG=tools/tizen4-config.xml
   NUVIO_WGT_NOME="NuvioTV-<v>-tizen4-expN" bash tools/tizen-wgt.sh`
5. `bash tools/tizen4-sonda.sh` (sonda ES5 que diz se a TV aguenta, sem rede).
6. Tag `native-tizen4-exp.N`, `gh release create --prerelease --latest=false`.

## Verificar sem TV

Chrome de mesa com o estágio servido SEM COOP/COEP e
`--enable-features=SharedArrayBuffer` (o iframe do YouTube é bloqueado com
COEP). Há emulador Tizen 6.0 (x86) no PC Windows do dono — funcional, não mede
desempenho.
