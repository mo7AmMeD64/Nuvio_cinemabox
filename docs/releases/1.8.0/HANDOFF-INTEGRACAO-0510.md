# Handoff — integração Nuvio 2.0 (ex-1.8), 05/10/2026 ~16h40 (atualizado)

Quem lê: o agente que assume a coordenação da integração. Leia inteiro antes de juntar qualquer coisa.
Para desempenho há um documento à parte: [HANDOFF-PERFORMANCE.md](HANDOFF-PERFORMANCE.md).

## Onde

| Item | Valor |
|---|---|
| Checkout de integração | `/Users/hrocha/.codex/worktrees/integration-180-glass/nuvio-native-legacy` |
| Branch | `codex/integration-180-glass` — **sem push** (release pública = v1.7.4) |
| HEAD | `13848088` (+ este documento). **Duas sessões juntam neste mesmo checkout** — `git log -1` antes de cada merge |
| Repo do dono | `/Users/hrocha/Projetos/Pessoal/LG WEB/nuvio-native-legacy` (branch `fix/tpk-selo-hdr`) — **não tocar** |
| Mockups 2.0 | `/Volumes/ExternalSSD/nv-analise/mockups-20/` |
| Auditoria issues/logs | `/Volumes/ExternalSSD/nv-auditoria/` (`ISSUES-MATRIZ.md`, `LOGS-ANALISE.md`) |

## Como trabalhar (combinado com o dono)

- O coordenador **não programa no checkout de integração**: subagentes trabalham em worktree própria no SSD
  (`git -C <integração> worktree add -b <branch> /Volumes/ExternalSSD/nv-<nome> HEAD`), o coordenador faz merge `--no-ff`.
- Sonnet por padrão; Opus só para causa difícil, estado/concorrência ou redesenho grande.
- `export TMPDIR=/Volumes/ExternalSSD/tmp` (disco interno quase cheio).
- Depois de cada merge: testes focados do que mudou ("menos teste, mais rápido") + compilação inteira no Mac limpa:
  `cc src/*.c -o /Volumes/ExternalSSD/<x>-bin -O1 -I/opt/homebrew/include -I/opt/homebrew/include/SDL2 -L/opt/homebrew/lib -lSDL2 -lSDL2_image -lSDL2_ttf -lz -framework OpenGL -Wno-deprecated-declarations`
