---
name: samsung-tpk
description: Gerar, testar e publicar o Nuvio nativo .tpk para Samsung Tizen 6.0+ (host .NET NUI; GLWindow no 6.0/6.5, GLView na janela principal no 8/9; pacotes NuvioTpk60/65/NuvioTpk). Use para qualquer build .tpk, canario nativo da Samsung, ou mudanca em src/tpk.c, src/video_tpk.c, src/gpunivel.c, tizen-tpk/.
---

# .tpk Tizen 6+ (NUI)

O mesmo C do app (`src/*.c`, `-DNV_TPK`) vira `libnuvio.so`. Um host .NET
(`tizen-tpk/Program.cs`, compartilhado pelos tres pacotes) desenha e, a cada
quadro, passa o contexto EGL ao fio do app (`src/tpk.c`). Video:
`Tizen.Multimedia.Player` no plano de video (`tizen-tpk/Video.cs` +
`src/video_tpk.c`). Arquitetura: `tizen-tpk/README.md`. O `tools/tpk.sh` gera
tambem o pacote do 4/5 (skill `samsung-tpk-legacy`).

| Pacote | API | Tizen | Superficie |
|---|---|---|---|
| `NuvioTpk60` | 8 (`NV_API8`) | 6.0 | GLWindow |
| `NuvioTpk65` | 9 (`NV_API9`) | 6.5–7 | GLWindow |
| `NuvioTpk` | 11 | 8–9 | **GLView na janela principal** (`API11_GLVIEW`) |

## Build

```bash
git worktree add --detach ../nuvio-tpk-build <commit>   # nunca da arvore suja
cd ../nuvio-tpk-build
NUVIO_PROPERTIES=".../NuvioWeb-0.3.38-beta/local.properties" bash tools/tpk.sh
git checkout -- tizen-tpk/*/tizen-manifest.xml          # o tpk.sh reescreve a versao
```

Sai em `build/tpk/`: os quatro `.tpk`, `libnuvio.so` (6+), `libnuvio-tpk40.so`
(4/5, sem TLS) e os anexos de auto-atualizacao `libnuvio-<v>-tpk-arm.so` /
`-tpk40-arm.so`. Para release use `tools/release-samsung.sh` (skill
`samsung-release`), que faz isto e confere tudo.

Pre-requisitos: Docker/OrbStack ligado (`open -a OrbStack`; com o daemon
parado o build falha mal); imagem `nuvio-tpk-sdk` (`tools/tpk/Dockerfile`,
Debian buster armel, glibc 2.28); deps estaticas em `~/.cache/nuvio-tpk/prefix`
(`tools/tpk/deps.sh`, caminho sem espaco); `.NET 8 SDK` em `~/.dotnet` +
workload Tizen.

## Testar sem TV

`bash tools/tpk-testa.sh 60` roda a `.so` num host falso ARM (Mesa por
software). NAO exercita o host .NET, o player, nem a janela: mudanca em
`tizen-tpk/*.cs` so se prova na TV (canario).

## O que ja foi provado na TV (29/09/2026) e nao pode regredir

- **Tizen 8/9 pelo menu da TV**: com GLWindow separado a TV PAUSAVA o app ~3 s
  depois de abrir (quando o GLWindow pegava o foco) e fechava em ~10 s; pelo
  Apps2Samsung abria. Com `GLView` dentro da janela principal abre normal
  (S90C, QN90D/S90D, CU7700). Nao volte a GLWindow no API11.
- **Samsung TV Plus tocando por baixo**: 1,5 s apos abrir o host toca
  `res/silencio.mp4` pelo mesmo caminho do filme e solta (`PRIME_AUDIO`).
  AudioStreamPolicy NAO resolve (testado, e deixa a TV preta ao sair).
- **Trailer**: no `.tpk` a Apple toca so variante de VIDEO (sem audio, o master
  travava o muse-server); a tela cheia e o cartao que continua no detalhe
  tentam o IMDb primeiro. O zoom esconde o plano ate o recorte assentar.
- **Nivel de GPU** (`src/gpunivel.c`): adaptativo nos primeiros ~20 s de home.
  0 cheios -> 1 efeitos leves -> 2 efeitos MINIMOS (sem luz de tela cheia do
  tema imersivo, sem sombra/halo), sempre em 1080p; o 2 so quando o 1 ainda
  fica abaixo de 25 fps (Mali-400 do registro 9859). O 720p (nivel 3) ficou
  borrado demais nas 5.0 ("ninguem gostou", dono 29/09) e so existe forcado.
  Ajuste "Efeitos visuais" (Automatico/Completos/Leves) em Avancado, so no `.tpk`.
- **Locale**: o host .NET poe o processo no idioma da TV; com virgula decimal
  o JSON com `%f` quebra (todo /scrobble do Trakt deu 500 na 1.5.4). `main()`
  e `nv_tpk_iniciar` forcam `LC_NUMERIC=C` e a linha de FPS vigia. Log com
  "FPS=51,2" (virgula) = regressao disso.
- Selo HDR: so "Fonte HDR10/HDR10+/HDR" lido da fonte. Samsung nao tem Dolby
  Vision; nunca anunciar DV.

## Chaves de janela (topo de `tizen-tpk/Program.cs`)

`JANELA_PRINCIPAL_OPACA`, `JANELA_PRINCIPAL_TRANSPARENTE`, `JANELA_GL_OPACA`,
`JANELA_SOBE_GL_APPCONTROL`, `JANELA_SAIDA_LIMPA`, `API11_GLVIEW`,
`PRIME_AUDIO`. Cada uma loga `[janela] ...` e grava no rastro. Para isolar
um relato, desligue UMA por canario.

## Rastro de etapas (diagnostico sem TV aqui)

`data/tpk-etapas.txt`: `begin/ok/fail <etapa>` e `note ...` do host e do
nativo. Na abertura seguinte vira `tpk-etapas-anterior.txt`, sobe no log como
`[etapa-anterior]` e, se uma etapa ficou aberta, a TV mostra "Previous launch
stopped at: <etapa>" — peca FOTO disso ao testador. Linha `[tv] modelo=...
tizen=... host=...` diz qual TV e qual pacote.

## Regras que ja custaram caro

- `nv_tpk_quadro` NUNCA segura o framework (API8 roda o callback no fio
  PRINCIPAL). Retorno: 1 troca, 0 pula, -1 fim.
- `eglSwapInterval(0)` na API9+ (vsync derrubava o Tizen 9 a 3 fps).
- EGL por `dlopen` (`src/tpk_egl.h`); nao linkar `-lEGL`.
- `Referer` nao passa no player: so `UserAgent` e `Cookie`.
- Avisos: `plataforma` `tizen` e `tizen-tpk` (`src/avisos.c`).
- Auto-atualizacao (#181): sem lib encenada o host faz o `dlopen` de sempre;
  com lib encenada e verificada por sha256 (digest do anexo da release) carrega
  por memfd. Falha no staging apaga e volta para a empacotada.

## Referencias

- dali-adaptor `gl-window-impl.cpp` / `gl-window-render-thread.cpp`: retorno
  do callback, swap, iconify pausa o fio de desenho.
- `tizen-tpk-spike/`: o que foi medido em TVs reais (GLView provado no 9).
