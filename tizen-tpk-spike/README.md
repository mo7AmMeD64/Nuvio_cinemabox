# tizen-tpk-spike

Spike da Fase 0 (plano `~/.claude/plans/fazer-uma-versao-nativa-parsed-twilight.md`):
descobrir se um `.tpk` .NET consegue rodar `.so` própria, com GL dentro do
processo, em TVs Samsung de anos diferentes, instalado via Apps2Samsung.
Fora de `src/`; nada do app WASM/webOS muda.

## Build

```bash
NUVIO_EXIGIR_ENVIO=1 bash tools/tizen-tpk-spike.sh
```

O token vem de `NUVIO_DIAG_TOKEN` ou, sem ela, de `~/.config/nuvio/diag-token`.

Gera três pacotes em `out/`, um por faixa de TV. O relatório sobe sozinho
para `/v1/registro` no modo "diagnóstico" da build #77 (grava como
`diag:spike-<modelo>`, com um código de 6 letras que também aparece na tela).
Sem `NUVIO_DIAG_TOKEN` o build sai com envio desligado e a tela pede foto;
`NUVIO_EXIGIR_ENVIO=1` transforma isso em erro. O token vai para
`dotnet/Segredos.g.cs` (fora do git). Se o TLS falhar por CA vencida em TV
antiga, o envio tenta de novo sem validar o certificado: não expõe nada novo,
o token já está dentro do `.tpk`.

Todo build também carrega a `.so` num container `arm32v5/debian:buster`
(ARM soft-float, glibc 2.28) e roda `nv_ping`, `nv_thread_test` e o
`spikebin` (`native/teste/carrega.c`); se falhar, o build para. O `.tpk` sai com a
assinatura de desenvolvedor padrão do SDK; o Apps2Samsung reassina por DUID
na instalação (`TizenInstallerService.cs`, `manualResign`).

| Pacote | TFM | api-version | TVs | Testes |
|---|---|---|---|---|
| `NvSpikeLegacy` | `tizen40` (ElmSharp) | 4 | 2018–2020 (Tizen 4–5.5) | 1, 2, 4, 5 |
| `NvSpikeNui60` | `tizen80` (NUI) | 6 | 2021 (Tizen 6.0) | 1, 2, 3b, 4, 5 |
| `NvSpikeNui65` | `tizen90` (NUI) | 6.5 | 2022–2023 (Tizen 6.5–7) | 1, 2, 3b, 4, 5 |
| `NvSpikeNui` | `net6.0-tizen8.0` (NUI) | 8 | 2024+ | 1, 2, 3, 3b, 4, 5 |

Testes (uma linha na tela cada, OK/FALHOU + mensagem; usuário manda foto):
1. `DllImport` da `.so` própria (`nv_ping` = 42)
2. pthread dentro da `.so`
3. `GLView` (widget GL na árvore NUI) desenhando pela `.so`
3b. `GLWindow` (janela GLES inteira, o modelo do SDL de hoje) desenhando pela `.so`
4. `Process.Start` do `spikebin` estático (copiado para `data/` + `chmod`)
5. versão do Tizen e modelo

3 e 3b dão veredito objetivo após 3 s: quadros desenhados pela `.so` e
`glGetError`, não "olhe se animou".

## O que o build revelou (2026-09-24)

- **O workload .NET moderno (`net6.0-tizen`) só aceita Tizen ≥ 8.0**
  (`Samsung.Tizen.Sdk.Versions.targets`: plataformas 8.0–11.0). TV 2018–2022
  só com o SDK antigo (`Tizen.NET` + `Tizen.NET.Sdk` via NuGet, TFMs `tizenNN`).
- **`GLView` só existe a partir da API11 (Tizen 8).** O plano assumia 6.5.
  `GLWindow` existe desde a API8 (Tizen 6.0) e casa melhor com o Nuvio (janela
  GLES inteira, como o SDL). API7 (Tizen 5.5) e anteriores não têm nenhum.
