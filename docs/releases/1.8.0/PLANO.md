# Plano de fechamento da 1.8.0

03/10/2026. Plano executável de integração, correção, desempenho, validação e entrega. A versão pública atual é [1.7.4](https://github.com/iqui27/nuvio-native-legacy/releases/tag/v1.7.4). Este documento não declara a 1.8 pronta. Substitui o agrupamento de versões do roadmap de 02/10; aquele arquivo continua como histórico.

## Estado real da base

- Branch de trabalho: `codex/integration-180-glass`, fora do checkout principal do dono.
- Merge Glass + master publicado: `b98bd2f5`. Agenda mensal integrada pela outra conversa: `e6885991`. Alterações adicionais de vários agentes ainda estão no índice/working tree.
- Glass, correções da 1.7.4, escala própria de Ajustes, menus, ilha, player, Onde assistir e parte do Social já estão na base. Isso não elimina a validação integrada e física.
- PR [#230](https://github.com/iqui27/nuvio-native-legacy/pull/230) já foi merged em `66af5817` e está nessa base. Falta reteste de seleção de faixas no Samsung; não fazer novo merge da mesma PR.
- AutoSync tem motor temporal e documentos testados, mas ainda falta ligação ao player, referência embutida independente e segundo renderer. Não está disponível ao usuário só por existir esse código.
- Seekr tem chave pessoal e limite de 50 chamadas `/sprites` por instalação/dia, compartilhado por contas/perfis/chaves. Faltam mensagens de estado/uso, diagnóstico e tratamento do Retry-After no WGT. Reinstalar/limpar dados remove a proteção local; enforcement após isso exige backend.
- #228: correções reproduzidas de fallback e estados nativos; #158: legendas e CW local reproduzidos e corrigidos. #233 está em implementação. Stream-fit tem engine/folha com testes focados, mas integração de diagnóstico/runtime/rede e telemetria passiva Android ainda está pendente.
- Logs novos do worker: acesso bloqueado por Cloudflare D1 Error7403. Vaultwarden CLI não autenticado. Nenhum desses erros identifica a causa do relato do usuário.
- Não houve pacote final 1.8, publicação ou deploy do backend Social desta integração.

O [inventário de merges](INVENTARIO.md) discrimina branches já ancestrais, patches diferentes e alterações não commitadas. Quantidade de commits diferentes não significa quantidade de features novas.

## Paralelismo e escolha dos modelos

O objetivo é usar contexto curto e resultado verificável. Até três executores em paralelo neste chat, além do coordenador; o chat Android é uma tarefa independente autorizada pelo dono. Cada tarefa recebe arquivos de responsabilidade, contratos existentes, comando de teste e critério de conclusão. Agentes não revertem trabalho alheio.

| Trabalho | Modelo / esforço | Uso concreto |
|---|---|---|
| APKs, inventário, notas, traduções e adaptação simples de fixtures | **gpt-6-luna**, low/medium | Checklist fechado; preserva assinatura, dados e versões; escala para Sol se houver defeito de runtime |
| Implementação comum C/Android, UI sobre o design existente, testes e correções reproduzidas | **gpt-6.1-sol**, medium; high quando houver concorrência | Execução por módulo e validação focada |
| Contratos de identidade Social, isolamento de plugins, cancelamento P2P, confiança AutoSync/áudio | **gpt-6-astra**, high | Revisão pontual dos riscos e decisões difíceis; a implementação volta para Sol |
| Fechamento de integração | Coordenador + Sol para revisão independente | Dependências, conflitos, evidências e pacote do mesmo commit |

Não enviar o histórico inteiro a cada executor nem pedir a vários agentes a mesma investigação. Reutilizar resultados. Luna não resolve corrida/threading sozinho sem revisão. Astra não será usado para tradução, geração de notas ou inventário mecânico. Não estimar custo em dólares sem consumo/preços disponíveis; controlar a despesa pelo tamanho e número das tarefas.

## Sequência e entregáveis

As ondas organizam a execução; funcionalidades ainda pendentes não são automaticamente retiradas do escopo pedido. Uma capacidade que não funciona em certo backend precisa ser implementada/provada ou ter sua limitação explicitada antes de qualquer anúncio.

### Onda 0 — preservar, inventariar e consolidar

| ID | Tarefa | Modelo | Entrega / condição de conclusão |
|---|---|---|---|
| G01 | Conferir branches, equivalência de patches e alterações soltas, incluindo checkout principal e worktrees antigas | Luna + revisão Sol | Manifesto `origem → arquivos → integrado/duplicado/substituído/pendente → testes`; nenhum trabalho apagado |
| G02 | Consolidar os agentes atuais em commits por responsabilidade | Sol/coordenador | #158, #228, #233, Seekr, AutoSync e layout em commits revisáveis; resolver sobreposições em `home.c`, `descoberta.c`, `streams.c`, rede e traduções |
| G03 | Importar somente os deltas úteis de ícones, Unicode do Guia, Biblioteca, documentação Social e outras worktrees | Sol | Confirmar que o comportamento ainda falta na base; evitar substituir módulos novos por versões antigas; testar cada delta |
| G04 | Registrar uma fonte congelada para cada build de teste | Luna | Commit/tree, lista de features incluídas/excluídas, pacote/hash, assinatura, alvo e build instalado |

A build Android intermediária foi instalada pelo chat Luna no Smart TV Pro/Android14, preservando dados; versão1.8.0/code10800 confirmados no aparelho. Snapshot `38b0efb0` inclui os hunks estáveis do índice sobre `e6885991`; exclui os WIP #233, #158 e stream-fit existentes no momento da captura. Versão de teste não equivale a release publicada. Essa build serve ao teste do dono, não é a candidata final desta sequência. A correção mínima de comentário em socialvis.h descoberta pelo build Android701d4113 foi portada à integração; a mudança de versão local098cd245 permanece somente na cópia de preview.

### Onda 1 — bugs, logs e estabilidade da base

| ID | Tarefa | Modelo | Entrega / condição de conclusão |
|---|---|---|---|
| B01 | **#233 — conta, coleções, ordem e editor de fileiras** | Sol/high | Snapshot válido vazio remove coleções; erro/RPC vazio não apaga estado; Home e editor exibem os mesmos IDs/ordem/hidden; excluir não promove fileiras aleatórias; coleções não consomem o limite de catálogos; publicação concorrente segura |
| B02 | **#228 — trailers Home/hero/poster** | Sol + revisão de lifecycle | Testes WGT/TPK e `Prepared → Playing`; fallback usa ordem real e não reabre origem falhada; timeout/pausa/ROI corretos; reteste Samsung com fonte/build identificados |
| B03 | **#158 — subtitles, Flix e Continuar assistindo** | Sol | Todas as origens de legendas consultadas, provider correto; CW funciona offline com IDs IMDb/TMDB/Kitsu/custom e limiares preservados. Diagnóstico de stream distingue pedido não feito/HTTP/vazio/parse/rejeição. Flix exige manifesto/resource/log real antes de atribuir causa |
| B04 | **#223/#211/#188/#197 — arranque, reprodução e catálogo** | Sol/high; Astra somente para causalidade difícil | Reproduzir cenário por aparelho, lifecycle/erro e versão. Corrigir defeito confirmado; conservar como conhecido o caso sem dados. Nenhuma afirmação universal baseada em outra TV |
| B05 | Recuperar acesso aos logs e analisar recorte atual | Luna para coleta sanitizada; Sol para diagnóstico | SELECT somente leitura por horário/plataforma/build; separar upload de sessão, log antigo e tentativa de diagnóstico. Vincular eventos ao problema antes de responder |
| B06 | Melhorar logs novos e relatório de desempenho | Sol | Eventos em inglês com etapa, geração, resultado, HTTP/erro, bytes e duração; startup/seek/rebuffer, memória/cache/disco, Seekr e AutoSync. Nada de chave, header, URL assinada ou corpo privado |

B05 pode seguir em paralelo aos testes reproduzíveis. O bloqueio de D1 não bloqueia toda a implementação, mas impede afirmar que os logs novos foram lidos ou que um fix explica o relato específico.

### Onda 2 — funcionalidades autorizadas da 1.8

| ID | Tarefa / dependência | Modelo | Trabalho que ainda falta |
|---|---|---|---|
| F00 | **Provas pequenas de capacidade por backend e baseline** — antes de F06/F07/F10/F11 | Sol para spikes; Astra só quando necessário | Matriz capacidade/aparelho/proprietário/experimento/evidência: PCM, extração embutida, cache em arquivo/Range/auth, ganho, motor/toolchain e mídia autenticada. Registrar habilitado/indisponível/bloqueado e o trabalho exigido para atender o pedido; não esperar a Onda4 para descobrir impedimento estrutural |
| N01 | **Infraestrutura de rede e jobs por chamada** | Sol/high; revisão Astra curta | Ownership rede.c/h e workers/adaptadores; deadlines/cancelamento/geração, limites de bytes antes de alocar, redirects/headers autenticados, transporte native/WGT. Portar API genérica sem regressão TLS/TPK40. Testes de cancel/timeout/cap/profile/late result; cancelamento impossivel em XHR síncrono deve ser resolvido ou explicitado, sem promessa falsa |
| F01 | **Perfis por TV + diagnóstico** — após B01 | Sol; revisão Astra no isolamento | Separar preferências da pessoa de desempenho do aparelho; UUID de instalação, migração e overrides locais. Mesma conta em Android/Samsung preserva orçamentos diferentes. Diagnóstico mostra cache efetivamente mantido após reteste/rollback; não confundir 300 MB de textura com RAM do processo |
| F02 | **Seekr** — motor já testado | Sol | Integrar estados locais/provedor/chave/rede/storage/relógio no player e Ajustes; contador por dia UTC e apresentação do uso/horário de liberação no fuso local; Retry-After WGT; diagnóstico. Cache/singleflight não gasta consulta repetida; falha após dispatch consome reserva; key pessoal não reinicia 50/dia |
| F03 | **Streams que cabem na conexão + speed test** — em execução | Sol/high | Bytes confiáveis por arquivo e duração real; orçamento conservador do mesmo host/rede com origem/idade. Partição estável dentro da ordem/grupos existentes, desconhecidos conservados; snapshot/foco estáveis. Preservar modo primeira fonte. Alimentar com teste real e transferências Android; host lento não vira diagnóstico de Wi-Fi lento |
| F04 | **Legendas simples + principal/secundária** — após B03/F01 | Sol | Tela inicial idioma + origem (embutida/addon), só idiomas escolhidos; Mais opções para detalhes/versões. Seleções/offsets independentes e chegada tardia sem perder foco; implementar segundo overlay e colisões |
| F05 | **AutoSync de legendas** — depende dos documentos/seletor F04; segunda língua acrescenta segundo renderer | Sol para wiring; Astra para confiança/revisão | Referência embutida completa em documento paralelo, sem trocar a faixa para coletar; sessões e cancelamento. Quick/Thorough, tolerância, desfazer e outra referência. Provar primeiro uma língua e depois duas, sem bloquear o primeiro pipeline pelo segundo renderer. Iniciar filme sem esperar; aplicar somente resultado seguro. Corpus adversarial: nenhuma falsa aplicação observada; p95 do erro residual entre aceites≤tolerância configurada, com tamanho da amostra, recusas e limites relatados. Prova por backend |
| F06 | **Sync por áudio/modelo opcional** — após prova PCM | Astra para contrato; Sol para Android e coletores | Toggle independente; VAD e avaliação ASR local inglês. Download explícito uma vez com tamanho/hash/cancelar/remover. Comparar métodos por latência/efetividade/RAM. Passthrough/offload não é desligado silenciosamente. LG/Samsung precisam demonstração de acesso ao diálogo ou decodificação auxiliar com orçamento |
| F07 | **Seek buffer em disco + volume até200%** — após F00/F01/N01 | Sol Android; Astra só para backends fechados | Android cache de sessão256/512/1024 MB, Range/auth/cancel/ENOSPC/crash/limpeza. Ganho por sessão, tint acima100 e reset; compatibilidade de áudio. LG/Samsung: provar cache/ganho pelo backend; buffer interno de AVPlay não comprova arquivo temporário nem boost |
| F08 | **Social com identidade unificada** — após F01 | Astra contrato; Sol cliente/backend | Social Nuvio independente de login Trakt/Simkl, perfil canônico vinculado por identidade verificada, amigos/atividades/canais deduplicados. Comparações filme/série/gênero/gosto/match com cobertura real; cada progresso ou dado privado/ausente explicitado. Card horizontal, menu fora do card, marcar visto e reação independentes. Revisar backend e migração; deploy ainda pendente |
| F09 | **Plugins — #134** — após N01/F01 | Astra revisão curta; Sol implementação | Portar módulos de `plugins2` seletivamente. HTTP por chamada, limites antes de alocar, orçamento global JS/DOM/filas, prazo/cancelamento, geração de conta/perfil e cache. Snapshot vazio/remover último repo com ACK correto. UI Glass; toolchains/licenças e testes por alvo |
| F10 | **P2P — #171** — após F00/F01/N01 | Astra revisão curta; Sol implementação | Portar `p2p2` seletivamente. Cancelar sem segurar mutex durante metadata/probe, descartar respostas antigas, teto duro de disco/RAM, statvfs falha conservadora, watchdog fora do UI, limpeza só após parar escritores. Empacotar/testar motor real LG/Android/TPK; WGT não tem sockets para motor local |
| F11 | **Jellyfin e outros servidores pessoais** — após F00/F01/N01 | Sol; Astra profile/autenticação | Jellyfin primeiro: conectar/login/biblioteca/detalhes/DirectPlay/progresso. Depois Emby e Plex com contratos próprios. Origem/IDs/usuários isolados, PlaybackInfo/profile do aparelho, HLS/Range/auth, retomada/transcode/check-ins e fim de sessão. Pesquisa pronta, implementação não iniciada |
| F12 | **TopN, ícones e scripts** | Luna para scripts/notas; Sol UI/Android | Conferir pendência real do TopN e custo/paginação. Integrar ícones opcionais sem fechar a tarefa ao mudar alias; tratar #226/#234. Conferir portabilidade Linux/Mac e nomes com espaços. Experimentos de Ajustes antigos não substituem a v2 |

Referências de implementação: [player](../../plans/player-1.8/README.md), [motor AutoSync](../../plans/player-1.8/AUTOSYNC-IMPLEMENTACAO.md), [servidores pessoais](../../plans/media-servers-1.8/README.md). Simkl e Letterboxd não são canais totalmente integrados hoje; acesso/API/contrato precisa ser comprovado antes de prometer cobertura.

### Onda 3 — visual e interação consolidados (em paralelo às ondas1/2)

Responsável Sol, com Luna para traduções e fixtures; usar o desenho aprovado e as correções mais recentes do dono.

**Direção atual, após iniciar a execução:** o dono rejeitou o novo visual dos detalhes e pediu voltar ao layout antigo. Usar `v1.7.4` como referência visual da página de título, preservando os fixes funcionais posteriores, o fan, a navegação e as integrações novas. A restauração substitui U02 e a proposta de redesenho Glass dos detalhes abaixo. Não restaurar o arquivo inteiro sem mapear diferenças funcionais; guardar snapshot antes. As tarefas de consistência, véu e desempenho continuam, agora sobre o visual restaurado. Home, menus e material global seguem suas tarefas próprias.

- Apple TV: **logo → botões diretamente → informações**. Manter uma sinopse curta e legível; retirar o parágrafo gigante e a linha resumida de elenco do hero. Se a arte tem idioma estrangeiro confirmado, nome localizado fica nas informações; sem logo, nome ocupa seu lugar. A seção de elenco da página continua. A imagem enviada pelo dono em 03/10 às 20:35 é referência de conteúdo, densidade e leitura; não desfaz a ordem dos botões nem a prioridade da logo já aprovadas.
- Véu de leitura, texto ampliado ao descer, posição mais baixa no card e detalhe expandido, carrossel bloqueado no modo expandido; fan nasce de Adicionar por foco e clicar ainda adiciona. Restart cresce por foco; duração/replay/restante e temporadas menores.
- Moderna centralizada e20% maior; menu Apple TV com as margens/espaços aprovados. Ajustes80/90/100%, padrão90. Validar navegação/desenho/medição na mesma escala.
- Menu Apple, painéis addon/categorias do Guide, fontes e envio ao amigo respeitam material/opacity/Frost compartilhados; conferir vidro/sólido, contraste, accent claro e movimento reduzido.
- Guia: conferir individualmente herói esquerdo, busca, ajuda em teclas-pílula, véu e painéis citados no handoff; marcar integrado/substituído/pendente com captura e D-pad.
- Discord: preservar vincular/presença/pausa/retomar/troca de perfil/desvincular; o patch divergente do inventário pode ser semanticamente redundante.
- Home: ilha persiste durante carregamento, abre informação útil e mostra conclusão por1s. Medir demora do fornecedor separadamente de fila/parsing/arte/render.
- Player: pausa após seek, blur do próximo episódio e sobreposição com Skip credits; áudio/legenda/erro/loading expandem da ilha conforme design. Todas as entradas de detalhe, inclusive hero/filmografia, usam o mesmo comportamento.

As inconsistências de título/hero e o corte no véu são tarefas explícitas desta entrega:

| ID | Tarefa / responsabilidade | Modelo | Entrega / condição de conclusão |
|---|---|---|---|
| U01 | **Conteúdo consistente entre hero e detalhe** — `home.c`, `detail.c` e helpers de metadados; coordenar hunks com B01/B02 | Sol/medium | Comparar Moderna e Apple TV nas entradas hero Home, card/carrossel expandido, detalhe direto e filmografia. Mesmos dados disponíveis e mesmas ações, respeitando geometria e material de cada layout. Logo primeiro, botões abaixo, metadados/país/direção e sinopse curta depois; nome localizado só nas condições aprovadas. Gêneros, duração, classificação/notas e dados sociais usam uma origem consistente e omitem o desconhecido. Qualidade/codec/quantidade de fontes só aparecem após obter dados confiáveis, sem exemplares fictícios. Testar filme/série, logo localizada/estrangeira/ausente, sinopse ausente/tardia e título longo; o mesmo item não perde sinopse ou ações por mudar a entrada/layout |
| U02 | **Restaurar o layout antigo dos detalhes** — `detail.c/h`, referência `v1.7.4`; substitui o redesenho Glass | Sol/medium; coordenador revisa diferenças | Recuperar composição e geometria antigas do hero/detalhe, botões, sinopse e fundo, comparando capturas com a tag. Preservar fan por foco, restart, texto ampliado/lock do carrossel, duração/replay/restante, retorno/filmografia, retirada de elenco inline e integrações novas. Guardar patch do experimento interrompido. Separar mudanças visuais de correções de dados/lifecycle; conferir filme/série, Moderna/Apple, logo/título longo e episódios. Não inventar outro arranjo na restauração. Capturas host e D-pad físico são evidências distintas |
| U03 | **Linha inferior do véu Apple TV ao descer** — gradientes, recorte e composição da transição hero→conteúdo | Sol/medium | Reproduzir o limite horizontal relatado durante expansão e scroll, comparando fonte/build antiga identificada quando disponível. Conferir término do gradiente, cobertura fora do recorte do card e fundo das seções; causa só após reprodução. Véu contínuo durante a transição, sem linha/retângulo divisório, preservando leitura e artwork. Testar scroll para baixo/cima, card→expandido, vidro/sólido, Frost e movimento reduzido; capturas em pontos intermediários e foto/vídeo da TV com build identificada |
| U04 | **Custo e fluidez das telas** — instrumentação antes/depois de U01–U03, em paralelo a F00/F01 | Sol/high para renderer/concorrência; Luna para coleta repetível | Medir navegação/foco, expansão/scroll/fan, tempo de quadro, uploads/decode de arte, cache, RAM e custo de Frost por plataforma. Reutilizar gradientes/texturas quando aplicável, invalidar cache com contexto/escala/material e evitar blur/decode repetido no UI thread; aplicar apenas a otimização sustentada pelas medições. Sem regressão de legibilidade, foco, latência ou crescimento de memória em ciclos. Ver protocolo da Onda 4; não usar captura estática como prova de fluidez |

Diagnóstico histórico anterior à restauração de `v1.7.4` (não reaplicar automaticamente sobre a composição restaurada): `heroWeb` em `detail.c` suprime a sinopse com `apple || serie`; `desenhaCopiaHero`/`desenhaHero` em `home.c` medem e desenham o texto. Habilitar a sinopse exige ajustar também `heroTopo` e a altura reservada, usando o mesmo corte/medição de texto. O candidato para U03 é `veuLeitura(-scrollY)`: a máscara de filme termina em `1080-scrollY`, e séries combinam gradiente/fundo de outra cor. `gfx_veu_css` tem limite espacial rígido; o carrossel usa outra máscara em `carFundo`. Esses são candidatos para reprodução, não causa física comprovada. Testar scroll inteiro/fracionário, trailer ligado/desligado e render nativo/720p; comparar perfil vertical de pixels e tempo de quadro, evitando acrescentar passadas fullscreen/FBO/blur indiscriminadamente.

U01–U04 entram no fechamento da 1.8. A consistência de conteúdo não exige desenhar todos os layouts com a mesma geometria. Nenhuma dessas tarefas está concluída só porque foi adicionada ao plano.

As capturas `apple-no-cast-*` do host passaram e mostram a retirada da linha de elenco. São fixtures, não fotos da TV ou aceite do dono. A versão1.7.7/10707 instalada anteriormente era etiqueta local de teste.

### Onda 4 — otimizar e provar em cada plataforma

Fazer depois da integração funcional, no mesmo conjunto de mídia/rede. O baseline é coletado cedo em F00; nesta onda repetir na fonte final. Comparar1.7.4 e1.8 sem a feature versus1.8 com a feature isolada e conjunto; registrar hardware, firmware, commit, codec/HDR/áudio/legenda, fonte/host e cache frio/quente. Dez repetições por cenário como protocolo inicial; medir mediana/p95/falhas, sem estimar resultados.

Cobrir três tempos diferentes: **abrir Home→fileiras úteis**, **clicar Reproduzir→primeiro frame com áudio** e **comando no controle→resposta visual**. Para U01–U04 incluir hero Moderna/Apple TV, card expandido, tela de título, menus, fan e descida/subida pelo véu. Registrar distribuição de tempo de quadro, quadros perdidos, decode/upload/cache e memória por ciclo; comparar Frost/vidro/sólido e animações, sem esconder a espera da fonte dentro do tempo de renderização. O diagnóstico deve mostrar orçamento/cache solicitado e efetivamente aplicado, incluindo 128/300 MB e a razão de eventual rollback.

| Plataforma | Medição e investigação | Gates próprios |
|---|---|---|
| **Android TV** | Clique→addons→fonte→URL→prepare→Playing→primeiro frame; lifecycle da Surface, render/layout, Media3 buffers, transferências, cache/boost, heap/RSS/GC | Cold start no aparelho atual e caso Android11/Mali-G31; conservar dados/assinatura. Cache/boost/PCM por rota de áudio; ambas ABIs e compatibilidade de páginas16KB quando houver bibliotecas novas |
| **LG webOS** | uMS/ACB, ROI/legendas, artes/decode, threads/cache de textura, disco livre e motor P2P | Pacote realmente instalado pelo instalador, modo755, início/player/seek/retomada/HDR/ASS/CW. Testar C9 e cenário C4 relatado quando disponível |
| **Samsung WGT** | AVPlay, WASM/IDBFS/localStorage, decode/Frost, menus, chamadas XHR e seleção de faixas | Quota persistente/restart, faixas/trailers/legendas e cancelamento possível; sem motor P2P de sockets; não apresentar backends TPK como prova WGT |
| **Samsung TPK4/5 e6+** | Eventos de prepare/Playing, track settle, ROI, curl/jobs, texturas/memória e motor | TPK40 sem compiler TLS e símbolos incompatíveis; chamadas de host/lifecycle separados de WGT; seleção e fallback na TV alvo. Quatro variantes do pacote verificadas |

Otimizar a etapa que foi medida como lenta: não baixar qualidade/HDR nem aumentar buffers indiscriminadamente. Reteste com a mesma amostra e rollback; escolhas manuais persistem. Exigir ausência de regressão reproduzível em startup/seek/foco e ausência de crescimento sem retorno em ciclos abrir/tocar/parar. Quando não houver aparelho, registrar como sem prova física e manter esse gate visível.

Gate de desempenho inicial proposto: além de crashes/erro funcional, bloquear regressão de mediana ou p95>10% que se repita em duas rodadas comparáveis e exceda a variabilidade medida da baseline. Guardar distribuição/ruído; se inconclusivo, retestar, sem anunciar ganho. Metas de RAM/disco seguem limites reais do perfil/capability, e sua medição não pode exceder teto duro. Esse protocolo pode ser ajustado com evidência antes da comparação, nunca para esconder um resultado.

Testes focados por módulo primeiro. Uma regressão integrada serial após congelar o candidato, com exclusões e falhas preexistentes explicadas. Não repetir a suíte inteira a cada mudança; nova rodada só por alteração relevante/falha/risco não resolvido. ASan/UBSan/TSan nos módulos concorrentes adequados. Teste P2P que SKIPa por falta do motor não conta como PASS do motor.

### Onda 5 — issues, notas, pacotes e envio

Luna prepara notas/What's New, textos de issue e empacotamento. Sol confere que cada frase corresponde ao código/teste. Coordenador integra/publica quando a entrega estiver pronta e dentro do escopo autorizado.

1. Atualizar matriz `issue → causa reproduzida → fix/commit → teste → build → resultado físico → próximo passo`. Responder em inglês com evidência e versão entregue; revisar comentários já enviados na1.7.4 para não repetir promessas.
2. Encerrar somente o cenário corrigido e verificável. Pedidos de produto não atendidos, agradecimentos, portas experimentais e relatos sem dados seguem com status adequado. Não fechar tudo por número.
3. Congelar um commit1.8.0; appinfo/config/Android/TPK e banners concordam. Criar fonte de empacotamento limpa no SSD externo; Mac estava com~3.8GB livres. Assinaturas existentes e conta/addons/log do núcleo são obrigatórios.
4. Gerar2IPKs, APK, WGT e4TPKs, núcleos exigidos pelo updater, `repo.json`, `webosbrew.manifest.json` e SHA256SUMS. Confirmar nomes de atualização, ABIs/símbolos/mode755/certificados, ausência de dados pessoais e hashes.
5. Instalar essa mesma candidata nos alvos, conferir hash/versão/banner e executar smoke/performance. A Android preview intermediária não substitui este pacote final.
6. Finalizar notas em inglês e cartão visual1.8 com só novidades entregues. Instalação nova vê o cartão atual; histórico de marcadores1.7.3/1.7.4 preservado.
7. Revisar mudanças de servidor Social/migração e compatibilidade com clientes antigos antes do deploy; registrar backend usado nos testes. Banco/registros privados não são material de release.
8. Publicar tag/release/anexos e metadados Homebrew do mesmo commit; baixar anexos publicados e conferir SHA256. Confirmar canal de atualização. Vincular correções verificadas às issues e acompanhar regressões concretas.

## Triagem das issues abertas

Consulta GitHub03/10:22 issues abertas; estado pode mudar. [#229](https://github.com/iqui27/nuvio-native-legacy/issues/229) está fechada na consulta atual; conservar testes de rating e não duplicar a resposta antiga.

| Grupo | Issues | Encaminhamento |
|---|---|---|
| Correções imediatas em execução | [#233](https://github.com/iqui27/nuvio-native-legacy/issues/233), [#228](https://github.com/iqui27/nuvio-native-legacy/issues/228), [#158](https://github.com/iqui27/nuvio-native-legacy/issues/158) | B01–B03; prova física/log atual antes de declarar os relatos resolvidos |
| Arranque, player e catálogo | #223, #211, #188, #197 | B04; prioridade por reprodução/severidade, contexto correto por plataforma |
| Recursos já corrigidos/configurados na base, ainda abertos | #232, #231, #227, #209, #202, #216 | Revalidar blur, Cinemeta, idioma, contraste e toque no candidato; responder sem confundir opção existente com correção nova |
| Features1.8 | #134, #171, #226 | Plugins, P2P e ícones com seus gates |
| Pedido novo | #234 | Avaliar em UX ícone/splash, overlay do trailer e botão manual. Não chamar configuração parcial de atendimento completo; overlay/splash precisam decisão de comportamento e prova |
| Observabilidade/feedback UI | #192, #172, #144 | Diagnóstico separado de crash real; mapear feedback aos fluxos visuais/físicos |
| Porta experimental | #135 | VIDAA em matriz própria; não bloquear Android/LG/Samsung nem prometer porte só por merge |
| Informativo | #155 | Agradecimento, sem fix técnico obrigatório |

## Checklist de saída

- [ ] Todos os trabalhos úteis têm destino explícito; checkout do dono preservado.
- [ ] #158/#228/#233 integrados com regressão e resultado atual registrado; Flix sem falsa atribuição.
- [ ] Logs atuais acessíveis ou limitação explícita; P0/P1 reproduzíveis do candidato resolvidos.
- [ ] Features anunciadas ligadas ponta a ponta, com isolamento/cancelamento/orçamento/capability.
- [ ] Social Nuvio não depende de conectar Trakt/Simkl e nenhum campo inventa dados de comparação.
- [ ] Layout aprovado preservado; capturas reais/builds identificados por plataforma.
- [ ] Startup/seek/fluidez/memória medidos; reteste do diagnóstico e rollback corretos.
- [ ] Versão, assinatura, bibliotecas, updater/Homebrew e hashes conferidos.
- [ ] Notas/What's New/issues dizem somente o que foi entregue e testado.
- [ ] Publicação e atualização conferidas após upload; limitações permanecem visíveis.

## Próxima execução

Terminar B01/#233 e os hooks de F03/stream-fit; consolidar B02/#228 e B03/#158 já testados. A Android preview do Luna está instalada, com manifesto próprio; a coleta de logs ainda precisa ser desbloqueada. Em paralelo à integração funcional executar U01–U04: conteúdo/sinopse, continuidade do véu e desempenho medido por plataforma. Depois: UI/diagnóstico Seekr + perfis, Social, legendas/AutoSync e Android cache/boost; plugins/P2P por módulos revisados; media servers/áudio conforme os contratos acima. Congelamento, benchmark e release vêm depois da integração funcional, não apenas depois de compilar.
