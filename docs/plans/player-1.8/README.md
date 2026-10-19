# Player 1.8 — especificação e provas de viabilidade

03/10/2026. Proposta de implementação, sem benchmark físico e sem funcionalidades implementadas por este documento. Escopo definido pelo usuário: estas melhorias ficam na 1.8; Settings/fixes, recuperação de trabalho local e ícones opcionais são tratados separadamente na 1.7.3.

## Base e conclusão

Reaproveitar o player C, `legenda.c`, `mkvass.c`, `vazao.h`, `diagnostico.c`, `perfiltv.c`, `seekr.c`, `streams.c`, e o Android Media3 existente em `android/app/src/main/java/space/nuvio/nativelegacy/`. Android já está integrado à release; a pesquisa inicial que o descrevia como branch separada é anterior ao inventário de merges.

O caminho com menor dependência externa é AutoSync temporal entre legendas e adequação de streams pelo diagnóstico. Android tem APIs prontas para cache/ganho e instrumentação de transferências; acesso a PCM, ganho e cache controlado pelo app em LG/Samsung exigem provas por backend. Não prometer a mesma capacidade por plataforma antes delas. A pesquisa inicial completa está no arquivo local ignorado `docs/PESQUISA-AUTOSYNC-PLAYER.md` do checkout principal; este plano incorpora os requisitos adicionais e será versionável.

## Contratos de preferência e estado

| Dado | Escopo | Política proposta |
|---|---|---|
| Idiomas principal/secundário, escolha de add-ons | Perfil de usuário | Sincronizável pela conta, respeitando escolhas explícitas |
| Performance, cache, ganho permitido, modelo de fala e diagnóstico | Aparelho | Local; uma mesma conta pode ter ajustes distintos na Android TV e Samsung |
| Limite Seekr e uso diário | Aparelho, todos os perfis | Nunca reseta ao mudar conta, perfil ou chave |
| Chave Seekr pessoal | Aparelho | Local, mascarada, removível; não sincronizar em texto claro |
| Offset automático/manual | Sessão + identidade do documento | Independentes por idioma; troca de fonte invalida a análise |
| Aprendizado de vazão | Aparelho + rede + host/provedor | Guardar origem, idade e qualidade; não confundir CDN lenta com conexão lenta |

Identidade do aparelho: UUID aleatório gerado pela instalação, sem serial/MAC. Um perfil de performance efetivo resolve `padrão do hardware → ajustes do aparelho → override explicitamente local`; conta não deve sobrescrever silenciosamente esse resultado. Reinstalar ou limpar dados pode apagar o UUID e o contador: limite estritamente fiscalizado por aparelho requer cadastro autenticado do aparelho no backend, não apenas armazenamento local. MVP local limita uso normal; essa limitação deve estar documentada.

## AutoSync de legenda

Proposta: habilitado automaticamente quando há legenda externa preferida e referência disponível, com opção de desligar em Ajustes. A reprodução inicia sem esperar pelo resultado. Aplicar somente correções aceitas; manter ajuste manual e ação Desfazer. Não trocar a faixa escolhida para produzir referência.

Refatorar `LegendaDocumento` independente de overlay: cues, idioma, origem, identidade/hash e geração. A referência embutida fica em outro documento. O coletor ASS atual não cobre automaticamente SRT/MP4, e uma amostra à frente não comprova o título inteiro. Cancelamento usa geração de sessão, fonte e seleção.

MVP: deslocamento constante por atividade temporal, com reforço de texto apenas quando compatível. Aceitação exige cobertura de diálogo, pico distinto de alternativas e concordância entre janelas separadas. Forced/sinais, introduções repetidas, cortes diferentes e pouco diálogo devem poder resultar em recusa. Escala e retiming por segmentos são etapa posterior; offset discordante entre regiões nunca vira correção global.

| Controle | Semântica proposta |
|---|---|
| Quick | Poucas referências/janelas e orçamento curto; nunca baixa o limiar de confiança |
| Thorough | Mais referências e regiões; mesmo limiar; execução cancelável em segundo plano |
| Tolerância | Erro residual admitido, separado do intervalo máximo de busca |
| Tentar outra referência | Exclui a referência rejeitada na sessão e reinicia a busca |
| Estado exibido | Analisando / Sincronizada / Sem correspondência segura / Indisponível |

Orçamentos iniciais para protótipo, não resultado medido: Quick até 5 s de processamento e 3 regiões; Thorough até 20 s e 6 regiões, com teto separado de bytes de referência. Não aplicar durante seek/menus se o custo causar travamento. Tolerância inicial candidata 250 ms com opções 150/250/500 ms: calibrar pela distribuição de erro de anotações humanas antes de definir o padrão final.

## Segunda legenda e seletor simples