- **A API do `GLWindow` muda a cada versão** (conferido por reflexão):
  API8 `SetEglConfig(.., GLWindow.GLESVersion.Version_2_0)` + quadro `void` e
  sem `RenderingMode`; API9 `SetEglConfig(.., GLESVersion.Version20)` + quadro
  `int`; API11 `SetGraphicsConfig` + `RegisterGLCallbacks`. Por isso há um
  pacote por API: o binário da API8 numa TV API9 poderia falhar no registro
  do callback sem que a Samsung tivesse bloqueado nada. Um port de verdade
  teria de resolver isso em tempo de execução (ou por reflexão).
- **glibc**: linkando contra o rootstrap do Tizen 10 (glibc 2.39) a `.so`
  exigia `pthread_create@GLIBC_2.34` e não carregaria em TV com glibc
  anterior. O build compila com headers do 10 e linka contra o 9 (glibc 2.30);
  o script falha se aparecer `GLIBC_2.34+`. Verificado em container
  `arm32v5/debian:buster-slim` (ARM soft-float, glibc 2.28): `dlopen` OK,
  `nv_ping=42`, `nv_thread_test=1`, `spikebin` roda.

## Preparar o Mac (Apple Silicon)

1. Tizen Studio: `NativeCLI`, `NativeToolchain-Gcc-14.2`,
   `TIZEN-9.0-NativeAppDevelopment-CLI`, `TIZEN-10.0-NativeAppDevelopment-CLI`
   (`package-manager-cli.bin install ... --accept-license`, um por vez; em lote
   morreu com exit 137).
2. O `gcc-14.2` é binário x86_64 (roda por Rosetta) e procura `libisl.23`,
   `libintl.8` e `libzstd.1` em `/usr/local/opt/<pkg>/lib`. Homebrew não
   instala mais cópia Intel lado a lado, e o CLT desta máquina não tem fatia
   x86_64 do `libxcrun` (não dá para compilá-las). Solução: baixar as bottles
   x86_64 (`sonoma`) direto do GHCR (`ghcr.io/v2/homebrew/core/<pkg>`, token
   anônimo), copiar a `.dylib`, `install_name_tool -id/-change` para o caminho
   absoluto e `codesign --force --sign -`. `isl` 0.28, `gmp` 6.3.0 (dependência
   do isl), `gettext` 1.0, `zstd` 1.5.7. `/usr/local/opt` precisou de
   `sudo chown` uma vez.
3. `gcc-9.2` fica sem uso: pede `libisl.19` e o GHCR só tem isl ≥ 0.23.
4. .NET: `dotnet-install.sh --channel 8.0 --install-dir ~/.dotnet` e o
   workload da Samsung (`Samsung/Tizen.NET/workload/scripts/workload-install.sh`).

## Falta

- **TV real** (plano, passo 3): pré-lançamento pedindo TVs 2018, 2019/20,
  2021/22 e 2023+; a tabela ano × teste decide entre port .NET e NaCl.
  TV 2023 (Tizen 7) fica com o pacote 6.5 (`net6.0-tizen` exige 8.0).
- Emulador no winpc: não feito. É x86 (a `.so` ARM falharia lá de qualquer
  jeito) e o Tizen 10 recusou pacote por cadeia de certificado. Nenhuma das
  três telas foi vista rodando: a UI .NET só vai ser exercitada na TV.

## Spike de UEP (Tizen 4/5) — `NvUepProbe`

    bash tools/tizen-uep-spike.sh   # tizen-tpk-spike/out/NvUepProbe-0.1.0.tpk

Pacote/appid proprio (`NuvioTV002.NvUepProbe`); nao encosta nos publicados.

**Pergunta:** o `NvSpikeLegacy` ja provou que na Tizen 4/5 FALHAM o DllImport da
`.so` propria (dlopen de arquivo) e o `Process.Start` de binario estatico — a
UEP (Unauthorized Execution Prevention) recusa `mmap PROT_EXEC` de arquivo nao
assinado por autor confiavel ("failed to map segment from shared object"). Ha
como carregar codigo C proprio de outra forma?

