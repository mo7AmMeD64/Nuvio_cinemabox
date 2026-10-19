# Handoff da integração Nuvio 1.8 — 03/10/2026

## Pedido final do Henrique

Parar ao terminar somente o que já estava em execução. Não iniciar tarefas novas. O próximo agente assume este documento e o plano; não interpretar a existência do roadmap como autorização para o agente anterior continuar. Nenhuma release 1.8 foi publicada.

## Onde continuar — não perder o trabalho

- **Checkout de integração:** `/Users/hrocha/.codex/worktrees/integration-180-glass/nuvio-native-legacy`
- **Branch:** `codex/integration-180-glass`
- **HEAD:** `e688599116043057f72a83fbee9087cac1ea973d`
- **Há muitas alterações staged, unstaged e arquivos novos. O HEAD sozinho NÃO contém a entrega.** Não resetar, limpar, trocar arquivos inteiros nem fazer cherry-pick presumindo que as correções estão commitadas. Revisar `git status`, `git diff`, `git diff --cached` e arquivos não rastreados.
- Checkout original do usuário: `/Users/hrocha/Projetos/Pessoal/LG WEB/nuvio-native-legacy`, branch `fix/tpk-selo-hdr`, também sujo. Foi preservado. Não confundir com a integração.
- `saida=` não rastreado tem proprietário desconhecido; não apagar.
- Disco interno ficou apertado; builds e snapshots estão em `/Volumes/ExternalSSD`.
- Plano completo: [PLANO.md](PLANO.md). Inventário: [INVENTARIO.md](INVENTARIO.md), [INTEGRACAO.md](INTEGRACAO.md).
- Histórico de execução: `.superpowers/sdd/PLANO/progress.md`. É cronológico: as últimas entradas substituem estados antigos, inclusive mensagens de testes inicialmente falhos.
- Referências do dono: `docs/HANDOFF-1.8.0.md` e `/private/tmp/nv-codex-ctx/docs/glass-ui-contexto.md`.

## O que está na Android TV

Smart TV Pro, Android 14, `192.168.1.128:5555`. Instalação autorizada feita com `adb install -r`, preservando dados. APK armeabi-v7a e arm64-v8a; TV usa armeabi-v7a. Versão de prévia **1.8.0 / 10800**, não release pública.

- Fonte imutável: `/Volumes/ExternalSSD/nuvio-180-preview-ui-home-final`, `manifest.json` com 1275 hashes de src/tests.
- Cópia de empacotamento: `/Volumes/ExternalSSD/nuvio-180-android-preview`.
- Commit local de empacotamento: `6fa8e2d4d8abb9c8b28054d720e9729f8bd7ca2d`.
- APK: `build/release-1.8.0/Nuvio-1.8.0-android.apk` nessa cópia.
- SHA256 produzido e extraído da TV, comparados pelo coordenador: `8d012314de5b73d60958dec7f06c1f6620abd7842550d8308f66fb46b03216d4`.
- Foto física inspecionada: `build/release-1.8.0/android-tv-home-settled.png`; Home com CW, amigos e For You.
- Detalhes: [ANDROID-PREVIEW.md](ANDROID-PREVIEW.md).

Inclui restauração Details 1.7.4, véu contínuo Apple TV, ratings 1.7.4, elenco/recomendações separados, Home inicial progressiva, #158/#233/#228 reproduzidos, diagnóstico/persistência, proteção de preferências locais, Seekr Settings/quota e primeira versão N01. **Não inclui o OSD Seekr nem o wiring F03 feitos depois de congelar a fonte.**

As capturas espaçadas não medem latência de startup. Não houve benchmark comparável 1.7.4/1.8 na TV nem prova de reprodução/fluidez completa. Luna usou monkey para lançar; o salto do seletor para Home não prova retomada automática. Em próximas aberturas, resolver componente e usar `am start`, sem evento aleatório. Nenhum teste físico Samsung/LG novo.

A cópia de build tem versão ajustada; a integração ainda conserva versões antigas em alguns arquivos. Não sincronizar versão/release a partir de um número de preview. Um build anterior pegou objetos antigos porque copy2 preservou timestamps: invalidar objetos ou tocar todos os fontes antes de recompilar essa cópia.

## Concluído nesta integração, com limites claros

