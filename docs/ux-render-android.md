# Renderização e movimento da interface no Android TV

**Estado:** proposta técnica para avaliação; não altera o renderer.

**Inspeção do repositório e fontes Android consultadas:** 2026-10-02.

**Escopo:** Android TV / Android com controle remoto no Nuvio Native. Samsung Tizen e LG webOS têm documentos e limites próprios.

Este documento propõe como elevar a sensação de acabamento da interface — foco, tipografia, movimento e composição — sem transformar a TV em uma página mobile nem pressupor que toda Android TV tenha a mesma GPU ou frequência. Os contratos Android abaixo estão ligados ao pipeline C/Kotlin encontrado no repositório. Recomendações e hipóteses estão identificadas separadamente de fatos do código e de medições.

## O que existe hoje no repositório

| Área | Evidência no código | Limite do que se pode concluir |
|---|---|---|
| Aplicação e contexto | `NuvioActivity` é uma `SDLActivity`. O C solicita contexto OpenGL ES 2; o CMake liga SDL2 2.30.9, SDL2_image, SDL2_ttf, GLESv2 e EGL. O Manifest exige GLES 2.0, declara Leanback como opcional e orientação horizontal. | Isso define o mínimo declarado; não revela a taxa da tela, o driver, a resolução efetiva ou o desempenho do dispositivo. |
| Ciclo do quadro | `src/main.c` processa `SDL_PollEvent`, atualiza a app, desenha e chama `SDL_GL_SwapWindow`. No Android, o swap interval é solicitado como 1. Registra tamanho da janela e do drawable, renderer/versão GL e fases do quadro; linhas lentas aparecem nos registros `[quadro]`/`[qd]` e contadores relacionados. | `SDL_GL_SetSwapInterval(1)` é um pedido de sincronização, não prova de pacing uniforme nem de apresentação em cada VSync. Não há uso encontrado de Android `Choreographer` ou AGDK Swappy. |
| Coordenadas e resolução | A interface usa canvas lógico de 1920×1080 (`layout.h`). O caminho Android pode pedir uma superfície 4K; no arranque são consultados `SDL_GetWindowSize` e `SDL_GL_GetDrawableSize`, e o tamanho real é registrado. | A resolução lógica não comprova a resolução do painel ou a concessão de uma superfície 4K. Validar o valor real por aparelho. |
| GLES e composição de vídeo | `NvPlayer.kt` cria um `SurfaceView` para o Media3/ExoPlayer. A `SDLSurface` é colocada como media overlay translúcido sobre essa superfície. O C usa framebuffer com alpha e `gfx_furo` para abrir a região transparente que deixa o vídeo de trás aparecer. | O Android pode compor camadas de maneiras distintas conforme dispositivo, formato e sistema. A documentação de `SurfaceView` descreve o modelo da superfície, não prova que o aparelho concedeu overlay dedicado nem qual foi o custo desta composição. |
| Renderizador UI | `src/gfx.c` oferece modos GLES para cards, texto, sombras, véus, fundos, blur e outras primitivas. Um shader compartilhado atende diversos modos; não é um compositor declarativo Android nem Compose. O furo de vídeo é uma limpeza localizada do alvo com alpha, não um quad de vídeo desenhado pelo Nuvio. | A existência de efeitos não significa que cada tela os use, nem que seu custo seja medido em Android. |
| Texto e cache | `src/text.c` usa SDL_ttf/FreeType, cache de até 512 linhas rasterizadas, cache de fontes vivas e rasterização de texto limitada por quadro/tempo. `escalaTxt` permite rasterizar na escala do drawable enquanto medidas continuam no canvas lógico. | Comentários de custo e legibilidade incluem medições em aparelhos LG/Tizen; não devem ser apresentados como números Android. Em 4K o cache de glifos maiores também usa mais memória de textura. |
| Controles e eventos | Entrada chega pelo SDL. O laço consome eventos antes de atualização/desenho. O Android possui tratamento para retornar do segundo plano e ponte Kotlin para eventos do player. | A inspeção do pipeline não demonstra latência de ponta a ponta do controle Bluetooth/IR até o pixel visível. |
| Perfil adaptativo da GPU | `src/gpunivel.c` habilita leitura do perfil salvo, adaptação automática e `gpun_preferencia()` tanto em `NV_TPK` quanto em `NV_ANDROID` (exceto build com `NV_TPK_NIVEL_FORCADO`). `main.c` também alimenta a medição nos dois alvos. Na home cheia, o código compara FPS e espera (`glClear` + `swap`) com trabalho de CPU; pode reduzir efeitos de 0 para 1 e, se ainda ficar abaixo do limite crítico, de 1 para 2. O nível 3 (desenho interno em 720p) não é automático: `gpun_forcar_720()` o ativa por preferência explícita. A tela de Ajustes já expõe “Efeitos visuais” em Android. | Isto confirma a política existente no código, não sua qualidade ou adequação em qualquer Android TV. A chave persistida inclui renderer/versão GL, modelo e Tizen, mas a coleta de modelo/Tizen mostrada ali é exclusiva de `NV_TPK`; em Android esses campos ficam sem identificação específica de aparelho. O log `[perfil]` também é exclusivo de TPK. Os limites e a persistência entre aparelhos Android ainda precisam ser validados. |
| Movimento reduzido | A configuração compartilhada `ajustes_animacoes_reduzidas()` já é consultada em vários caminhos da UI. | A cobertura deve ser auditada por fluxo antes de afirmar que todas as animações do Android seguem a preferência. |