- Merges de Ajustes conflitam no fim do enum `AJ_*` (ajustes.c, ajustes_ux_padrao.inc, ajustes_ux_dados.inc): concatenar os blocos na ordem do merge e atualizar a assert da última opção em `tests/ajustes_ux_dados.c`. Opção nova precisa de ícone em `ajustes_ux_visual.inc` e i18n em todos `src/idioma_*.h`.
- **Instalar nas TVs só quando o dono pedir.** Não mandar teclas enquanto ele usa. Julgar visual por captura real da TV (Montserrat), não pelo fixture do Mac (Inter).
- Backend (D1 `nuvio-recomendacoes`, worker) **só com ok explícito do dono**, passo a passo.
- Nunca postar respostas de issue sem aprovação. Nunca imprimir/guardar token ou credencial.
- Commits em inglês terminando com `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.

## Build e deploy (quando o dono pedir)

1. `git -C <integração> worktree add --detach /Volumes/ExternalSSD/nuvio-180-apk-<sha> HEAD`
2. Trocar `1.7.4 → 1.8.0` em `deploy/app/appinfo.json` e `tools/tizen-config.xml`, commit.
3. `export NUVIO_PROPERTIES="/Users/hrocha/Projetos/Pessoal/LG WEB/NuvioWeb-0.3.38-beta/local.properties"`; `bash tools/env.sh --require-core`.
4. Android: `bash tools/release-android.sh` → `build/release-1.8.0/Nuvio-1.8.0-android.apk`; conferir `/v1/registro` no libmain.so;
   `adb -s 192.168.1.128:5555 install -r <apk>` (fora do sandbox); puxar o APK instalado e comparar sha256.
5. LG C9: `NUVIO_SSH_OPTS="-o ProxyCommand=none -o ConnectTimeout=10" bash tools/arm.sh --alto-cache` (sempre alto-cache; confere md5 e relança).
   Leitura: `sshpass -p alpine ssh -o StrictHostKeyChecking=no -o ProxyCommand=none root@192.168.1.32`, log em `/tmp/nuvio.log`.

**Nas TVs agora (05/10 ~16h30):** Android **e** LG = `13848088` (build `/Volumes/ExternalSSD/nuvio-180-apk-13848088`, apk sha256 `b8546d20…`, LG md5 `cf68c2f5…`).
Se o adb disser `No route to host` com a TV ligada: `adb kill-server && adb connect 192.168.1.128:5555`.

## Feito hoje (05/10)

- `29aad584` handoff de desempenho; `443b1f5d` splash neutra #0E0F12 sem logo.
- **D1: migração 009 (enquete) aplicada** (ok do dono). `--file` dá `Authentication error [code: 10000]`; usar `--command "<sql sem comentários>"`.
- **Worker: secret `SIMKL_CLIENT_ID` cadastrado** (valor dado pelo dono; nunca registrar). O worker no ar (de `394f9b7a`) já tinha a rota Simkl; a única diferença do servidor para HEAD é `enquete.js` (deploy pendente).
- Merges: Frost/Arte borrada pela pipeline da luz Imersiva (`83a8e7ad`); Simkl+Letterboxd na aba Amigos (`e0686df6`); cenas de Ajustes por categoria (`0c8a0f9d`) e por bloco/submenu (`5ca65e27`); OLED esmaecer + brilho da UI do player (`c2cd5a15`); legenda em negrito com face Bold real em vez de negrito sintético do SDL_ttf (`13848088`).
- Mockups publicados: What's New 2.0 https://claude.ai/artifact/4BTSfCUDQ1CJ7MrxWm49S7 (aprovado, em implementação) e Explorar https://claude.ai/artifact/TMzk2mta36E4PhFPbbH6GR (esperando escolha).
- Segunda legenda empilhada + estilo próprio **já existia** (`ee4bc8a3`, `AJ_LEG2_*`: "Junto da principal"); dono não tinha achado — buscar "segunda legenda" nos Ajustes.

## Conferir na TV (o dono estava para testar)

- LG `/tmp/nuvio.log`: `[cor] fundo frost|borrada conferido` ou `CONFERIDO ERRADO`; dumps `/tmp/nuvio-fundo-<tipo>.bmp` e `-assado.bmp` (gravados 1x por execução).
- `[leg] negrito: face Bold real de Montserrat, 36 px` ao tocar legenda SRT de addon (estilo do dono: Montserrat 90, negrito, sem contorno).
- Esmaecer (5 min, 60% → 92%), tecla que acorda não age, brilho do OSD 80% em HDR, Magic Remote tremendo acordando a tela.
- Simkl: ligar na aba Amigos com a conta do dono (nunca testado contra a API real).
- A C9 desenha a UI em 1920x1080 e o painel é 3840x2160 (a TV amplia); desenhar em 4K custaria ~4× GPU — não mexido, dono achava que já era 4K.

## Agentes em voo (o novo coordenador NÃO recebe as notificações deles — conferir as worktrees)

| Frente | Worktree / branch | Estado ~16h40 | Ao terminar |
|---|---|---|---|
| What's New 2.0 conforme mockup (abre 1x para todos na 1ª abertura da 2.0, inclusive instalação nova; Social diz "Trakt, Simkl e Letterboxd"; enquetes "Depende do servidor") | `/Volumes/ExternalSSD/nv-wn20`, `feat/whatsnew-20` (Opus) | 2 commits (`fb588471`, `2e6afd35`) + `src/novidades20.c` sujo | Merge, compilação, testes do guia + `ajustes_ux_dados`; mostrar capturas ao dono |

Se o agente sumiu sem terminar (limite de tokens), retomar com agente novo apontando `git -C <wt> log/diff`.

## Esperando o dono

- Escolher a variação do Explorar: https://claude.ai/artifact/TMzk2mta36E4PhFPbbH6GR (recomendação: B + grade C como entrada + botão no Detalhe).
- Deploy do worker da enquete (`npx wrangler@4 deploy --config servidor/recomendacoes/wrangler.toml`) + semear a primeira enquete (`N3-ENQUETE.md`).
- Página de perfil/estatísticas (refazer; mockup v2 foi achado "mal feito", adiado).
- Página de filme renovada/animada para a 2.0 (só mencionada, sem mockup).
- Logo "Clássico renovado" (desenhar).
- Postar respostas das issues (rascunhos em `ISSUES-MATRIZ.md`).
- Tamanho do D1 (4,2 GB).

## Pendências técnicas conhecidas

- Testes pré-existentes quebrados até `88d00591`: `detail_layout` e `home_hero_layout` (já consertados nesse commit; se voltarem, ver stub `desc_pedir_titulo_semente` e 11º argumento `friendsH`).
- Arquivo não rastreado no checkout: `docs/releases/1.8.0/HANDOFF-NEXT-AGENT.md` (de outra sessão; não apagar, não commitar sem saber de quem é).
- Fila de desempenho: ver `HANDOFF-PERFORMANCE.md` (carrossel do detalhe na C9, long tasks Samsung, onTrimMemory Android, arte pequena decodificada grande, etc.).
