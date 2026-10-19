# F03 — epoch e diagnóstico: milestone estável, execução pausada

2026-10-03. Checkout: `codex/integration-180-glass`, em
`/Users/hrocha/.codex/worktrees/integration-180-glass/nuvio-native-legacy`.
Este documento complementa `HANDOFF-NEXT-AGENT.md`. O usuário pediu encerrar
o que já estava rodando e transferir a continuidade. **Não foi iniciado o
wiring de duração/runtime. Não houve commit, staging, instalação ou publicação.**

## Entrega concreta

- Android observa a rede padrão com `ConnectivityManager`: nova rota,
  capabilities relevantes, link properties, bloqueio e desconexão invalidam
  imediatamente o histórico. Espera capabilities Internet+Validated e link
  properties antes de anunciar rede conhecida. A sequência é opaca, monotônica
  e somente do processo; não armazena nem registra SSID/IP/credenciais.
  Callbacks atrasados de outra rede e após destroy são ignorados. Falha ao
  registrar o monitor deixa a rede desconhecida. O monitor é desregistrado no
  destroy. JNI publica a invalidação e o epoch sob a mesma trava C.
- `redemarca.c/h` começa em zero, rejeita notificações antigas e limpa o engine
  ao trocar a marca, inclusive desconectar/voltar. Nenhum epoch constante,
  fingerprint de LAN ou sequência de health check foi criado.
- N01 ganhou GET descartável: guarda somente prefixo512 bytes, respeita cap
  de bytes aceitos **antes de alocar o corpo**, janela e cancelamento. Não
  acumula mídia em RAM. Mantém os limites de headers, TLS verificado, prazo
  total, redirects e descarte de headers autenticados entre origens. Distingue
  fim por janela, teto, EOF real, timeout, truncamento e cancelamento.
- Android/libcurl não possui CA fallback de sistema. O N01 agora reutiliza o
  bundle Mozilla já configurado no arranque por `rede_discord_ca`, salvo CA
  explícito do pedido. Não desabilita TLS nem baixa certificados adicionais.
- `medirNucleo()` real do diagnóstico usa `streamfitdiag_medir()` quando o
  epoch é confiável. O pedido já solicitado mede mídia via N01; sem HEAD
  adicional no resolvedor, sem retry implícito. Conserva a tentativa existente
  de416 a partir do começo. Usa os mesmos segundos/teto existentes.
- A ponte aceita apenas200/206, MIME `video/*` conhecido, prefixo sem HTML/JSON
  de aviso, host final e pelo menos5 intervalos completos. Cada taxa usa
  bytes×8/duração **real** do intervalo; não fabrica múltiplos segundos de uma
  transferência agregada. Intervalo final curto não entra; zero por stall
  real entra. Teto, transporte falho, timeout, HTML, MIME desconhecido,
  cancelamento ou epoch diferente não alimentam o histórico. Logs novos em
  inglês, com counts/status/erro, sem URL/headers/rede/título.
- UI thread impede iniciar o teste com player/mini ativos; `app.c` cancela
  antes de cada abertura de player e durante player/mini ativo. O cancelamento
  é pedido, não uma promessa de encerramento síncrono de socket/DNS.

## Arquivos desta fatia

Produção:

```text
android/app/src/main/java/space/nuvio/nativelegacy/NuvioActivity.kt
src/app.c                     (somente cancelamento do diagnóstico)
src/diagnostico.c              (epoch, rota N01 e guard de playback)
src/diagnostico.h              (cancelar_vazao)
src/rede.c                    (bundle CA acessível ao N01)
src/rede.h                    (campos aditivos descarte/cancelamento/resultado)
src/rede_pedido.inc            (pré-requisito N01; modo descartável)
src/redemarca.c                (novo)
src/redemarca.h                (novo)
src/streamfitdiag.c            (novo)
src/streamfitdiag.h            (novo)
```

Testes:

```text
tests/streamfit_diagnostico.c   (novo; inclui medirNucleo real)
tests/streamfit_diagnostico.sh  (novo; nenhum core inteiro)
tests/streamfit_diagnostico_server.py (novo; HTTP local)
tests/redemarca_android.py     (novo; Kotlin real extraído + SDK35 + replay JVM)
tests/rede_pedido.c            (pré-requisito N01; teste bundle CA do arranque)
tests/rede_pedido_buffer.c     (pré-requisito N01; descarte sem allocation)
```

`tests/rede_pedido.sh`, `tests/rede_pedido_server.py`, `tests/rede_retry_wgt.cjs`
e os arquivos N01 citados em `tests/rede_pedido.md` já pertenciam à entrega
anterior, ainda sem commit. **Não selecionar arquivo inteiro indiscriminadamente**:
`diagnostico.c`, `rede.c/h` também contêm fixes anteriores; o índice e arquivos
de outros agentes foram preservados. Nenhuma mudança em player.c, ajustes.c,
detail, Home renderer, streams.c/h, extras ou ParaleloDataSource/NvPlayer.

## Evidência final

Todos PASS na fonte final, sem core build ou rede externa:

| Comando | Evidência |
|---|---|
| `NV_SANITIZERS=1 bash tests/streamfit_diagnostico.sh` | `/tmp/nuvio-streamfit-diagnostic-final-asan.log` |
| `NV_TSAN=1 bash tests/streamfit_diagnostico.sh` | `/tmp/nuvio-streamfit-diagnostic-tsan.log` |
| `NV_SANITIZERS=1 NV_TPK40_TEST=1 bash tests/rede_pedido.sh` | `/tmp/nuvio-rede-pedido-discard-final.log` |
| `python3 tests/redemarca_android.py` | `/tmp/nuvio-streamfit-network-android.log` |
| `bash tests/rede_buffer.sh` | `/tmp/nuvio-rede-buffer-f03.log` |
| `bash tests/rede_reuso.sh` | `/tmp/nuvio-rede-reuso-f03.log` |
| `bash tests/rede_parada.sh` | `/tmp/nuvio-rede-parada-f03.log` |
| `bash tests/rede_sonda.sh` | `/tmp/nuvio-rede-sonda-f03.log`, incluiWGT |
| `bash tests/diagnostico_persistencia.sh` | `/tmp/nuvio-diag-persistencia-f03.log` |
| `bash tests/diagnostico_dispatch.sh` | `/tmp/nuvio-diag-dispatch-f03.log` |

Também PASS a porção focada de `tests/vazao.sh` **antes de `sources=()`**:
conta/agendador e transferência legacy contra seu HTTP local. Log
`/tmp/nuvio-vazao-legacy-focused.log`. **O fluxo full-core `vazao_fluxo` não
foi executado nesta fatia**; root serializa esse gate posterior.

O caminho real `medirNucleo` seguiu redirect de127.0.0.1 para localhost,
descartou o header sintético privado e registrou6 intervalos no host final,
incluindo zero por silêncio real. O resolvedor continuou desconhecido. Testes
também cobriram503, corpo truncado, cap, deadline, cancelamento em stall,
desconexão, HTML, HTML com MIMEvideo e MIMEunknown. Nenhuma fixture falsa foi
injetada no transporte. O relógio SDL da seleção é um double monotônico para
evitar dependência AppKit; libcurl, engine e bridge são produção real.

Syntax host de rede/bridge/epoch (`-Wall -Wextra -Werror`) e de app/diagnóstico
passou. NDK27.2 syntax arm64 e ARMv7 com `NV_ANDROID` também passou. O teste
TPK40 compilou rede e verificou ausência de TLS de compilador. Kotlin compilou
o bloco real contra android-35.jar e reproduziu route/link/caps/blocked,
notificação antiga, unregister e falha do monitor. Não é build de APK/TPK/WGT
nem prova de callbacks/latência em TV física.

## Limites e continuidade

1. **Runtime não ligado:** ainda falta duração real por id de filme/episódio
   e backend. Portanto a folha pode permanecer `SF_SEM_DURACAO`; não anunciar
   reordenação end-to-end pronta. `streamfit.c/h` e `streams.c/h` existentes
   foram preservados. Próxima tarefa: APIs de identidade/proveniência de extras
   para filme e CatEp real, depois `video_duracao()` com alvoPlayer + player
   com vídeo/pronto/sem falha, jamais trailer ou duração presumida. Não começou.
2. **Android diagnóstico usa uma conexão N01:** conservador; não é a soma
   antiga de4 conexões com segundos não sincronizados. Não afirmar equivalência
   ao throughput paralelo de playback ou aprendizado passivo. TransferListener
   em ParaleloDataSource continua pendente, sem novas mudanças nesses arquivos.
3. **LG e SamsungTPK:** sem adaptador confiável de rede, epochzero. Teste de
   velocidade legacy continua funcionando; não entra como dado StreamFit. Não
   deduzir identidade de interface/endereço ou de uma resposta HTTP.
4. **SamsungWGT:** XHR síncrono não fornece o contrato N01; não foi criado
   cancelamento fictício. Legacy mantém limitações existentes; sem ingestão.
5. **Cobertura de mídia é conservadora:** MIMEausente, octet-stream e playlists
   ficam desconhecidos. Providers que entregam vídeo genérico precisam prova
   adicional de formato. Não liberar essas respostas por extensão do caminho.
6. **Redirects e orçamento:** N01 descarta todos os headers do chamador quando
   muda a origem, inclusive Range. O host final pode responder200 começando do
   byte0; não inventar que foi medido offset5MB. Não resolve host de todos os
   links de um add-on nem generaliza o resolvedor. O cap limita bytes aceitos;
   libcurl pode já ter recebido um bloco rejeitado no fio. Nenhum corpo grande
   é alocado; timeout/DNS/cancel continuam cooperativos conforme o contratoN01.
7. O monitor nativo é invalidado imediatamente por JNI; fontes de conta/perfil
   usam a mesma rede física do aparelho. Histórico sóRAM, sem persistência ou
   medição depois de reinstalar/reiniciar. A UI de origem/idade/indisponibilidade
   e explicação do guard de playback ainda deve ser integrada/revisada depois.
8. Full-core integrado, package e validação física Android/LG/Samsung permanecem
   gates do coordenador. Não interpretar o APKsnapshotLuna instalado como prova
   deste código, que foi escrito **depois** do congelamento.

Fontes primárias usadas no contrato de callbacks/abort:
[Android NetworkCallback](https://developer.android.com/reference/kotlin/android/net/ConnectivityManager.NetworkCallback),
[Android network state](https://developer.android.com/develop/connectivity/network-ops/reading-network-state),
[libcurl progress](https://curl.se/libcurl/c/CURLOPT_XFERINFOFUNCTION.html).
