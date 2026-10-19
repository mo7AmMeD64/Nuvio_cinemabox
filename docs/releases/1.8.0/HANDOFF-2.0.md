# Handoff 2.0: tudo o que falta até a release (05/10/2026)

Quem assume: este é o documento ÚNICO do que falta para publicar a 2.0. Termina as tarefas na ordem abaixo. Leia o arquivo inteiro antes de mexer. Nada aqui autoriza publicar: tag, release, push, `avisos.json` e respostas em issue precisam de "pode" do dono.

## Ordem de trabalho

1. **Tarefa 1** — fonte árabe embarcada na Samsung + legenda em Windows-1256 (7 issues de árabe).
2. **Tarefa 2** — consertos de código que cabem na 2.0 (#254, #255, #252) e conferências pendentes.
3. **Tarefa 3** — provas em TV que faltam (Android, LG, Samsung).
4. **Tarefa 4** — release 2.0.0 (pacotes, notas, publicação só com autorização).
5. **Depois da release** — respostas curtas nas issues e pedidos de log.

## Onde está tudo

- Checkout de integração: `/Users/hrocha/.codex/worktrees/integration-180-glass/nuvio-native-legacy`, branch `codex/integration-180-glass`. HEAD no momento deste handoff: `cdc000aa` (ou um commit acima, se a sessão anterior commitar mais). Tudo **local, sem push**.
- `master` = última release publicada (1.7.4). A 2.0 sai da integração.
- Triagem das 44 issues abertas contra a 2.0: [ISSUES-2.0.md](ISSUES-2.0.md). Uma correção a ela: #226/#234 (ícone do launcher no Android) **já estão** na integração (`src/iconeapp.c` + `activity-alias` no `AndroidManifest.xml`); o relatório diz que só estavam em `agente/icones`, e isso está errado.
- Handoffs anteriores (contexto, já incorporados aqui): [HANDOFF-NEXT-AGENT.md](HANDOFF-NEXT-AGENT.md), [HANDOFF-FUNDOS-DESCANSO.md](HANDOFF-FUNDOS-DESCANSO.md).

### Entrou na integração hoje (05/10)

| Commit | O quê |
|---|---|
| `6a13452c` | merge feat/perfil-fundos-descanso (fundos Filmes/Luz/Projetor + tela de descanso) |
| merge selos | feat/selos-xperience (pacotes de selos embutidos) |
| `9ca29519` | `.tpk` 4/5: buffers de erro do curl sem `_Thread_local` (o `tools/tpk.sh` recusava) |
| `09124343` | descanso parado (sem Ken Burns), guia 2.0 com capítulo "Tela de descanso", migração única do fundo de perfil para Filmes |
| `247688b4` | merge PR #251 (legenda do .tpk conta a janela a partir do 1º quadro) |
| `709feaad` | relógio do descanso troca de lugar a cada minuto com fade 1,5 s |
| `d300f927` | folha de fontes: folga entre logo do título e selos; ícone do "Onde assistir" 56 px |
| `359b8000` | detalhe: altura da logo assenta suave; arte volta ao topo 2x mais rápido |
| `cdc000aa` | busca/Spotlight: esquece o termo pedido quando os alvos são refeitos (só achava pessoas depois de mudar addons) |

### NÃO entra na 2.0

- **PR #259 (DTS no webOS)**: testado na C9 na branch `test/dts-259` (`/Volumes/ExternalSSD/nuvio-dts-259`). A C9 toca DTS sozinha (o caminho do PR nem liga). Nas duas vezes em que o PR entrou (Top Gun, AIOStreams 2160p), falhou: `Selected DTS track or video absent` (provável TrueHD, não DTS) e `Container open: Input/output error` (o leitor de range dele não abriu o link). Fica para depois; o dono decide se comenta no PR. O app "Nuvio Legacy DTS Debug" (`space.nuvio.native.legacy.dtsdebug`) está instalado na C9 ao lado do normal.
- Canários do host `.tpk` (`agente/i195*`, `agente/i203`, `canary/*`, `canario/*`), `webos3`, `feat/vidaa`, `feat/tizen4-coop`, `perf/trim-fileira` (duplicata do `tex_pressao_memoria`).

---

## Tarefa 1 — fonte árabe embarcada (Samsung) e legenda em Windows-1256

### Por que

São 7 issues de árabe (#239 #245 #247 #250 #253 #258 #261). A forma das letras e a ordem RTL já entraram (`src/bidi.c`, merge do 6bb186ac), mas **a Samsung não tem fonte árabe**: as letras saem como quadrados (#253, #258). Na LG e no Android funciona porque o sistema tem fonte.

### O que o código já faz

- `src/bidi.c` faz a junção do árabe convertendo para **Presentation Forms-B** (U+FE70–FEFF) e algumas de Forms-A (U+FB50–FDFF, ex.: U+FB56..FB7D para پ چ). Não há HarfBuzz. Ou seja: a fonte embarcada precisa ter **esses blocos de formas de apresentação**, não só o bloco base U+0600–06FF.
- `src/text.c` ~1120–1160: tabela de reserva por escrita. `ESC_ARABE` só lista fontes de sistema (LG `/usr/share/fonts/DroidNaskh-Regular.ttf`, `LG_Display_Urdu.ttf`, macOS, Android `/system/fonts/Noto*Arabic*`). As escritas CJK terminam com `cjkEmbarcada` (`<base>fonts/DroidSansFallback-Subset.ttf`), e **é esse o modelo a copiar**: acrescentar `arabeEmbarcada` no fim da lista de `ESC_ARABE`.
- `NUVIO_SEM_RESERVA_DE_SISTEMA=1` no Mac finge a Samsung WASM (só o que está em `deploy/app/fonts`). Usar para provar sem TV.
- O pacote de fontes vai inteiro para todos os alvos: `tools/tizen.sh` faz `--preload-file deploy/app/fonts@/app/fonts`, `tools/tpk.sh` faz `cp -R deploy/app/fonts`, o webOS e o Android levam `deploy/app/fonts`. Arquivo novo ali entra sozinho.

### Passos

1. Fonte: **Noto Naskh Arabic Regular** (ou Noto Sans Arabic UI), licença OFL. Baixar o TTF oficial (github.com/notofonts/arabic, release). Guardar a licença ao lado, como `DroidSansFallback-LICENSE.txt` e `*-OFL.txt` já fazem.
2. Recorte (pyftsubset do fonttools, como `tools/fonte-cjk.py` faz para o CJK — leia esse script e siga o mesmo jeito):
   `U+0600-06FF, U+0750-077F, U+FB50-FDFF, U+FE70-FEFF, U+0020-007F, U+00A0, U+060C, U+061B, U+061F, U+066B-066C, U+200C-200F`.
   Escrever um `tools/fonte-arabe.py` que regera `deploy/app/fonts/NotoNaskhArabic-Subset.ttf`. Alvo de tamanho: dezenas a poucas centenas de KB. A Samsung WASM preload tudo em memória: não passar de ~300 KB.
3. `src/text.c`: `char arabeEmbarcada[600]; snprintf(..., "%sfonts/NotoNaskhArabic-Subset.ttf", base);` e pôr no fim de `ESC_ARABE` (antes do `NULL`), igual ao `cjkEmbarcada`.
4. `tools/idiomas.py` confere a cobertura de glifos das fontes embarcadas (item 10 do docstring). Não há tabela `ar` (#250 é feature nova, fora da 2.0), mas rode `python3 tools/idiomas.py --cobertura` para não quebrar nada.
5. Prova no Mac: uma legenda árabe e um título árabe com `NUVIO_SEM_RESERVA_DE_SISTEMA=1` (ver `tests/` com "arabe"/"bidi" — `grep -l bidi tests/*.c`). Captura antes/depois: quadrados → letras unidas, da direita para a esquerda.
6. Commit em português ou inglês, no estilo do repo, com `Co-Authored-By`.

### Legenda em Windows-1256 (#247, #261) — fazer junto

`src/legenda.c` ~495–560 só detecta UTF-8/16, 1251 e 1252. Legenda árabe antiga vem em **cp1256** e sai como lixo mesmo com bidi e fonte. Detectar (alta frequência de bytes 0xC1–0xED com poucos ASCII de letra latina) e converter por tabela de 128 posições (0x80–0xFF → Unicode). Teste com um `.srt` em cp1256 (gerar com `iconv -t CP1256`), no mesmo estilo dos testes de legenda existentes.

### Fora da 2.0 (decisão do dono)

- #250 (interface em árabe) e #260 (layout RTL): features grandes.

---

## Tarefa 2 — consertos que cabem na 2.0 e conferências

Fonte: [ISSUES-2.0.md](ISSUES-2.0.md). Só o que dá para fazer sem relator:

- **#254 (Samsung .tpk 6) Home não atualiza ao instalar/remover addon, catálogos "fantasma".** Nenhum commit trata. Reproduzir no Mac: instalar e remover um addon com catálogo e coleções, e ver se as fileiras somem/aparecem sem reiniciar. Relacionado ao conserto de hoje `cdc000aa` (busca não refazia ao mudar addons): olhe os outros pontos que dependem de `desc_alvos_busca_zerar`/lista de addons e guardam estado da lista antiga.
- **#255 (Samsung .tpk 6) Home mostra só 5 coleções.** Não ficou provado que o corte é o da TV; a UI não avisa quando corta. Pelo menos: log claro do corte e, se houver teto, dizer qual.
- **#252 Idioma de áudio padrão para anime.** Faixa sem tag de idioma é ignorada na escolha automática (`src/video.c` ~884). Conserto mínimo: faixa sem idioma não pode perder para nada quando é a única do tipo; opção "só para anime" é feature, fora.
- **#249** está resolvida para séries; o comentário em `src/posplay.c:411` ainda fala em 52% e está velho — corrigir o comentário.
- **#241** zoom do trailer no `.tpk` continua Desligado de fábrica: confirmar com o dono se fica assim (a ajuda avisa tela preta em algumas Samsung).
- **#226/#234 ícone do launcher no Android**: o código existe (troca o `activity-alias` ao sair do app). Na TCL nenhum alias trocou ainda. Pedir ao dono para trocar o ícone em Ajustes e sair pelo Home, e ler `adb logcat | grep app-icon` + `dumpsys package space.nuvio.nativelegacy | grep -A12 enabledComponents`.
- **Addon "Minha TV"** (`*.baby-beamup.club`) do dono não responde manifesto (timeout 12–20 s). É servidor dele; só confirmar que a Home não espera por ele.

---

## Tarefa 3 — provas em TV que faltam

| Onde | O quê | Como |
|---|---|---|
| Android TCL `192.168.1.128:5555` | Detalhe: Notas → topo e "logo cresce do nada" (`359b8000`) | Dono navega; `adb logcat` linhas `FPS=`, `[quadro]`, `[gpu-modos] ... tela=detalhe`. Antes: 37–41 FPS e `fill=4.7–4.9x` na volta. |
| Android TCL | Spotlight acha títulos depois de mudar addons (`cdc000aa`) | Adicionar/remover addon e buscar o mesmo termo. |
| LG C9 `192.168.1.32` | Build das mudanças de hoje (`tools/arm.sh --alto-cache`) | `ssh -o ProxyCommand=none root@192.168.1.32` (senha alpine); log em `/tmp/space.nuvio.native.legacy.log`; conferir `/proc/<pid>/exe`. |
| LG C9 | Descanso: vitrine parada, relógio troca de lugar com fade, sem hora na vitrine | Ajustes › Aparência › Tela de descanso 30 s. |
| Samsung | Nada de hoje foi provado. Prioridade: #233/#254/#255 e árabe | Só o dono/relatores têm Samsung. |

O `.tpk` 6+ gerado mais cedo (`LG WEB/nuvio-tpk-build/build/tpk/Nuvio-1.7.4-NuvioTpk60.tpk`) está **velho** (antes de #251, descanso parado, busca, detalhe). Refazer na release.

---

## Tarefa 4 — release 2.0

**Versão: 2.0.0** (o dono decidiu que 1.8 vira 2.0). Pedir confirmação ao dono antes de publicar qualquer coisa: tag, release no GitHub, push, `avisos.json` e respostas em issue são públicos.

### Regras que já custaram caro

- **NUVIO_PROPERTIES** sempre exportado. Fora de `LG WEB/` o `tools/env.sh` não acha o `local.properties` e o pacote sai **sem chaves** (sem login, sem sync, sem Discord). Aconteceu hoje no APK. Prova: no log do app, `[nuvem] https://api.nuvio.tv`, nunca `[nuvem] SEM CONFIGURACAO`.
  `export NUVIO_PROPERTIES="/Users/hrocha/Projetos/Pessoal/LG WEB/NuvioWeb-0.3.38-beta/local.properties"`
- **ares-package**: o da árvore do NuvioWeb sumiu. Instalado em `/Volumes/ExternalSSD/ares-cli`; `export NUVIO_ARES_PACKAGE=/Volumes/ExternalSSD/ares-cli/node_modules/.bin/ares-package`.
- **Versão em dois arquivos**: `deploy/app/appinfo.json` e `tools/tizen-config.xml` têm de bater (o `env.sh` recusa se divergirem). Os `tizen-tpk/*/tizen-manifest.xml` são carimbados pelo `tools/tpk.sh`.
- **`tools/arm.sh` sem `--build` instala na C9** do dono. Na C9 sempre `--alto-cache`.
- **Credenciais**: conferir cada pacote antes de subir (`ar p <ipk> data.tar.gz | tar tz | grep -E 'art/(trakt|addons|tmdb|mdblist|sessao)\.txt|collections\.json'`, `unzip -l <wgt|tpk|apk> | grep -E '\.txt|collections'`). Tem de dar zero.
- **Modo do binário no .ipk**: 1.7.1 saiu 705 e não abria. Conferir 755.

### Pacotes

1. LG: `bash tools/arm.sh --ipk --build` e `bash tools/arm.sh --high-cache --ipk --build`.
2. Samsung `.wgt`: `bash tools/tizen.sh` (precisa de `~/emsdk`; libass WASM em `~/.cache/nuvio-ass-wasm`).
3. Samsung `.tpk` 6+ e 4/5: skill `samsung-tpk` / `samsung-release`; `bash tools/tpk.sh` numa worktree limpa (ex. `../nuvio-tpk-build`, já existe, irmã do NuvioWeb, acha as chaves sozinha). Docker ligado (imagem `nuvio-tpk-sdk`).
4. Android: `bash tools/release-android.sh` (chave em `~/.nuvio-android/release.env`; recusa sem `NUVIO_PROPERTIES`). O APK de prévia na TCL (`192.168.1.128:5555`) foi feito com `tools/android.sh` + carimbo temporário 1.8.0; para a release, usar o script de release com a versão 2.0.0 de verdade.
5. Notas em inglês em `docs/releases/2.0.0/NOTAS.md` (`## Fixed` / `## Added`, uma bala por item, sem cara de IA; ver memória do What's New). Base: o guia do app (`src/novidades20.c`, capítulos) + o resumo de `ISSUES-2.0.md`.
6. Publicação (só com o dono autorizando): merge na `master` por fast-forward, `git tag v2.0.0`, `gh release create v2.0.0` com todos os anexos e `SHA256SUMS`, baixar de volta e conferir os sha256, `repo.json`/Homebrew (`tools/hb-repo.sh`), `avisos.json`. Responder as issues resolvidas, curto.

### O que ainda não tem prova em TV

- Android: a build de hoje está na TCL. O dono achou o detalhe lento indo das Notas para o topo; a correção (`359b8000`) foi instalada sem confirmação dele ainda.
- LG: build das mudanças de hoje sendo instalada na C9 quando este handoff foi escrito.
- Samsung (.wgt e .tpk): nada de hoje foi provado em Samsung. Os riscos maiores são #233/#254/#255 (catálogos e coleções no .tpk) e a fonte árabe da Tarefa 1.
- #223 (Android 11, tela preta no login) e #211 (LG webOS 4 fecha ao abrir) dependem dos relatores.

---

## Depois da release

- Responder curto (memória do dono: "manda bíblia ninguém lê") nas resolvidas: #223 #231 #233 #235 #237 #238 #239 #243 #244 #245 #249 e nas "resolvida antes" (#188 #197 #202 #209 #227) pedindo para fechar.
- Pedir log na 2.0: #211 (LG webOS 4 fecha ao abrir), #228 (trailer na Q80A), #232 (miniatura não desfocada), #240 (demora Torbox), #246 (seek HEVC anime), #256 (autoplay do próximo), #158 (Live TV C4), #171 (P2P/Torrentio no .tpk).
- PR #259 (DTS): o dono decide se comenta. Achados estão no topo deste arquivo. Remover o app "Nuvio Legacy DTS Debug" da C9 quando ele mandar.
- PR #251 entrou: agradecer o KeijoMika e fechar o PR apontando o commit `247688b4` (o merge foi local, na integração).

### Chaves da conta (medido hoje, só nomes)

`sync_pull_provider_credentials` por perfil: perfil 1 tem só `tmdb` (é token **v4**, o app ignora e usa a chave do pacote); perfil 2 tem só `mdblist`. Debrids, IntroDB e AnimeSkip vazios. Não existem trakt/imdb/omdb/fanart na conta. Nada a fazer para a release; anotar se alguém perguntar.
