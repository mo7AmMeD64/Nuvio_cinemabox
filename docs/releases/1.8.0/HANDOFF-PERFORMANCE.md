# Handoff — desempenho do Nuvio 2.0 (ex-1.8)

05/10/2026. Para um agente especializado em desempenho continuar a operação. Leia inteiro antes de mexer.

## Onde trabalhar

| Item | Valor |
|---|---|
| Checkout de integração | `/Users/hrocha/.codex/worktrees/integration-180-glass/nuvio-native-legacy` |
| Branch | `codex/integration-180-glass` (sem push; a release pública é a v1.7.4) |
| HEAD de referência | `ba1132a9` (+ este documento) |
| Repositório principal do dono | `/Users/hrocha/Projetos/Pessoal/LG WEB/nuvio-native-legacy` — **não tocar** (branch `fix/tpk-selo-hdr`, trabalho dele) |

Regras de trabalho, todas combinadas com o dono:
- **Nunca editar o checkout de integração.** Crie sua worktree no SSD externo e faça merge depois:
  `git -C <integração> worktree add -b perf/<nome> /Volumes/ExternalSSD/nv-<nome> <HEAD>`
- Disco interno tem pouco espaço: `export TMPDIR=/Volumes/ExternalSSD/tmp` e toda saída no SSD.
- Builds de pacote precisam de `export NUVIO_PROPERTIES="/Users/hrocha/Projetos/Pessoal/LG WEB/NuvioWeb-0.3.38-beta/local.properties"`
  (sem isso o pacote sai sem login/addons/log).
- Só testes focados ("menos teste"). Compilação inteira no Mac tem de ficar limpa:
  `cc src/*.c -o /Volumes/ExternalSSD/<nome>-bin -O1 -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 -L/opt/homebrew/lib -lSDL2 -lSDL2_image -lSDL2_ttf -lz -framework OpenGL -Wno-deprecated-declarations`
- Checagem LG (só build): `bash tools/arm.sh --build`. Deploy na TV **só com pedido do dono**.
- **Nunca reduzir qualidade** (resolução, HDR, efeitos, animações) como "otimização". Só tirar desperdício provado.
  A captura de tela tem de sair igual (diferença de poucos níveis por pixel).
