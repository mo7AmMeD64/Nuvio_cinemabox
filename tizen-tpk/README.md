# Nuvio .tpk (Samsung Tizen 6+)

> **Acompanhamento:** [Project (quadro)](https://github.com/users/iqui27/projects/2) · [Milestone](https://github.com/iqui27/nuvio-native-legacy/milestone/1) · [issue #137 (testes)](https://github.com/iqui27/nuvio-native-legacy/issues/137). Na aba Issues, filtre `label:samsung-native` (tudo do nativo) e `label:"status: blocked"` (travado).

O mesmo app C do webOS e do `.wgt`, empacotado como app .NET da Samsung.

    bash tools/tpk.sh          # build/tpk/Nuvio-<versao>-NuvioTpk{60,65,}.tpk
    bash tools/tpk-testa.sh    # roda a .so num host falso no container ARM

## Como funciona

- **`libnuvio.so`**: `src/*.c` compilado com `-DNV_TPK`, no container
  `tools/tpk/Dockerfile` (Debian buster armel, glibc 2.28). SDL2, SDL2_image e
  SDL2_ttf entram estaticos, com o video "dummy" do SDL: o SDL so da fila de
  eventos, tempo, fios e superficies.
- **Host .NET** (`Program.cs`): abre um `GLWindow` de tela cheia. A cada quadro
  do NUI chama `nv_tpk_quadro()`, que passa o contexto EGL para o fio do app e
  espera o `SDL_GL_SwapWindow` dele (`src/tpk.c`, `src/tpk.h`). Teclas vao pelo
  nome (`XF86Back`, `Up`, ...).
- **Video**: `Tizen.Multimedia.Player` no host, no plano de video, com o
  `GLWindow` translucido por cima. O C pede e le o estado por `src/video_tpk.c`.

## Pacotes

| Pacote | TFM | Para |
|---|---|---|
| `NuvioTpk40` | tizen40 + TVGLApplication | Tizen 4.0-5.5 (TVs 2018-2020) |
| `NuvioTpk60` | tizen80 (API8) | Tizen 6.0 (TVs 2021) |
| `NuvioTpk65` | tizen90 (API9) | Tizen 6.5 e 7.0 (2022-2023) |
| `NuvioTpk` | net6.0-tizen8.0 (API11) | Tizen 8+ (2024 em diante) |

O `GLWindow` mudou de assinatura na API9 e de nome na API11, por isso tres
pacotes para 6+. No 4.0-5.5 nao ha GLWindow: o `NuvioTpk40` usa a
`TVGLApplication` da Samsung (pacote `Tizen.NET.TV`), no molde do
JuvoPlayer.OpenGL, com o video numa janela ElmSharp rebaixada. La a `.so`
propria foi recusada com certificado Public (spike.2, #137, cara de UEP); o
manifesto declara um privilegio Partner para o Apps2Samsung assinar como
Partner. Se ainda assim a TV recusar, a tela mostra o erro do `dlopen`.

## Estado atual (2026-09-29)

Provado em TV real, nao afirmado:

- **Tizen 6.0 (2021): FUNCIONA.** rawldon (AU7000) roda como app principal —
  video, audio, legenda, GIF e trailer. `Tpk60`/`Tpk65`/`Tpk` sao a rota boa
  para 6+.
- **Tizen 9: abre pelo Apps2Samsung, NAO pelo menu da TV** (#170, #137).
  pokazideia (UN75CU7700GXZD, Tizen 9) disse isso com todas as letras, e o D1
  confirma: os logs "(anterior)" dessa TV (ids 4573, 6507, 8634, host api11,
  `espera=50 swap0=1`) chegam a `[t] ... primeiro quadro na tela` e param ali,
  sem mais nenhum quadro nem tecla, ate `DALI_FRAMEWORK_DESTROY`; o arranque
  seguinte, pelo Apps2Samsung, roda normal. Hov1122 (QN90D) e base08 (S90C)
  descrevem o mesmo pelo menu ("barra de carregamento e volta ao menu").
  Suspeita, NAO provada: pelo menu, o lancador sobe a janela principal (opaca
  para o gerenciador de janelas) por cima do GLWindow, que fica coberto e para
  de desenhar. No spike o Hov1122 viu o quadrado do GLWindow "rapidamente" no
  canto antes da tela do spike cobri-lo, o que casa com isso.
- **Rastro + vigia no host 6+** (`Program.cs`): `data/tpk-etapas.txt` com
  begin/ok/fail/note (dlopen-so, video-init, nv_tpk_iniciar, gl-window,
  first-frame, steady-frames = 30 quadros, first-key) e notas de pausa,
  retomada, visibilidade e foco das duas janelas. O arranque seguinte mostra
  "Previous launch stopped at: ..." no topo por 30 s e poe o rastro no
  nuvio.log (`[etapa-anterior]`). O vigia sobe o GLWindow (`Raise`) se as
  chamadas de desenho pararem por 1,5 s com o app em primeiro plano e a janela
  principal visivel; se em 4 s nao voltar, a tela explica, com os numeros.
- **Canario de janela** (`canary/tpk-janela`, NADA provado em TV): chaves
  `JANELA_*` no topo do `Program.cs`, cada uma com linha `[janela] ...` no
  nuvio.log e nota no rastro. Padroes: principal em `WindowMode.Transparent`
  so na API11 (Tizen 9 pelo menu); `SetOpaqueState(true)` na principal na
  API8/API9 (app anterior e TV Plus por baixo); GLWindow opaco DESLIGADO (ele
  cobriria a propria janela do video); GLWindow sobe apos cada AppControl fora
  da API8; saida limpa (player solto com Display nenhum, janelas escondidas,
  `_exit` se o processo ainda viver 4 s depois do `Exit()`). O `PRIME_AUDIO`
  (clipe mudo, provado contra o TV Plus) segue ligado e independente.
- **Tizen 4.0/5.0 (2018-2019): a `.so` de ARQUIVO e barrada pela UEP.**
  Medido no probe (optiman, QE55Q6FNA, Tizen 4.0): `dlopen` de `lib/` e de
  `data/` falham ("failed to map segment"), MAS memoria anonima executavel e
  permitida (`mprotect +EXEC` OK). Rota em teste: carregar a `libnuvio.so` por
  `memfd_create` (syscall 385, a libc da TV nao exporta o nome) + `dlopen`
  em `/proc/self/fd/N`, com um carregador de ELF em memoria anonima como
  reserva. Host `NuvioTpk40` sendo adaptado para isso.
- **NaCl (.nexe em .wgt): descartado** — nao instala nas TVs testadas
  (optiman erro 118019, rawldon idem).

### Tizen 4/5: `.so` propria, sem TLS, e rastro de etapas (#137, #180)

O `NuvioTpk40` carrega a `libnuvio.so` por memfd (`syscall 385` +
`dlopen("/proc/self/fd/N")`) ou, se a UEP barrar, por um carregador de ELF
proprio em memoria anonima (`NuvioTpk40/Program40.cs`). Esse carregador nao
monta TLS de compilador, e a TV 2018-2020 fechava na primeira requisicao HTTPS
(o primeiro acesso a um `_Thread_local` de `src/rede.c`). Por isso:

- `tools/tpk.sh` compila uma **segunda** `.so` so para este pacote
  (`build/tpk/libnuvio-tpk40.so`, `-DNV_TPK40`): `rede.c` guarda o estado por
  fio em `pthread_key` em vez de `_Thread_local`, e o link leva
  `--hash-style=both`. O script falha se ela sair com `PT_TLS`, relocacao
  `R_ARM_TLS_*` ou sem `DT_HASH`; `tests/tpk40_tls.sh` confere de novo. A
  `.so` dos pacotes 6+ e a de sempre.
- O carregador de ELF **recusa** (com a frase na tela, em ingles) `PT_TLS`,
  relocacao que nao sabe aplicar, falta de `DT_HASH` e simbolo indefinido
  nao-fraco que o `dlsym` nao acha — antes pulava tudo isso em silencio.
- `data/tpk-etapas.txt` recebe `begin X` / `ok X` / `fail X` / `note ...` do
  host e do C (`nv_tpk40_etapa`, `open`+`write` com `O_APPEND`, sem handler de
  sinal). No arranque seguinte ele vira `tpk-etapas-anterior.txt`, entra no
  `nuvio.log` e, se acabou numa etapa sem `ok`, a tela mostra
  "Previous launch stopped at: ..." por 30 s para fotografar.

## O que falta

- cabecalhos de addon alem de `User-Agent` e `Cookie` (o player da Samsung
  nao aceita `Referer`);
- zoom/recorte do trailer no `.tpk` (ROI do player), e audio inconsistente em
  tela cheia (#178, Tizen 6.0);
- foco de audio: o canario `AudioStreamPolicy` NAO calou o YouTube por baixo
  nem devolveu o audio ao sair (rawldon, Tizen 6.0) — precisa de outra via.