Primeira tela mostra somente os idiomas principal e secundário configurados. Para cada idioma, mostrar candidatos embutidos e de add-on com linha `Português · Embutida` ou `Português · OpenSubtitles`. Indicador de seleção distinto para principal/secundária, sem misturar idiomas. Embutida significa origem da faixa, não fornecedor externo.

Ação Mais opções abre lista completa e detalhes: nome da faixa, arquivo/release, codec, forced/SDH, origem/add-on, formato, atraso e estado AutoSync. Ocultar esses detalhes na tela inicial não pode impedir selecionar outra versão. Selecionar nenhum para secundária deve estar sempre disponível. Preservar identidade/foco na chegada tardia de add-ons.

Dois documentos, dois offsets e duas seleções. Uma referência pode ser reutilizada somente depois de validação individual. Definir duas áreas de overlay e evitar colisões com controles/letreiros ASS. Se o backend não permite duas faixas nativas, renderizar a secundária no app; testar compatibilidade real antes de habilitar.

## Áudio sync

Toggle separado, inicialmente desligado, texto claro sobre processamento local. Fluxo candidato: PCM mono → VAD → correlação temporal; ASR local opcional para áudio inglês produz referência cronometrada adicional. Legenda traduzida não corresponde diretamente às palavras inglesas: validar pelo tempo e estrutura.

Baixar modelo uma vez, aconselhar Wi-Fi, mostrar tamanho real/hash, progresso, cancelar e remover. Nenhum download automático ao iniciar filme. Silero VAD e sherpa-onnx são candidatos; comparar whisper.cpp para suporte multilíngue sem presumir menor latência. Não inferir RAM total pelo tamanho do arquivo.

Passthrough/offload e codecs codificados podem não entregar PCM. Não desativar silenciosamente passthrough, HDR/DV ou offload para analisar áudio. Se preciso, decodificação auxiliar precisa de limite de CPU/RAM e amostras, ou a função fica indisponível naquele modo. APIs externas só como avaliação separada com consentimento para envio do trecho; fluxo padrão não envia áudio, URLs assinadas nem headers de debrid.

## Cache de seek e volume

Cache de sessão em disco: Desligado, 256/512/1024 MiB (rótulos de produto 256 MB/512 MB/1 GB). Reservar espaço livre, reduzir/recusar a configuração se não couber, escrita assíncrona, limpeza ao fechar e na abertura após crash. Começar por VOD HTTP progressivo; HLS/DASH/live não herdam suporte sem testes próprios. O volume configurado e orçamento de prefetch são controles diferentes. Seek só será rápido quando o destino e dados de decodificação estiverem cobertos pelo cache.

Android: avaliar `CacheDataSource`/`SimpleCache` com `ParaleloDataSource`. HTTP Range, autenticação, redirecionamento, cancelamento e múltiplas conexões precisam preservar o contrato. LG/uMS, Samsung WGT/AVPlay e TPK: prova de proxy/fonte local compatível antes de oferecer a opção. Buffer interno AVPlay não equivale a cache de arquivo do app.

Volume: 0–100% usa caminho atual; 100–200% usa ganho real onde disponível. 200% de amplitude corresponde a aproximadamente +6,02 dB; `LoudnessEnhancer` recebe ganho em milibels, não percentagem. Controlar clipping/limitação, indicar vermelho acima de 100%, zerar ao fechar/trocar sessão. No controle remoto a barra recebe setas, sem depender de swipe. API ausente ou passthrough incompatível mantém teto100%; não elevar o volume físico da TV para simular ganho do player.

## Fontes adequadas e speed test

Aplicar partição estável sobre a ordem de base escolhida: adequadas/desconhecidas primeiro, provavelmente pesadas depois. Não ocultar fontes. Classificação fixada enquanto o seletor está aberto para preservar foco; novas respostas entram com identidade estável.

Demanda: bitrate confiável ou bytes exatos ×8/duração/1e6. Tamanho de pacote de temporada não serve para episódio; resolução sozinha não prova bitrate. `vazao.h` já oferece p20×0,75 como orçamento conservador e mediana×0,9 como teto; reutilizar com proveniência e expiração, não uma constante arbitrária copiada do fork.

Aprendizado passivo conta somente transferências de mídia com busca ativa, exclui cache hits, pausa e espera por buffer cheio. Stall enquanto download ativo é sinal; não excluí-lo para inflar a medida. `ParaleloDataSource` já recebe TransferListener: instrumentar bytes sem duplicar a contagem das conexões. LG/Samsung player fechado precisa de telemetria própria; na falta dela informar que a classificação vem do diagnóstico.