Referências do caminho: [main.c](../src/main.c), [gfx.c](../src/gfx.c), [gfx.h](../src/gfx.h), [text.c](../src/text.c), [gpunivel.c](../src/gpunivel.c), [video_android.c](../src/video_android.c), [NuvioActivity.kt](../android/app/src/main/java/space/nuvio/nativelegacy/NuvioActivity.kt), [NvPlayer.kt](../android/app/src/main/java/space/nuvio/nativelegacy/NvPlayer.kt), [AndroidManifest.xml](../android/app/src/main/AndroidManifest.xml), [CMakeLists.txt](../android/app/src/main/cpp/CMakeLists.txt), [layout.h](../src/layout.h).

## Proposta de qualidade visual

“Apple-style” aqui significa uma interface deliberada: hierarquia simples, tipografia consistente, foco legível, movimento curto e coerente, transições com início e pouso claros. Não significa copiar controles de toque, gestos ou convenções exclusivas de tvOS. Para Android TV, o D-pad, foco explícito e leitura à distância são contratos centrais.

### Foco e navegação

- Manter um único estado de foco real por camada. O foco deve ser visível antes da ativação; um cartão não deve executar sua ação ao receber foco.
- Dar feedback de foco no primeiro quadro disponível: contorno/halo ou realce local e uma escala pequena são suficientes. Evitar combinar escala grande, deslocamento, brilho e sombra no mesmo foco.
- Garantir que toda ação visível seja alcançável por D-pad, com vizinhanças previsíveis, sem saltos diagonais surpreendentes. `Back` fecha primeiro a camada mais interna e retorna ao mesmo item de origem sempre que essa origem ainda exista.
- Em busca, listas e modais, preservar seleção ao atualizar conteúdo. Se o item focado desaparecer, escolher um vizinho determinístico e manter o indicador visível.
- A animação pode enriquecer o foco, mas a diferença entre focado e não focado também deve ser reconhecível em movimento reduzido, baixa taxa ou imagem parada.