**Hipotese (evidencia externa):** a pesquisa de seguranca de TV Samsung da
califio (MADBugs/samsung-tv, TV KantS2/Linux 4.1.10/ARMv7 — a mesma geracao
2018-2019 das TVs que falharam) mostra que a UEP bloqueia execucao de ARQUIVO,
mas nao de MEMORIA ANONIMA: o exploit roda binario nao assinado por um wrapper
`memfd` ("loaded a program into an anonymous in-memory file descriptor and
executed it from memory instead of from a normal file path"). O proprio JIT do
.NET ja depende de memoria anonima executavel, entao ela tem de ser permitida.

`Program.cs` testa, em ordem, mostrando cada resultado na tela e em
`data/tpk-host.log`:

| # | Teste | Esperado |
|---|---|---|
| A | `dlopen(lib/libnvprobe.so)` | FALHA (confirma UEP ativa) |
| B | `dlopen(data/` copia`)` | FALHA (copia fora da base de assinaturas) |
| C | `memfd_create` + `dlopen("/proc/self/fd/N")` | **se OK, a rota existe** |
| D | `mmap` anon RW + `mprotect(+PROT_EXEC)` | OK = carregador proprio viavel |

A `.so` de teste (`native/uepprobe.c`) NAO tem segredo: so `nv_probe()==42` e um
teste de pthread. Compilada no mesmo container ARM do `tools/tpk.sh`
(`arm32v5/debian:buster`, glibc 2.28, softfp NEON), carrega nas TVs alvo.

Se **C** der OK, a `libnuvio.so` real passa a ser carregada assim (ship como
recurso, nunca em `lib/`, aberta por memfd) e o host GL do `NuvioTpk40` adota a
mesma carga. Se **C** falhar mas **D** passar, a saida e um carregador de ELF
proprio em memoria anonima (mmap + relocacoes + `mprotect RX`). Se as duas
falharem, a rota nativa em .tpk esta fechada para 4/5 (resta o NaCl em `.wgt`,
que outro agente estuda).

## Spike 2 de UEP (Tizen 4/5) — `NvMemfd`

    bash tools/tizen-memfd-spike.sh   # tizen-tpk-spike/out/NvMemfd-0.1.0.tpk

Pacote/appid proprio (`NuvioTV002.NvMemfd`); nao encosta nos publicados.

**O que o `NvUepProbe` rodou numa TV real (optiman, QE55Q6FNA, Tizen 4.0):**
A `dlopen lib/` FALHOU, B `dlopen data/` FALHOU ("failed to map segment from
shared object"), C `memfd` deu **EXCECAO `EntryPointNotFoundException:
'memfd_create'`** (a libc do Tizen 4/5 nao exporta esse simbolo), D
`mprotect +EXEC` **OK**. Ou seja: `mmap PROT_EXEC` de ARQUIVO e barrado, mas
memoria ANONIMA executavel e liberada — e a linha C so falhou por chamar
`memfd_create` por NOME.

`NvMemfd` corrige e amplia, cada teste em `try/catch` (uma falha nao impede a
proxima), tela + `data/tpk-host.log`:

| # | Teste | Esperado |
|---|---|---|
| 1 | `syscall(385)` (memfd_create por NUMERO, ARM EABI) + `write` + `dlopen("/proc/self/fd/N")` | **se OK, a rota preferida existe** |
| 2 | Carregador de ELF32 ARM proprio em C# (rota D): `mmap` anon RW, mapeia PT_LOAD, aplica `R_ARM_RELATIVE/GLOB_DAT/JUMP_SLOT/ABS32`, resolve externos por `dlsym(RTLD_DEFAULT)`, roda `DT_INIT_ARRAY`, `mprotect +EXEC`, chama `nv_probe` pelo dynsym | OK = plano B sem arquivo nenhum |
| 3 | Diz na tela QUAL metodo carregou codigo nativo | — |

A `libnvprobe.so` e a mesma `native/uepprobe.c` do spike 1, mas linkada com
`--hash-style=both` para o `DT_HASH` existir (o carregador em C# usa `nchain`
para contar simbolos). Se **1** passar (`nv_probe=42`), o proximo passo e
carregar a `libnuvio.so` real por memfd no host `NuvioTpk40`.