| Frente | Entrega | Prova / limite |
|---|---|---|
| Details / Apple TV | Restauração visual v1.7.4 preserva fan, episódios, navegação e integrações; logo primeiro, botões abaixo; legenda do título estrangeiro sem esperar decode; sem elenco inline | GL local e revisão; não declarar toda UI fisicamente aprovada |
| Véu | Endpoint da expansão usa composição contínua, inclusive Imersiva/Frost/blur/poster/sem arte | 7 cenários GL; diferença p95≤1/255; [VEU-CONTINUIDADE.md](VEU-CONTINUIDADE.md); vídeo nativo/trailer e GPU real pendentes |
| Ratings e fileiras | Render notasui v1.7.4, API média preservada; elenco seguido de recomendações em filme/série | 10 itens, foco/scroll além do 7º, ponteiro, rota TMDB tv/movie; [DETAIL-ROWS-REVIEW.md](DETAIL-ROWS-REVIEW.md) |
| Home loading | Só CW/social é estado inicial: primeira fileira pode aparecer sem esperar última; cache/listas/coleções já prontos não encolhem | Antes controlado pronta1ms/visível354ms; depois publicação no instante de pronta; 5 cenários + suites legadas ASan/UBSan. Não é ganho físico medido. [HOME-LOADING-IMPLEMENTATION.md](HOME-LOADING-IMPLEMENTATION.md), [review](HOME-LOADING-REVIEW.md) |
| #233 | Coleções por ID estável, snapshot vazio válido remove; JSON/RPC inválido não apaga; editor e Home consistentes; coleção fora da quota de catálogo | Parser/projeção/Home/concorrência e revisão. Logs atuais do reportador/TV Samsung pendentes |
| #158 | Addons de legenda não colapsam por fornecedor; CW local preserva metadados/registro offline | Correções reproduzidas/testadas; não provado que explicam Flix-Streams específico ou todos os logs do usuário |
| #228 | Fallback de trailer e estado prepared/playing corrigidos nos caminhos reproduzidos | Suites focadas; Samsung Q80A físico pendente |
| Diagnóstico de memória | Separa candidato, orçamento aplicado, persistido, rollback, manual override; trata falha de disco/timeout/commit | 19 casos, ASan/TSan, teste real de cache; não aumentou caps de hardware nem provou ganho |
| Perfis por aparelho | Incoming blob agora respeita somenteDesteAparelho; RAM/qualidade/GPU locais não seguem outra TV | ajustes_perfil PASS; UUID cliente.txt existente reutilizado; teste de duas TVs pendente. [DEVICE-PREFERENCES.md](DEVICE-PREFERENCES.md) |
| Seekr engine/Settings | Chave pessoal, 50 consultas por instalação/dia UTC compartilhadas entre perfis/chaves; persistência, cache/singleflight, 429 separado, retry local, falhas de gravação | Suites quota/estados/Settings; WGT Retry-After exposto ou desconhecido |
| Seekr OSD, NÃO instalado | Cápsula durante scrubbing com causa, uso/reset; sem chave/clock/storage/provider distintos; some ao soltar | Teste unit rerodado pela raiz + 16 GL vidro/sólido do executor; raiz inspecionou quota. [F02-SEEKR-PLAYER-OSD-ENTREGA.md](F02-SEEKR-PLAYER-OSD-ENTREGA.md) |
| N01 inicial | API de request isolado, TLS, caps antes de alocar, redirects sem vazamento, deadline, cancelamento/geração, métricas | HTTP/TLS local, ASan/UBSan/TSan, TPK40 host e wrappers; raiz rerodou teste. [NETWORK-REQUEST-CONTRACT.md](NETWORK-REQUEST-CONTRACT.md). F03 abaixo estende esta API |
| Biblioteca/Guia | Fade/recorte da biblioteca portado seletivamente, Unicode do Guia confirmado | Fixture GL e teste jscadeia |

O OSD adicionou getter narrow `ajustes_seekr_habilitado()` (opção local), mantendo `ajustes_seekr_ligado()` (opção+chave) para dispatch. Arquivos: player.c, ajustes.c/h e quatro tests/player_seekr_status*. Sem novos pedidos a providers.

## Último trabalho concluído antes da parada: F03