O Android TV recomenda navegação previsível, suporte completo ao controle direcional e um sistema de foco inequívoco. As referências estão em [Design for TV](https://developer.android.com/design/ui/tv/guides/foundations/design-for-tv), [Navigation on TV](https://developer.android.com/design/ui/tv/guides/foundations/navigation-on-tv) e [Focus system](https://developer.android.com/design/ui/tv/guides/styles/focus-system).

### Movimento e resposta à entrada

- Separar animação de estado funcional: atualizar seleção/ação primeiro, animar a apresentação até o novo estado. Tecla repetida não deve acumular tweens atrasados; deve convergir para o alvo mais recente.
- Usar duração e curva consistentes por família de interação: foco curto, mudança de seção mais longa, modal com entrada/saída simétricas. Começar sem overshoot pronunciado; validar num painel grande e a distância.
- Basear progresso em tempo decorrido, limitar `dt` após suspensão e restaurar no estado final se houve perda prolongada de foco/atividade. `main.c` já limita o `dt` de animação a 100 ms, mas esse limitador não garante frame pacing.
- Respeitar movimento reduzido em transições de foco, zoom/parallax, blur animado, entrada/saída de página e efeitos ambientes. Manter feedback essencial instantâneo por contorno, contraste, ícone e texto.
- Evitar animação decorativa contínua quando não existe mudança de estado. Não animar todo o card quando uma borda local resolve a comunicação.
- Acknowledge de OK/Back/direcional deve ocorrer sem espera de rede, decodificação de arte ou chamada síncrona ao player. As atualizações JNI do player devem permanecer assíncronas no caminho UI; nenhuma espera por estado Media3 pode bloquear o loop SDL.

Essa proposta precisa ser validada para os controles e TVs suportados. Documentação de frame pacing não substitui a medição de atraso de entrada.

### Layout e tipografia a três metros

- Projetar primeiro para a área segura e o drawable que o Android realmente concedeu. Evitar informação essencial colada aos limites ou dependente de resolução física presumida.
- Reusar a grade 1920×1080, escalas e estilos tipográficos existentes até que captura em dispositivo mostre um problema. Aumentar legibilidade pela hierarquia, largura de linha, contraste e espaçamento antes de adicionar superfícies ou sombras.
- Preferir títulos concisos, limites de linhas explícitos e truncamento com expansão/detalhe. Não encolher o tipo para fazer caber títulos longos.
- Reservar peso/contraste para estados de foco e informação primária. Texto auxiliar precisa continuar distinguível sem virar microtexto, especialmente quando canvas lógico de 1080p é ampliado para um drawable 4K.
- Validar layout para 720p, 1080p e drawable 4K quando disponível, escalas de texto, traduções longas, safe areas reais e distâncias diferentes. Não inferir densidade de TV pela resolução do painel.

O guia oficial [Typography for TV](https://developer.android.com/design/ui/tv/guides/styles/typography) recomenda tipografia legível à distância e texto simples. A página é guia de design; os limites precisos de fonte/safe-area do Nuvio devem ser provados no dispositivo e na captura real.

### Renderização, texto e orçamento de GPU

Proposta de implementação futura, condicionada por perfil:

1. **Preservar o canvas lógico e o raster de texto na escala-alvo.** Manter `escalaTxt` e cache por chave atual; medir memória e falhas de cache no drawable real. Ao trocar família, idioma, escala ou renderer/contexto, invalidar as texturas dependentes sem rerasterização a cada quadro.
2. **Evitar churn de texturas.** Preparar texto e arte durante a janela de transição quando possível, mantendo o trabalho incremental do `TXT_POR_QUADRO`/orçamento existente. Não aumentar esse orçamento com base apenas em resultado Tizen: no Android, capturar custo de rasterização e atraso visual do texto antes de escolher.
3. **Controlar overdraw e preenchimento.** Reusar primitive batching e shader modes existentes. Limitar fundos de tela cheia, múltiplos véus sobrepostos, sombras grandes, blur em tempo real e camadas translúcidas. Preferir arte já preparada/cacheada ou aproximação estática quando ela comunica igual.
4. **Manter 720p como escolha explícita até validar qualidade.** O código atual limita a adaptação automática aos níveis 0–2; o nível 3 reduz o desenho interno para 720p e só é ativado por `gpun_forcar_720()`. Pode borrar texto. Antes de propor uma política automática de resolução, medir gargalo, leitura e foco em painel real.
5. **Validar o fallback adaptativo já existente por aparelho.** O código consulta renderer para marcar GPUs consideradas fracas e também mede espera/CPU na home; não há ainda evidência Android nesta tarefa de que os limiares e a chave salva distinguem corretamente aparelhos e firmwares. Preserve tamanho/contraste do texto, navegação e feedback de entrada; se a carga for CPU/rasterização, cortar camadas GPU não resolve.
6. **Evitar estado GL caro ou inválido.** Agrupar por textura/programa, reduzir binds e mudanças de blend, usar descarte de attachments somente quando extensão/API e vida útil do alvo confirmarem que seu conteúdo não será lido. O código existente detecta GLES 3/extensão em alguns descartes; novas passadas precisam seguir essa verificação.

O framework Android view pode usar aceleração de hardware, mas este UI é GLES nativo por SDL. As recomendações Android gerais de hardware acceleration não se convertem diretamente em APIs para `gfx.c`. Usar a doc oficial [Hardware acceleration](https://developer.android.com/topic/performance/hardware-accel) como contexto sobre camadas/memória, não como instrução de migrar a UI para View.

## Vídeo e compositor: preservar a arquitetura até provar outra

O player apresenta vídeo via `SurfaceView` do Media3 e a UI GLES fica acima, com região alfa aberta para revelar a superfície de baixo. Essa composição permite UI sobre vídeo sem enviar os pixels decodificados de volta para a textura do Nuvio. A documentação Android descreve `SurfaceView` como superfície separada; em certas configurações isso pode permitir composição/overlay dedicado. Hardware overlay é uma possibilidade do sistema, não uma garantia por modelo nem por chamada.

Recomendações:

- Preservar `SurfaceView` como caminho padrão de vídeo. Não substituir por `TextureView` ou copiar quadro decodificado à GPU sem requisito concreto e comparativo de energia, latência, cor/HDR, legendas, resize e estabilidade em dispositivos-alvo. Media3 documenta diferenças de consumo e recomenda `SurfaceView` em muitos casos; comportamento continua sendo dependente do aparelho.
- Preservar o alpha real do SDL surface e validar o hole após qualquer mudança de framebuffer, FBO, resolução interna, full-screen pass ou troca de orientação. Se alpha ficar opaco, o vídeo pode sumir apesar de o player continuar ativo.
- Evitar efeitos de tela inteira por cima do vídeo, blur de conteúdo ao vivo e camadas translúcidas desnecessárias. Testar controles/legendas/menus com vídeo, HDR e mudança de tamanho de superfície.
- Tratar a composição como três componentes mensuráveis: decoder/SurfaceView; UI GLES/alpha; compositor/display. Um quadro lento ou preto não identifica qual desses componentes falhou.
- Para experiência de “mini player”, diferenciar imagem estática de vídeo ao vivo. Não prometer PIP ou áudio de fundo com base na existência da superfície de vídeo; Android PiP, permissões/manifest, comportamento do player, foco e retorno são um projeto separado.

Referências oficiais: [SurfaceView](https://developer.android.com/reference/android/view/SurfaceView) e [Media3 ExoPlayer battery consumption](https://developer.android.com/media/media3/exoplayer/battery-consumption). A última descreve tendências gerais (por exemplo, custo possível de `TextureView`) e ressalta o caráter específico de suporte e desempenho do dispositivo; não é benchmark do Nuvio.

## Pacing e desenho de fallback

`SDL_GL_SetSwapInterval(1)` e `SDL_GL_SwapWindow` são o mecanismo atualmente visível no loop. Android apresenta conteúdo em deadlines sincronizados; buffers que chegam fora da janela podem repetir o quadro anterior. Android Frame Pacing (Swappy) fornece uma rota OpenGL que substitui `eglSwapBuffers` por `SwappyGL_swap` e recebe janela/período de refresh. Como Nuvio delega janela/contexto/swap ao SDL, integração direta exige uma prova de arquitetura SDL/EGL e do SurfaceView de vídeo. Não passar a chamar Swappy em paralelo ao SDL sem estabelecer propriedade e ciclo de vida do contexto.

`Choreographer` sincroniza callbacks a um Looper. A documentação prevê callback de frame para thread GL separada, mas Nuvio hoje é um loop C/SDL; usar o callback exigiria coordenar o thread do loop sem duas agendas concorrentes, lidar com suspensão/context-loss e não bloquear o Looper principal/Kotlin. Primeiro descobrir se o SDL Android já agenda adequadamente no alvo e medir deadlines perdidos. Não introduzir `Choreographer` como chamada JNI superficial.

Fallback recomendado após identificar a causa:

| Gargalo confirmado | Mudança proposta | Preservar |
|---|---|---|
| GPU/fill-rate | O perfil Android existente já pode reduzir efeitos dos níveis 0–2 após medir a home. Se isso não resolver, investigar efeitos ambientes caros, blur, sombras extensas, camadas fullscreen e parallax antes de criar outro degrau. | Tamanho de texto, foco explícito, contraste, ação imediata. |
| CPU no thread SDL | Diminuir trabalho de atualização e geração de geometria/textura por quadro; manter tarefas de rede/media fora do loop. | Ordem de eventos, consistência do D-pad, estado correto. |
| Rasterização/cache de texto | Reusar linha/fontes, pré-aquecer textos conhecidos sem criar pico, observar cache eviction e chamadas SDL_ttf. | Resolução adequada do glifo e leitura de títulos. |
| Compositor/alpha/vídeo | Manter SurfaceView e reduzir cobertura alfa sobre ela; avaliar formatos e superfície no dispositivo. | Legendas, quadro de vídeo, foco dos controles. |
| Refresh baixo/variable refresh ou deadline | Medir período real e pacing em plataforma; investigar SDL/Swappy/Choreographer como spike. | Não confundir animação mais lenta com input lento. |

Um limite de 30 fps não deve ser ativado como fallback genérico: pode estabilizar trabalho quando o hardware não sustenta 60, mas reduz a frequência de resposta percebida do controle e pode não combinar com refresh de 50/60 Hz. Definir a decisão pelo refresh efetivo e teste de percepção. Não codificar ou forçar taxa de refresh sem consultar a [orientação Android sobre frame rate](https://developer.android.com/media/optimize/performance/frame-rate) e validar o efeito no aparelho, no vídeo e na navegação; Android TV pode ter trocas de modo com custo visível.

## Medições existentes, evidência ausente e plano

**Instrumentação existente no repositório:** dimensão window/drawable e versão/renderer GLES no arranque; cronômetro do quadro e repartição de fases no `main.c`; registros de quadros lentos; contagem/tempo de raster de texto. Estes valores descrevem o trabalho do processo e o tempo de retorno de operações GL/swap, que pode incluir espera indireta pelo compositor. Não são sozinhos timestamp de apresentação em tela, medida de VSync perdido ou latência de controle.

**Evidência de desempenho Android ausente nesta tarefa:** não executei em TV, box nem aparelho Android; não há FPS, p95/p99, input-to-photon, consumo térmico/energia, frames dropped do decoder, superfície concedida, hardware-overlay efetivo ou consistência sob vídeo medidos aqui. Comentários medidos em C9/Tizen são fatos daquele alvo e não previsão de Android.

**Plano de benchmark para a proposta:**

1. Selecionar aparelhos físicos de entrada, intermediário e alta capacidade que sejam alvos reais, incluindo ao menos um Android TV integrado e um box se ambos fizerem parte do suporte. Registrar fabricante/modelo, Android/firmware, build/APK/commit, refresh reportado, densidade/resolução da janela, drawable, `GL_RENDERER`/versão e modo de vídeo. Emulator serve para fluxo funcional, não para aprovação de desempenho de GPU/compositor.
2. Repetir com cache frio/quente e animação normal/reduzida: inicialização; home carregada; foco rápido em fileiras; scroll; busca/teclado; abertura/fechamento de modal; troca de página; troca de tema; churn de pôsteres/texturas; pausa/retorno do app; reprodução em janela completa e controles sobre `SurfaceView`; redes lenta/ausente quando relevante.
3. Coletar telemetria existente e Perfetto/System Trace com FrameTimeline/SurfaceFlinger, thread SDL, escalonamento, renderização e eventos de input. Correlacionar a mesma sequência de tecla com o momento que o foco fica visível. Em reprodução, coletar dropped/late frames, primeira imagem, timestamps do player e mudanças de superfície.
4. Reportar taxa de quadros apresentada e missed/deadline frames por intervalo, distribuição de duração (p50/p95/p99 e pior), atraso input→feedback, fase/causa dominante, cache/rasterização, memória de textura e comportamento térmico após regime estável. Anotar se medida veio do processo, Perfetto ou percepção; não misturar FPS do loop com FPS do painel.
5. Comparar baseline com cada mudança isoladamente. Derivar os limites de aceitação da taxa de refresh e dos dispositivos suportados, exigindo que navegação e texto não regridam. Fixar metas numéricas só depois de captar baseline/referência em cada faixa de hardware.

As recomendações oficiais apontam Perfetto e FrameTimeline para investigar frames/jank e Macrobenchmark para percursos de UI em dispositivos reais. O benchmark automatizado pode guiar uma sequência de controle e abertura; não substitui prova de input-to-photon, vídeo real ou uma TV específica. Ver [Rendering performance](https://developer.android.com/topic/performance/vitals/render), [Rendering overview](https://developer.android.com/topic/performance/rendering), [System tracing](https://developer.android.com/topic/performance/tracing), [On-device system traces](https://developer.android.com/topic/performance/tracing/on-device) e [Benchmarking overview](https://developer.android.com/topic/performance/benchmarking/benchmarking-overview).

## Critérios para levar uma mudança a código

1. O cenário e o alvo Android são definidos; a limitação foi reproduzida no aparelho físico com build e configurações registrados.
2. A métrica distingue loop SDL, espera GPU/compositor, decoder, apresentação e resposta do input; uma captura estática não vale como medição de fluidez.
3. A mudança é testada isoladamente e preserva foco D-pad/Back, movimento reduzido, legibilidade e camada de vídeo transparente.
4. Perfil lento melhora sem redução de legibilidade; perfil rápido não recebe efeitos extras que causem custo ou inconsistência sem intenção de produto.
5. A política não depende só da marca/modelo comercial: consulta capacidade/renderer e mantém fallback seguro quando dados faltam.
6. Build Android, ensaio de dispositivo e limite de suporte ficam registrados separadamente. Esta proposta não é evidência de pacote, instalação ou funcionamento em TV.

## Fontes Android consultadas em 2026-10-02

- [Android Frame Pacing library](https://developer.android.com/games/sdk/frame-pacing/) e [OpenGL functions](https://developer.android.com/games/sdk/frame-pacing/opengl/add-functions) — sincronização/pacing, Swappy e interface EGL.
- [Choreographer API](https://developer.android.com/reference/android/view/Choreographer) — callbacks ligados a frame e Looper.
- [SurfaceView API](https://developer.android.com/reference/android/view/SurfaceView) e [Media3: ExoPlayer battery consumption](https://developer.android.com/media/media3/exoplayer/battery-consumption) — semântica de superfície e trade-offs de saída de vídeo.
- [Design for TV](https://developer.android.com/design/ui/tv/guides/foundations/design-for-tv), [Navigation on TV](https://developer.android.com/design/ui/tv/guides/foundations/navigation-on-tv), [Focus system](https://developer.android.com/design/ui/tv/guides/styles/focus-system), [Typography for TV](https://developer.android.com/design/ui/tv/guides/styles/typography) — layout a distância e interação de TV.
- [Rendering performance](https://developer.android.com/topic/performance/vitals/render), [Rendering overview](https://developer.android.com/topic/performance/rendering), [System tracing](https://developer.android.com/topic/performance/tracing), [On-device system traces](https://developer.android.com/topic/performance/tracing/on-device) — jank, tracing e inspeção de frame.
- [Benchmarking overview](https://developer.android.com/topic/performance/benchmarking/benchmarking-overview) — desenho de benchmark e dispositivos físicos.
- [Frame rate](https://developer.android.com/media/optimize/performance/frame-rate) e [Adjust display settings on Android TV](https://developer.android.com/training/tv/playback/adjust-display-settings) — reporte de frequência do conteúdo e tratamento de refresh no vídeo.
- [Hardware acceleration](https://developer.android.com/topic/performance/hardware-accel) — contexto geral sobre camadas e memória, sem pressupor que API View se aplique ao GLES SDL.
