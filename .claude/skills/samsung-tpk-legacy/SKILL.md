---
name: samsung-tpk-legacy
description: O .tpk nativo do Nuvio para Samsung Tizen 4.0-5.5 (TVs 2018-2020), pacote NuvioTpk40 sobre a TVGLApplication, com a lib carregada por memfd (ou carregador ELF proprio) porque a UEP barra .so de arquivo. Use ao mexer em tizen-tpk/NuvioTpk40, em src/rede.c sob NV_TPK40, ao ler relatos de TVs 2018-2020, ou em erro de instalacao 118014.
---

# .tpk Tizen 4.0–5.5 (TVGLApplication)

Provado em 29/09/2026: 2x Tizen 4.0 (optiman, rawldon UA40N5300) e 2x 5.0
(singhsamarveer UE55RU7170, KeijoMika QE65Q80R) — login, home, filme 4K,
trailer. Entra no release normal desde a 1.5.4.

## Como carrega (`tizen-tpk/NuvioTpk40/Program40.cs`)

- **UEP**: a TV recusa `dlopen` de `.so` em arquivo nao assinado (com Public
  OU Partner), mas permite memoria anonima executavel.
- `CarregaNativo`: 1) `memfd_create` por **syscall 385** (a libc do 4/5 nao
  exporta o simbolo) + `dlopen("/proc/self/fd/N")`; 2) se falhar, carregador
  ELF32-ARM proprio em C# (PT_LOAD, relocacoes RELATIVE/GLOB_DAT/JUMP_SLOT/
  ABS32, `DT_INIT_ARRAY`, `mprotect`, `cacheflush`). Na TV do sigmaboy19 o
  memfd falhou e o ELF carregou — os dois caminhos sao reais.
- O carregador ELF RECUSA (com mensagem na tela) o que nao sabe: PT_TLS,
  relocacao desconhecida, sem DT_HASH, simbolo nao resolvido.

## A lib propria do 4/5 (`libnuvio-tpk40.so`, `-DNV_TPK40`)

- **Sem TLS**: os `_Thread_local` de `src/rede.c` viram campos da struct
  `RedeFio` (pthread_key). Era o crash de sign-in (#180): o carregador ELF
  ignorava R_ARM_TLS_* e o primeiro HTTPS morria.
- **Com DT_HASH** (`--hash-style=both`).
- `tools/tpk.sh` passo [1b/3] recompila so as unidades que citam `NV_TPK40` e
  FALHA se sobrar TLS ou faltar DT_HASH (`tests/tpk40_tls.sh`).
- Anexo de auto-atualizacao proprio: `libnuvio-<v>-tpk40-arm.so`
  (`src/atualizacao.c` procura `-tpk40-arm.so` sob `NV_TPK40`). Nunca deixe o
  4/5 baixar a lib comum.
- `dt-init`: enderecos em `uint` (32 bits); 0 e 0xFFFFFFFF sao sentinelas;
  cada init no seu try (antes, OverflowException abortava as inits).

## Assinatura: PUBLIC, sem `drminfo`

O manifesto NAO declara `http://developer.samsung.com/privilege/drminfo`. Com
ele o Apps2Samsung assinava Partner e parte das TVs recusava a instalacao com
**`install failed[118014]`** (rawldon UA40N5300). Sem ele instala e abre.
`tools/release-samsung.sh` recusa pacote 4/5 com drminfo.

## Diagnostico

- Rastro `data/tpk-etapas.txt` (host + nativo: `read-so`, `memfd`,
  `elf-loader`, `dt-init`, `libcurl-dlopen` (o 4/5 tem `libcurl.so.4`),
  `https-1`, `sign-in-thread`). Etapa aberta na abertura seguinte vira
  "Previous launch stopped at: ..." na tela. Sem handler de SIGSEGV: o CoreCLR
  ja e dono desse sinal.
- Desempenho: GPU Mali-TDVX (1 GB) presa no preenchimento; o nivel de GPU
  adaptativo (`src/gpunivel.c`) desce para efeitos leves. Posteres cinza ao
  rolar rapido = decode atrasado (orcamento de textura 64 MB no `.tpk`).

## Tizen 5.5

API7 sem GLWindow: vai neste pacote (api-version 4 roda no 5.5).