**Estável, não instalado:** epoch Android real via ConnectivityManager/JNI e diagnóstico de mídia ligados ao engine por host final. N01 ganhou modo descartável limitado e usa o bundle CA já configurado no arranque. Player/mini bloqueiam ou cancelam o teste ativo. Android usa uma conexão conservadora; LG/TPK sem epoch confiável e WGT sem contrato N01 continuam desconhecidos para StreamFit, mantendo o diagnóstico legacy.

ASan/UBSan, TSan, callbacks Kotlin/SDK35, syntax NDK arm64/ARMv7 e regressões focadas PASS. Coordenador conferiu relatórios/logs e hunks de cancelamento/epoch; **revisão integrada completa, full-core/APK e prova física deste delta ainda faltam**. Runtime metadata/playback e telemetria passiva NÃO começaram. Sem duração válida, a classificação ainda pode ficar desconhecida: não anunciar seleção end-to-end pronta.

Entrega e lista exata de arquivos/comandos: [F03-EPOCH-DIAGNOSTICO-HANDOFF.md](F03-EPOCH-DIAGNOSTICO-HANDOFF.md). Produção: NuvioActivity.kt, app.c (cancelamento), diagnostico.c/h, rede.c/h/rede_pedido.inc, novos redemarca.c/h e streamfitdiag.c/h. Não modifica NvPlayer/ParaleloDataSource/streams runtime nesta fatia.

## O que falta, em ordem de retomada

1. **Revisar e consolidar o estado sujo:** comparar staged/unstaged/untracked, incorporar relatórios/fixtures novos; commits seletivos por frente e merge final ainda faltam. Não repetir port de arquivo inteiro. Fazer build integrado a partir de fonte congelada contendo os últimos deltas, pois a TV está numa fonte anterior.
2. **Fechar F03 StreamFit:** runtime verdadeiro por alvo exato (filme/episódio), duração do backend sem PLR_DUR_PADRAO/trailer; duração real vence metadado atrasado. Telemetria passiva Android TransferListener ainda falta; não usar soma sem sincronização dos downloads paralelos nem cache/pausa/buffer cheio como vazão. Expor origem/idade/orçamento/indisponibilidade de forma traduzida. Não generalizar host de resolvedor nem rede por chave constante. Preservar ordem/grupos/foco e modo primeira fonte.
3. **F04 legendas:** somente auditoria/contrato, nenhum código novo de F04. [F04-LEGENDAS-AUDITORIA-CONTRATO.md](F04-LEGENDAS-AUDITORIA-CONTRATO.md). Seleção atual está em faixas.c, legendasui.c não existe. UI simples idioma+origem/embutida/addon e Mais opções; candidatos estáveis sob chegada tardia. Segunda legenda precisa loader/documento/render independente e colisões; ASS/mkvass atuais são singletons. Não oferecer opção secundária que não desenha.
4. **F05 AutoSync:** engine/documentos/562 testes existem; faltam coletor de referência embutida independente e ligação ao player, sessões/cancel, Quick/Thorough/tolerância/desfazer/outra referência. Sem bloquear startup; aplicar apenas confiança comprovada; primeiro uma língua, depois duas. Documento: docs/plans/player-1.8/AUTOSYNC-IMPLEMENTACAO.md.
5. **F06 áudio:** provar PCM por backend antes de VAD/modelo opcional; download explícito, hash/tamanho/cancel/remover, RAM/latência/confiança. Não desligar passthrough/offload silenciosamente. LG/Samsung ainda sem prova de diálogo acessível.
6. **F07 cache/boost:** implementar/provar cache temporário 256/512/1024MB, Range/auth, cancel, ENOSPC, crash/limpeza; volume até200% por sessão, reset e vermelho >100%. Buffer interno AVPlay não é cache em arquivo. Capacidade real por plataforma é gate F00.
7. **F08 Social:** consolidar conta/perfil Nuvio independente de Trakt/Simkl; deduplicar canais/amigos e associar identidades verificadas. Completar comparações filme/série/gênero/gostos/match/progresso com cobertura real e estados desconhecidos. Revisar/deploy backend/migração ainda pendentes; não presumir APIs Simkl/Letterboxd plenamente integradas. Card horizontal/menu fora/verde online precisam QA final.
8. **F09/F10 plugins/P2P:** port seletivo plugins2/p2p2 após revisar N01; limites globais JS/DOM/filas, cancel/geração/auth; P2P shutdown antes de limpar escritores, RAM/disco/statvfs/watchdog. Toolchains/motor/licenças LG/Android/TPK; WGT não tem sockets locais. Não chamar inventário de implementação completa.
9. **F11 servidores pessoais:** pesquisa pronta em docs/plans/media-servers-1.8; implementação Jellyfin ainda falta (auth/biblioteca/PlaybackInfo/perfil/DirectPlay/progresso), depois Emby/Plex próprios. Isolamento de origem/usuário/IDs e sessões autenticadas.
10. **F12 TopN/ícones/scripts:** conferir deltas reais restantes, paginação/custo, ícones opcionais Android e issues226/234; scripts Mac/Linux/caminhos com espaços. Ainda há itens no inventário.
11. **QA e performance de plataforma:** medir Home→fileiras, Play→primeiro frame+áudio e controle→resposta; frames/decodes/uploads/RAM/cache/scroll/fan/Frost, antes/depois com condições equivalentes. Reproduzir Android e LG/Samsung separadamente. Checar menu Apple, sidebar Moderna, Guide/addons/categorias/source/modal amigo, escala Settings e detalhes/heros sem regressão. Captura estática não mede fluidez.
12. **Logs/issues/release:** restaurar acesso a logs atuais do worker, correlacionar build/horário/aparelho antes de responder/fechar #158/#228/#233 e demais issues. Revisar pendências #229/#231/#232/#227 e PR230 contra código atual, não assumir encerramento remoto. Atualizar What's New e versões finais só após escopo real pronto; builds assinados por alvo, hashes e updater/Homebrew, smoke tests, tag/release/artefatos do mesmo commit e respostas honestas às issues. Nenhuma publicação nesta pausa.