Speed test deve oferecer: rápido com endpoint conhecido (rede) e teste da fonte/host (caminho real). Exibir Mbps sustentados, margem, instante e origem, junto de demanda estimada quando confiável. Não garantir resolução ou ausência de buffering. Invalidar após troca de rede/reteste e usar expiração candidata24h até calibração. Não disparar testes ativos durante reprodução sem pedido do usuário; eles competem com a mídia. Contabilizar bytes e permitir cancelar.

## Seekr — 50 consultas por dia por TV

Usuário definiu limite50/dia por aparelho e retorno da chave pessoal. Na base examinada e1f2936b, `seekr.h` mantém a API de chave pessoal, enquanto `ajustes.c` esconde AJ_SEEKR_CHAVE/AJ_SEEKR_TESTAR quando `seekrEmbutida()` é verdadeiro (filtro próximo da linha3591). Restaurar a visibilidade e revisar precedência de chave pessoal versus embutida; editar a API operacional sozinha não restaura o campo.

Contar cada despacho HTTP de `/sprites`, incluindo retry/refresh; frames, VTT e JPEG em cache não gastam o limite local. Reserva atômica durável ANTES do despacho, mutex e escrita substitutiva; queda após reserva pode consumir uma chamada sem enviá-la, aceito para evitar ultrapassar50. Single-flight por título/duração/identidade de chave evita consultas concorrentes iguais. Cache lookup respeita validade; VTT/URL assinada não persiste além da expiração.

Proposta de janela: dia UTC para combinar com a documentação do fornecedor; apresentar horário de reinício no fuso local. Salvar último dia observado e impedir clock rollback de liberar contador; sem relógio confiável, usar última janela válida e degradar com mensagem explicativa. Para enforcement forte, backend deve controlar contador e janela, sem expor a chave compartilhada no pacote. Usar chave pessoal não aumenta o limite50 da TV; quotas do fornecedor continuam independentes e podem ser menores.

Estados distintos: limite local atingido, 429 fornecedor com Retry-After, 401/403 chave recusada, título sem preview, rede indisponível. Nada disso bloqueia seek/playback. Campo mascarado com testar/remover; validação de chave não é chamada `/sprites` e não deve gastar quota50, mas respeita seus próprios limites. Não registrar chave, Bearer, URL assinada ou resposta que a contenha.

## Diagnóstico, otimização e logs

Estender o relatório: backend/build/aparelho, memória livre/pico, armazenamento livre, bitrate e bytes por mídia, tempo de startup/seek, rebuffering, tempo de render/menu, capacidade cache/coverage, AutoSync tempo/resultado/erro/recusa, VAD/ASR RAM/CPU, Seekr uso local/status/latência. Dados agregados; conteúdo e identificadores pessoais não necessários.

A tabela `perfiltv.h` distingue LG, WGT e TPK, mas não traz entrada Android explícita: adicionar avaliação própria antes de compartilhar limites. Reutilizar decisão com histerese e reteste/rollback existente; automatizar somente knobs cuja melhoria foi medida com mesma amostra e respeitar override manual. Limite Seekr, idioma, consentimento de áudio e boost não são knobs a maximizar por otimização.

Logs novos em inglês com eventos/razões estáveis, por exemplo `subtitle_sync rejected reason=ambiguous_peak`, `seekr_lookup blocked reason=device_daily_limit used=50`, `stream_fit classified reason=estimated_bitrate`. UI continua traduzida. Preservar compatibilidade com ferramentas que parseiam logs antigos; migração por módulo com teste dos consumidores, sem trocar todos os textos de uma vez.

## Matriz de viabilidade

| Capacidade | Android/Media3 | LG/uMS | Samsung WGT/AVPlay | Samsung TPK |
|---|---|---|---|---|
| Documentos/offset externo e seleção simples | Caminho comum C | Caminho comum C | Caminho comum C | Caminho comum C |
| Referência embutida | Extração/track a demonstrar | ASS Range existe; expandir | ASS Range existe; expandir | ASS Range existe; expandir |
| Bytes reais de playback | DataSource instrumentável | API fechada: prova pendente | API fechada: prova pendente | Contrato multimedia: prova pendente |
| PCM para VAD/ASR | AudioProcessor/decoder; excluir modos incompatíveis | Prova pendente | Prova pendente | Prova pendente |
| Cache arquivo app | Media3 candidato | Fonte/proxy: prova pendente | Fonte/proxy: prova pendente | Fonte/proxy: prova pendente |
| Boost acima100 | LoudnessEnhancer candidato | Ganho/PCM: prova pendente | Ganho/PCM: prova pendente | Ganho/PCM: prova pendente |
| Quota Seekr/performance local | Comum com adaptador storage | Comum | Comum | Comum |

## Dependências prontas e fontes oficiais