- Nunca afirmar causa sem prova nem FPS de TV que não mediu. Fixture do Mac usa Inter; a TV usa **Montserrat**.
- Commits em inglês terminando com `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.
- Quem junta na integração é a sessão "Nuvio 1.8 integration handoff". Avise antes de juntar ou abrir frente que se sobreponha.

## Aparelhos (somente leitura, o dono usa)

| Aparelho | Acesso | O que ler |
|---|---|---|
| LG OLED C9 (webOS, Mali-G71, alvo mais fraco de GPU) | `sshpass -p alpine ssh -o StrictHostKeyChecking=no -o ProxyCommand=none root@192.168.1.32` | `/tmp/nuvio.log`: linhas `FPS=`, `[gpu-modos]`, `[perf]`, `[cor]`, `[tex-trace]` |
| TCL Android TV (Android 14, armeabi-v7a, Mali-G52) | `adb -s 192.168.1.128:5555 logcat -d -t 20000` | mesmas linhas no logcat |
| Samsung (WGT wasm / TPK nativo) | sem aparelho na bancada | só logs dos usuários (D1) |

Nunca mande teclas, não instale e não crie arquivos-gatilho de captura na TV sem o dono pedir
(um gatilho de captura que o app não consegue apagar derrubou a LG para 9 fps uma vez).

Builds instaladas agora: **LG = `ba1132a9`** (alto-cache), **Android = `78ff220a`**.
Receita de build com etiqueta de prévia: worktree destacada do HEAD, troca `1.7.4 → 1.8.0` em
`deploy/app/appinfo.json` e `tools/tizen-config.xml`, commit, `tools/release-android.sh` / `tools/arm.sh --alto-cache`.

## Ferramentas de medida

- `tests/fluidez_perf.sh` — conta **telas pintadas por quadro**, camadas de tela cheia com blend, chamadas de desenho, por cena (Home nos 3 layouts, detalhe, página do título, Ajustes, Agenda, Biblioteca, painel de Salvos, menu, cenário `c9` = config do dono). Número repetível.
- Linha `FPS=` (a cada 3 s): `pior=`, `janks=`, `gpu-cache=`, `tela=` (conjunto quente), `fila-tex=`, `tex-despejos=N(q=M)`, `disco-direto=`, `neg-arte=`, `cache-arte=… hit/miss`, `rss=`.
- `[gpu-modos] fill:` — telas por modo de shader. Atenção: o assado 320×180 da luz conta como tela cheia nessa linha (superconta).
- `[perf]` — tempos de `cat_definir_tudo`, publicação, snapshot e gravação do cache do catálogo; esperas de trava acima de 8 ms.
- Samsung: `[navegador] … quem=<atribuição>@<fase>` (fase = arranque/catálogo/player/home) nos long tasks.
- Linha do tempo `[t]` do arranque (montagem, CW pronto, manifestos, primeiro catálogo, publicação).
- Harnesses: `tests/trakt_arranque.sh` (latência simulada), `tests/catpub_bench.c`, `tests/logofila.sh`, `tests/texb3.sh`, `tests/fundo_assado.sh` (`gles` usa ANGLE).

## Dados reais dos usuários

`/Volumes/ExternalSSD/nv-auditoria/LOGS-ANALISE.md` (D1 `nuvio-recomendacoes`, tabela `registro`, 22/09–05/10,
1.163 sessões, 364 pessoas; só SELECT, nunca escrever). Matriz de issues: `ISSUES-MATRIZ.md` na mesma pasta.
Principais achados de desempenho:
- Hero sem arte no prazo ("ESTOUROU"): LG 96 pessoas, WGT 63, TPK 52, Android 21.
- Espera na fila de rede da arte maior que o download (LG p50/p90/p99 0,6/2,4/10 s).
- FPS: 76 pessoas na LG com mediana < 45 fps (mais webOS 4); WGT é o pior.
- Long tasks no WGT: > 2 s para 50 pessoas, > 5 s para 19, até 22 s.
- Sessões que morrem sem despedida: TPK 30, WGT 25, LG 17, Android 5.

## Já feito nesta operação (na integração, provado só no Mac salvo indicação)

| Frente | O que mudou | Medida |
|---|---|---|
| B1 desenho | sombra de painel sólido desenhada só nas bordas (`gfx_sombra_vazada`) | Ajustes 3,62 → 2,92 telas/quadro; Agenda 1,48 → 1,13 |
| B2 arranque | fichas de título em disco (`fichameta.c`, TTL 6 h); manifesto lento não segura o ciclo (6 s); feed social em paralelo | Trakt no arranque 8,9 → 5,1 s frio, 0,7 s quente (Mac, latência simulada) |
| B3 texturas | arte em disco vai direto ao decode; 404 de arte lembrado 24 h; upload > 2 MB em faixas dentro de 4 ms/quadro; logo/hero sobem antes | na C9 já aparece `disco-direto=`; antes: 150 s somados de fila para arte em disco, hero de 8,3 MB em 14–58 ms |
| B4 Samsung | cache do catálogo compacto (26,9 → 1,9 MB a 1600 títulos), codificação fora da trava do FS; marcadores `[perf]`/fase | não medido na TV |
| Logos Android | logos furam a fila de rede e de decode; logo do card pedido no foco | simulação: espera 1,2–4,5 s → 85 ms |
| Cache negativo | endpoints que falham (introdb, tiffara, cinemeta, tmdb) e Trakt 401 param de ser repetidos | — |
| Frost/Arte borrada | assados em 320×180 + 1 quad opaco (1bcd6ebe) | página do título 6,3 → 2,3 telas/quadro |

Medida na C9 depois do deploy de `ba1132a9` (dono navegando): FPS 49,5 estável, pior quadro ~30 ms, 0 janks,
gpu-cache 293 MB de 300, `tex-despejos=0`.

## Em aberto (prioridade)

1. **Frost e Arte borrada errados na LG** (regressão do assado). Na C9 a autoconferência passa
   (`[cor] fundo assado conferido`), mas na tela a Arte borrada fica só escura (sem a arte) e o Frost escuro demais.
   Agente rodando em `fix/frost-ambiente-lg` (`/Volumes/ExternalSSD/nv-frostamb`): reaproveitar o caminho da luz
   ambiente da Home Dinâmica (que é leve e certo na C9) e um dump único do fundo em `/tmp/nuvio-fundo-<tipo>.bmp`.
   Coordene antes de mexer em `src/gfx.c` / `src/fundo.c`.
2. **Carrossel do detalhe na C9**: a TV mede 7,4 telas/quadro com 7 camadas de tela cheia (3,3 de cor chapada,
   2,5 de arte do carrossel); o harness do Mac só reproduz 3,0. Achar o que existe na TV e falta no fixture.
3. **Long tasks do Samsung WGT** (2–22 s): suspeito principal (não provado) era gravação do catálogo + `FS.syncfs`;
   já reduzido 14×. Falta log real com `quem=…@fase` para confirmar.
4. **Android: travamento de upload de 16 s** com o low-memory killer do sistema ativo — falta reagir a
   `onTrimMemory` (liberar cache de textura / pausar pré-busca).
5. **Arte pequena decodificada grande**: avatares, logos de addon e de estúdio decodificam até 640 px e são
   desenhados a 40–200 px (~1 MB cada a economizar). Decodificar no tamanho desenhado × escala, nunca abaixo.
6. **Samsung `ADD_FIOS` = 2** (12 nas outras): consultas a addons enfileiram atrás de 2 fios. Só mudar com teste na Samsung.
7. **Marcador "primeira fileira desenhada"** no arranque: sem ele não se mede publicar → pintar no campo.
8. LG webOS 4 com mediana < 45 fps: perfil de GPU e camadas por cena nessas TVs.

## Como entregar

Worktree própria, testes focados, compilação limpa, `arm.sh --build`, capturas antes/depois com Montserrat
(no SSD), números antes/depois na mesma cena. Relatório curto: causa medida, correção, números, o que precisa da TV.