## Testes/evidência para retomar

Comandos na integração, ou no snapshot correspondente quando houver WIP concorrente:

```sh
bash tests/detail_layout.sh
NV_ROWS_ONLY=1 NUVIO_SHOT_IDIOMA=0 bash tests/detail_secoes_shot.sh /tmp/nuvio-rows
bash tests/player_seekr_status.sh
bash tests/ajustes_perfil.sh
bash tests/diagnostico.sh
SANITIZE=1 bash tests/home_progressiva.sh
SANITIZE=1 bash tests/montagem_cedo.sh
bash tests/rede_pedido.sh
```

Antes de repetir testes caros, consultar logs e quais fontes foram alteradas. GL exige contexto gráfico no Mac. TPK40 host/syntax não prova ARM ou TV. Relatórios de F03 devem adicionar seus comandos específicos.

Snapshots úteis: `/Volumes/ExternalSSD/nuvio-180-ui-continuity` (UI final), `/Volumes/ExternalSSD/nuvio-180-execution-51fxfkxd` (coorte anterior), `/Volumes/ExternalSSD/nuvio-174-detail-reference` (baseline v1.7.4). `/tmp` contém capturas/logs, pode ser volátil; referências versionadas estão em docs/releases/1.8.0.

## Restrições e coordenação

- M3 localhost18080 recusou conexão; memory_write foi tentado, não salvo. Não há memória remota confirmada desta entrega.
- Bitwarden `bw status`: unauthenticated. D1 anterior Error7403; não afirmar leitura de logs novos do worker. Segredos nunca em handoff/log.
- Usuário autoriza subagentes e modelos por custo: Luna para build/coleta/docs; Sol para código/integração/review; Astra pontual para contratos de risco, não para tarefas simples. Neste chat slots estavam ocupados e agentes foram reutilizados; não pressupor que continuarão após a pausa.
- Chat Android Luna: `01a10412-0de7-7480-9bd5-55a436a37c6c`, finalizado; não fará outra instalação/interação.
- Preservar feedback visual: layout antigo de Details; logo antes de tudo e botões abaixo; fan abre ao focar primeiro botão funcional; carrossel não troca título enquanto expandido; elenco separado de recomendações; não recriar redesign rejeitado.

## Checkpoint para recuperação

`/Volumes/ExternalSSD/nuvio-180-handoff-stop-u4ldpf03` contém HEAD/status, patches separados staged/unstaged, cópias dos arquivos alterados e fontes/testes/docs novos, com manifesto SHA256. Não é clone completo; usar o checkout de integração é o caminho principal. Não aplicar os patches novamente sobre ele. Os agentes de implementação encerraram; o chat Android está idle. O slot antigo issue158_fix permaneceu pending_init e recebeu interrupt, sem execução nova.