| Projeto/API | Reutilização | Licença / condição |
|---|---|---|
| [alass](https://github.com/kaegi/alass) | Engine temporal / baseline; FFI Rust-C necessária | GPL-3.0; verificar compatibilidade e toolchains antes de vendoring |
| [ffsubsync](https://github.com/smacke/ffsubsync) | Baseline para referência e áudio | MIT; Python/FFmpeg inadequados para embutir sem estudo de footprint |
| [Silero VAD](https://github.com/snakers4/silero-vad) | Detecção local de fala | MIT; pesos/runtime precisam inventário |
| [sherpa-onnx](https://k2-fsa.github.io/sherpa/onnx/index.html) | ASR com interface nativa | Apache-2.0 no projeto; conferir licença de cada modelo |
| [whisper.cpp](https://github.com/ggml-org/whisper.cpp) | ASR C/C++ alternativo | MIT no projeto; conferir modelos e RAM do alvo |
| [Media3 network stacks](https://developer.android.com/media/media3/exoplayer/network-stacks) | Cache/transferências Android | Apache-2.0 nas bibliotecas AndroidX; usar versão pinada compatível |
| [LoudnessEnhancer](https://developer.android.com/reference/android/media/audiofx/LoudnessEnhancer) | Ganho por sessão Android | API plataforma; disponibilidade depende do dispositivo |
| [Seekr docs](https://seekr.tv/docs) | HTTP `/sprites`, SDK Android0.2.0 opcional | Serviço com quotas/termos; SDK precisa revisão de licença antes de adoção |
| [TypeSafe](https://docs.typesafe.ai/introduction) | Julgamento semântico opcional | API proprietária; typed output não comprova timing |
| [AVPlay](https://developer.samsung.com/smarttv/develop/api-references/samsung-product-api-references/avplay-api.html) | Capacidades/buffer backend | API plataforma; buffer interno não é cache disco |

Jev pode priorizar referências por trecho/metadados, mas não calcula offset nem substitui validação temporal. Começar em modo observação e comparar com regras locais. Sem ganho medido, remover do caminho crítico. Credencial fica no backend; enviar trechos mínimos sem URL/headers privados. ElevenLabs forced alignment é alternativa externa quando texto corresponde à fala, não solução direta para tradução. APIs externas não foram chamadas nesta tarefa.

## Sequência e critérios de liberação

1. Separar preferências aparelho/conta, quota Seekr e diagnóstico; migração preserva comportamento atual.
2. Seletor simples, documentos independentes e partição estável de fontes com diagnóstico.
3. AutoSync offset uma língua no host, depois extração em cada backend; segunda língua em seguida.
4. Android cache/boost; VAD e ASR opt-in somente depois da prova de PCM.
5. Provas LG/Samsung; habilitação por capability e não só macro da plataforma. Jev somente após avaliação comparativa.

Benchmark em LG C9, Android TV/TCL identificado por modelo/SoC/build, Samsung identificada e WGT/TPK separados. Usar mesmos arquivos de teste legais, hashes, rede controlada, builds pinadas, execução fria/quente, pelo menos10 repetições por cenário; reportar mediana/p95 e número de falhas. Não agregar plataformas para esconder regressão.

Corpus temporal: offsets ±0,25/1/5/30s; edição incompatível/corte no meio; escala23,976→25; música, fala esparsa, diálogos repetitivos, forced/SDH; SRT/VTT/ASS embutido; legendas traduzidas e duas línguas com offsets diferentes. Medir erro contra anotação humana, aceites corretos, falsas correções e recusas corretas. Como gate inicial proposto: nenhuma falsa aplicação no conjunto adversarial, p95 erro≤tolerância configurada entre aceites; explicitar tamanho do corpus, pois zero observado não garante zero em produção.

Performance: tempo startup até primeiro frame, seek dentro/fora cache e keyframe, rebuffer por hora, frames/menu p95, pico RAM, CPU, watts se disponível e bytes adicionais. Comparar baseline1.8 sem feature versus cada feature isolada e conjunto. Toda medição deve incluir codec/HDR/passthrough/storage/rede. Escolher o método com menor latência ENTRE os que passam efetividade e não degradam playback; não premiar correção rápida e errada.

Testes determinísticos necessários: geração/cancelamento; offset independente e manual; quota50 com concorrência/restart/clock rollback/perfil/chave; cache ENOSPC/cancelamento/crash/Range; stable partition com metadados ausentes e chegada incremental; isolamento da mesma conta em duasTVs; rollback otimizador e overrides. Provas físicas: vídeo segue responsivo durante análise, seek realmente melhora sob cobertura, áudio não clipa/muda rota, overlay ASS/segunda língua não colidem.

## Servidores pessoais

Jellyfin, Emby e Plex devem compartilhar negociação de capacidades, bitrate, idiomas e progresso com este player: [pesquisa e sequência](../media-servers-1.8/README.md). Servidores são um escopo futuro separado; ainda não implementados.
