# LG webOS: movimento, componentes e renderização

Pesquisa e proposta em **2 de outubro de 2026**, sobre o checkout com base em `2ab3ccc8` e alterações de Ajustes em andamento. Este documento não altera o renderer. O objetivo é uma experiência de TV com a clareza, continuidade e acabamento associados à Apple, preservando as características da LG: Magic Remote, D-pad, webOS antigo, GLES 2 e vídeo nativo separado.

## Evidência e limites

**Verificado nesta pesquisa:** leitura dos caminhos C/SDL/GLES e fontes primárias indicadas abaixo. **Histórico do repositório:** comentários com medições anteriores na C9, sem reprodução nem acesso aos registros brutos nesta tarefa. **Proposta:** técnicas, durações e limites experimentais identificados como tais. Não houve instalação, captura de desempenho ou teste com controle em uma LG nesta pesquisa. Compilar no Mac, mostrar o protótipo HTML ou capturar uma fixture não comprova fluidez da TV.

O [README](../README.md) menciona 60 fps e zero janks na home da C9. Esse registro não fornece sozinho condições, distribuição de tempos nem desempenho das novas telas. A referência C9 é importante para regressão, mas não representa todas as GPUs/firmwares webOS. Não deduzir a cadência do compositor nativo a partir da frequência anunciada do painel ou de uma entrada HDMI.

## Arquitetura que precisa ser preservada

| Componente | Evidência no código atual | Consequência para a proposta |
|---|---|---|
| Janela, desenho e coordenadas | [main.c](../src/main.c), [gl_compat.h](../src/gl_compat.h), [layout.h](../src/layout.h): SDL2, GLES 2 na TV, OpenGL 2.1 no Mac; canvas lógico e drawable real são distintos. `SDL_GL_GetDrawableSize` determina os pixels efetivos. | Usar dimensões concedidas e escala real, sem presumir que solicitar 4K garante um framebuffer 4K. Shaders e extensões precisam de fallback GLES 2. |
| Composição com vídeo | [main.c](../src/main.c) solicita alfa e torna a superfície Wayland não opaca. [video.c](../src/video.c) usa LS2/`com.webos.media`, `libAcbAPI` por `dlopen` e caminho de janela exportada SDL para gerações posteriores. | O vídeo aparece abaixo da UI através da região transparente. Não é uma textura GL disponível para blur ou captura por `glReadPixels`. Respeitar alfa, crop, retângulo e vida útil do player. |
| Laço e pacing | [main.c](../src/main.c) pede swap interval 1 na TV e 0 no Mac; o retorno da configuração não é conferido nesse trecho. Mede etapas com relógio de CPU, limita `dt` visual a 100 ms e conta jank com limiar fixo de 33 ms. | Verificar suporte e medir cadência. A espera de swap inclui sincronização e trabalho pendente; não equivale a tempo puro de GPU. O limiar de 33 ms não detecta todos os deadlines perdidos. |
| Animação | [anim.h](../src/anim.h): aproximação exponencial, mola criticamente amortecida, rampa temporal e resposta de borda; política de movimento reduzido resolve o alvo imediatamente. | Há base reutilizável. A mola atual limita `dt` a 50 ms, zera velocidade contrária ao novo alvo e impede cruzá-lo. Continuidade de velocidade em reversões não é garantida; mudar isso exige comparação perceptiva. |
| Primitivas e efeitos | [gfx.c](../src/gfx.c): shaders de formas, cache de estado, submissão por retângulo, blur separável, fundos combinados, ambientação em FBO pequeno com alternância de alvos. | Otimizar o que existe. Não reintroduzir passadas fullscreen para simular profundidade que uma única superfície material consegue comunicar. |
| Imagens | [tex_cache.c](../src/tex_cache.c), [perfiltv.c](../src/perfiltv.c): decode fora do desenho, upload no thread GL, dimensionamento pela apresentação, prioridades/LRU e proteção de arte visível. `main.c` bombeia até três entradas por quadro. | Quantidade de uploads não limita bytes nem duração. Um upload de fundo pode custar mais que vários pôsteres; observar backlog e custos por recurso. |
| Texto | [text.c](../src/text.c): 512 entradas de linhas, famílias/estilos e fallback, cache de medida e de escolha de fonte, raster conforme escala. Quatro linhas são o **piso**, com orçamento de 24 ms e máximo de 64 linhas por quadro. | Há uma escolha explícita entre pico ao abrir tela e rótulos aparecendo em etapas. A solução proposta é preparar texto previsível antes da entrada, não apenas reduzir o limite e restaurar o aparecimento gradual. |
| Capacidade | [gpunivel.c](../src/gpunivel.c): adaptação compilada para `NV_TPK`/`NV_ANDROID`; LG permanece no nível 0 desse mecanismo. [perfiltv.c](../src/perfiltv.c) define orçamento de texturas por RAM. | LG não possui essa política adaptativa por simples existência do módulo. Extender requer um experimento próprio; RAM total não mede VRAM disponível nem velocidade de GPU. |
| Entrada | [ponteiro.c](../src/ponteiro.c), [main.c](../src/main.c): alvos com IDs estáveis, lista de hit-test e tratamento SDL do Back LG. | Compartilhar estado lógico entre ponteiro e D-pad. Não substituir códigos nativos por keycodes JavaScript retirados da documentação web. |

A documentação pública LG diferencia resolução gráfica de resolução de vídeo: para web apps em UHD, descreve gráficos 1920×1080 e vídeo 3840×2160. Isso contextualiza a separação de superfícies; não documenta o teto do framebuffer deste aplicativo nativo. A dimensão correta continua sendo a obtida no processo. Fontes: [LG — App Resolution](https://webostv.developer.lge.com/develop/specifications/app-resolution), [SDL — GetDrawableSize](https://wiki.libsdl.org/SDL2/SDL_GL_GetDrawableSize), consultadas em 02/10/2026.

Os caminhos ACB/Wayland descritos acima são fatos da implementação do Nuvio, não garantias das APIs públicas LG. Não transferir contratos de webOS OSE ou de um navegador para essas integrações privadas.

## Direção visual: uma gramática para todos os componentes

O foco deve explicar onde o próximo OK atuará; seleção, alteração de valor e reprodução permanecem estados distintos. A referência Apple separa foco e ativação e recomenda espaço para realce do elemento. Aplicar esse princípio a cartões, linhas de Ajustes, tabs, controles e botões, sem copiar restrições de ponteiro de tvOS. Fonte: [Apple — Focus and selection](https://developer.apple.com/design/human-interface-guidelines/focus-and-selection/), consultada em 02/10/2026.

Proposta de contratos compartilhados, a validar em painel e à distância:

| Componente | Comportamento proposto | Custo e fallback |
|---|---|---|
| Cartão | Realce imediato; escala discreta, sombra curta e pouso estável; imagem mantém seu enquadramento. Borda não recorta no limite da fileira. | Transformar geometria, reutilizar textura; no perfil simples, contorno/contraste sem escala ou sombra ampla. |
| Linha de Ajustes/lista | Indicador de foco e valor legível; dependência indisponível mantém explicação. Realce percorre o espaço sem mover o layout inteiro. | Uma superfície de realce; não rasterizar texto de novo porque a linha mudou de posição. |
| Scroll | Seleção lógica acompanha a tecla já recebida; apresentação converge ao alvo mais recente. Repetição não cria fila de animações. | Uma translação e clipping por região; evitar redesenhar conteúdo fora da área visível. |
| Sheet/modal | Entrada curta com hierarquia clara; fundo estabiliza, foco nasce no controle útil; Back fecha a camada interna e devolve a origem. | Material preparado e um véu; sobre vídeo, não exigir captura do plano nativo. |
| Hero | Troca entre duas artes prontas; título e ações permanecem legíveis. Sem iniciar vídeo por deslocamento incidental de foco. | Reusar caminho de mistura já existente; cancelar preparação de artes de alvos superados. |
| Teclado/busca | Digitação e foco recebem feedback antes de resultados remotos. Resultado tardio não move a seleção sem regra determinística. | Texto e geometria estáveis; resultados entram por atualização de dados, sem reconstruir todos os recursos. |
| Player | Controles aparecem sem alterar o enquadramento de vídeo; legendas e foco mantêm prioridade visual. | Preservar região alfa e superfície de mídia, com superfícies locais discretas. |

A biblioteca deve descrever posição atual/alvo, velocidade quando aplicável, opacidade, escala, prioridade e condição de término. Desenho recebe estado resolvido; não dispara rede, escrita ou navegação. Interromper uma transição não deve restaurar posição velha nem ativar callbacks de uma tela encerrada. IDs e geração da tela vinculam o resultado assíncrono à origem correta.

**Hipóteses iniciais de timing:** foco deve começar no primeiro quadro disponível; assentamento de foco em 140–220 ms; sheet em 220–300 ms; troca visual de hero em 280–420 ms. São faixas de comparação, não números Apple nem limites comprovados na C9. Experimentar três variantes dentro da faixa e registrar primeira resposta, pouso e legibilidade. Reversões rápidas, foco em bordas e deslocamentos longos precisam de curvas próprias; uma mola universal não resolve todas as distâncias.

Avaliar continuidade de posição e velocidade separadamente: preservar velocidade ao inverter pode criar afastamento temporário do novo alvo; zerá-la imediatamente pode parecer uma quebra. O contrato prioritário em TV é resposta previsível ao controle. Comparar a implementação atual com retarget de velocidade limitado, sem adotar overshoot expressivo por estética.

### Movimento reduzido e entrada LG

Reutilizar `anim_politica_reduzida`, auditar transições fora de `anim.h` e desativar parallax, zoom, respiração e blur animado. Manter feedback por contraste, contorno, texto ou dissolução discreta quando ela ajudar a orientação. A orientação Apple trata esses efeitos como potenciais gatilhos e recomenda alternativas que mantenham o significado; a preferência local do Nuvio deve funcionar mesmo sem API LG comprovada para a preferência de acessibilidade do sistema. Fonte: [Apple — Reduced Motion evaluation criteria](https://developer.apple.com/help/app-store-connect/manage-app-accessibility/reduced-motion-evaluation-criteria), consultada em 02/10/2026.

LG documenta ponteiro, clique, roda e navegação de cinco vias, inclusive alternância entre os modos. O Nuvio precisa validar todos. Uma regra de interação de Apple TV que dispense cursor não se aplica ao Magic Remote. Fonte: [LG — Magic Remote](https://webostv.developer.lge.com/develop/guides/magic-remote), consultada em 02/10/2026.

No hit-test proposto, regiões de intenção permanecem estáveis durante a escala de foco, enquanto o retângulo visual acompanha a animação. Testar especificamente a fronteira entre cartões para evitar alternância de hover causada pelo próprio crescimento. Ao alternar D-pad/ponteiro, conservar o último foco válido e evitar que um evento de mouse residual capture a seleção. Não atrasar OK ou Back para esperar o fim da animação.

## Renderer: etapas concretas de melhoria

### 1. Preparar o trabalho antes da transição

Criar uma preparação explícita de tela: geometria, textos principais e arte já disponível recebem prioridade; imagens extras continuam incrementais. `txt_pendentes`, `txt_rasterizadas`, `txt_despejos`, bytes de upload e idade da fila mostram se a cena está pronta. Preparar somente a próxima tela provável, com geração/cancelamento e teto de memória, evitando pré-carregar catálogos inteiros.

O orçamento atual de texto de 24 ms já ultrapassa um período nominal de 16,67 ms em 60 Hz, antes de qualquer outro trabalho. Isso é uma observação aritmética do código, não prova de que todo modal custa 24 ms. Medir a abertura fria/quente. Experimento: pré-aquecer rótulos de Ajustes e modal no thread responsável pelas fontes, em pequenas fatias antes da entrada, preservando ação e feedback imediatos. Se dados ainda faltarem, apresentar um estado coerente de carregamento; não deixar controles focáveis sem rótulo.

Preservar Inter e alternativas já suportadas, raster na escala real, pesos e fallback de escrita. Não rasterizar durante cada passo de zoom; usar a textura adequada ao maior tamanho de foco previsto. Atlas de glifos é uma investigação posterior, condicionada a evidência de pressão do cache de linhas: precisa manter shaping, kerning, acentos, scripts complexos, peso e fallback. Não trocar texto por SDF por pressuposto de economia; comparar leitura de corpo pequeno e custo total primeiro.

### 2. Separar camadas por frequência de mudança

Proposta: fundo/material preparado; conteúdo visível; realce de foco; texto/ícones; modal; controles e máscaras do vídeo. Essa separação é de invalidação e responsabilidade, não uma exigência de um FBO por camada. Um FBO fullscreen para cada camada multiplicaria memória e preenchimento.

Reusar blur separável e caches de `gfx.c`. Ambientação atual já utiliza 320×180 e alternância de alvos; um novo painel pode derivar cor/blur da arte estática em 160×90 ou 320×180, conforme experimento. Recalcular ao mudar fonte/tema/tamanho, não em toda oscilação do foco. Se o fundo for vídeo nativo, usar material tonal/escuro derivado de metadados ou arte estática. Blur real do vídeo exigiria outra arquitetura de composição, fora deste plano.

O material aspiracional combina luminosidade local, contraste e uma borda de luz contida. Não requer refração por pixel nem muitas sombras translúcidas. Para reduzir overdraw, desenhar opaco onde semanticamente possível, eliminar véus redundantes, recortar passadas à região útil e evitar duas misturas fullscreen quando um shader existente já combina as fontes. Uma superfície de fallback deve preservar a mesma hierarquia e dimensões.

### 3. Submeter menos trabalho sem quebrar a ordem visual

Medir chamadas de desenho, binds, mudanças de programa/scissor, área coberta e passadas FBO. Experimentar batching de retângulos consecutivos com textura, programa, blend e clipping compatíveis, usando VBO dinâmico compatível com GLES 2. Não ordenar primitivas transparentes globalmente por textura: isso altera a composição. Flush em troca de estado, máscara, alvo ou barreira de ordem. Comparar ganho CPU com aumento de buffer e custo de upload de vértices; nenhum ganho é presumido.

Substituir o limite exclusivo de três uploads por quadro por limite conjunto de quantidade, bytes e tempo observado. Priorizar texto essencial e a arte do foco; descartar pedidos obsoletos por geração, proteger o conjunto visível e limitar superfícies decodificadas aguardando upload. Um prazo suave não interrompe `glTexImage2D` já em execução: reduzir a dimensão do recurso e o trabalho por chamada continua necessário.

## Memória e capacidade: orçamento completo

O cache atual de imagens não deve ser confundido com toda memória gráfica/processual. Criar um inventário local: imagens GL, linhas de texto, FBOs de efeitos, staging de decode, corpos de download, buffers de vértices e recursos pendentes de liberação. Vídeo/driver/compositor podem ter alocações opacas; registrar essa lacuna, junto com RSS e pressão observável. `MemTotal` não fornece VRAM livre.

Para dar escala, um alvo RGBA8 sem mipmaps tem aproximadamente **7,91 MiB em 1920×1080** e **31,64 MiB em 3840×2160**; dois alvos 4K adicionais somam 63,28 MiB antes de alinhamentos e cópias. Um par RGBA8 de 320×180 representa cerca de 0,44 MiB. São contas de dimensão × quatro bytes, não medições do driver; o ambiente atual usa RGB e a implementação pode alocar de outra forma.

Preservar inicialmente a tabela existente de `perfiltv.c`: LG abaixo de 1,2 GB recebe cache automático de 48 MB/teto 64; de 1,2 a menos de 3 GB recebe 128 MB, com tetos distintos; a faixa de 2–3 GB associada à C9 permite teto 300 MB. Esses são limites de política já codificados, não recomendação de aumentar a ocupação. O comentário histórico relata C9 com 2245 MB e uma sessão que usou 35 MB em 44 texturas. Confirmar o uso real do novo fluxo antes de mudar orçamento.

Proposta de capacidades detectadas por sessão: versão/renderer GL, extensões válidas, tamanho máximo de textura, suporte e completude de FBO, precisão de shader, formato efetivo, drawable, RAM e versão do caminho de vídeo. Registrar suporte separadamente de desempenho. Um FBO completo não demonstra que seu custo cabe no quadro.

| Perfil experimental | Aparência preservada | Redução prioritária |
|---|---|---|
| Essencial | Texto nítido, foco evidente, geometria/ações completas e resposta imediata | Material tonal opaco, sem blur novo, sem respiração/parallax; cache pequeno e artes adequadas ao tamanho. |
| Equilibrado | Mesmo layout e tipografia, foco com escala curta, profundidade discreta | Blur pequeno preparado por mudança e sombras limitadas; sem efeitos caros contínuos. |
| Amplo | Continuidade rica de foco, transição de arte e material preparado | Só habilitar variações extras depois de medir margem com vídeo e memória sob carga. |

Escolha inicial conservadora; adaptação futura com janelas estáveis, histerese e uma redução por vez. Não oscilar efeitos durante uma interação. Separar escolha explícita do usuário de degradação temporária; não sobrescrever preferências nem sincronizar capacidade desta TV. Diferenciar falta de GPU, CPU, memória ou I/O antes de reduzir qualidade. Diminuir blur não resolve espera de rede; reduzir resolução não corrige fila de input.

## Frame pacing e medição

Primeiro conferir o retorno de `SDL_GL_SetSwapInterval(1)` e registrar o intervalo consultado; 1 pede sincronização, não uma frequência fixa. Adaptive vsync é opcional e sua disponibilidade não é garantida na LG; não ativar -1 como solução genérica. Fonte: [SDL — SetSwapInterval](https://wiki.libsdl.org/SDL2/SDL_GL_SetSwapInterval), consultada em 02/10/2026.

Instrumentar amostras por frame com sequência de evento, foco lógico, início de feedback, fases CPU, uploads/texto e retorno do swap, com buffers limitados e saída assíncrona. O contador SDL mede tempo de CPU decorrido, não apresentação óptica. Fonte: [SDL — GetPerformanceCounter](https://wiki.libsdl.org/SDL2/SDL_GetPerformanceCounter), consultada em 02/10/2026.

Se houver `GL_EXT_disjoint_timer_query` e entrypoints válidos, usar consultas de GPU em anel, ler apenas resultados disponíveis em quadros posteriores e descartar intervalo disjoint. Sem extensão, manter métricas CPU identificadas como tal. Não inserir `glFinish` no benchmark de produção para inventar uma medida de GPU: ele muda o pipeline. Fonte: [Khronos — EXT_disjoint_timer_query](https://registry.khronos.org/OpenGL/extensions/EXT/EXT_disjoint_timer_query.txt), consultada em 02/10/2026.

Definir `T = 1000 / frequência efetiva` em ms apenas depois de caracterizar a cadência. Para iniciar os experimentos, propor p95 de CPU do renderer abaixo de `0,35T`, p95 de GPU abaixo de `0,60T` quando mensurável, e trabalho incremental de upload/texto em torno de `0,10T`, ajustando ao baseline. Essas parcelas não são somáveis como um pipeline serial: CPU/GPU podem sobrepor trabalho e compositor tem sua própria agenda. Em 60 Hz seriam aproximadamente 5,8 ms, 10 ms e 1,7 ms; não são resultados da C9.

Meta inicial de interação: foco lógico no processamento do evento, feedback submetido em até dois períodos e ausência de cauda crescente ao segurar uma direção. Isso mede evento recebido→submissão, não controle→fóton. Para o último, filmar controle/indicador de acionamento e tela com câmera de alta cadência, repetindo pelo menos 30 ações por cenário e reportando resolução temporal do instrumento. Informar p50/p95, pior caso e amostra; sem afirmar precisão abaixo do intervalo de captura.

O contador atual `dt > 33 ms` permite declarar zero janks e ainda perder quadros em uma apresentação de 60 Hz. Acrescentar histogramas de período, quantidade de intervalos acima de `1,5T`/`2T` e rajadas consecutivas, identificados como aproximação pelo loop quando não houver timestamps do compositor. Média de FPS isolada não aprova a mudança.

### Experimento posterior: redraw parcial

Se EGL e SDL deste firmware permitirem, avaliar `EGL_EXT_buffer_age` com histórico de regiões alteradas e `EGL_KHR_swap_buffers_with_damage`. A primeira permite reconstruir corretamente conteúdo antigo do backbuffer; a segunda informa dano da superfície ao compositor, sem dispensar um backbuffer consistente. Idade zero, resize e invalidação exigem redraw completo. Não substituir a função de swap do SDL nem manter uma segunda agenda de apresentação sem definir propriedade/ciclo de vida de EGL. Fontes: [Khronos — buffer age](https://registry.khronos.org/EGL/extensions/EXT/EGL_EXT_buffer_age.txt), [Khronos — swap with damage](https://registry.khronos.org/EGL/extensions/KHR/EGL_KHR_swap_buffers_with_damage.txt), consultadas em 02/10/2026.

É uma etapa tardia: hero em movimento, transparência e superfície de vídeo tornam a região de dano menos óbvia. Benefício possível em Ajustes estáticos ou foco localizado; verificar artefatos e custo com a região de vídeo aberta antes de habilitar.

## Vida útil e retorno ao app

LG documenta mudanças entre app visível e suspenso para web apps. O renderer C precisa mapear os eventos realmente recebidos pelo SDL/casca nativa; não presumir que `visibilitychange` JavaScript chega ao loop C. Fonte: [LG — App Lifecycle Management](https://webostv.developer.lge.com/develop/guides/app-lifecycle-management), consultada em 02/10/2026.

Proposta: pausar animação decorativa quando oculto, limitar trabalho pendente, invalidar preparação obsoleta e restaurar um estado coerente ao retornar. Tratar separadamente suspensão, perda de foco por overlay do sistema, recriação de drawable e eventual perda de contexto. Reiniciar relógio da apresentação sem executar segundos de animação acumulada. Recriar recursos GL somente quando necessário, preservando dados CPU úteis dentro do orçamento. Testar Home, menu LG, teclado/voz, retorno com vídeo e Back, inclusive durante transição de modal.

## Matriz de experimentos e entrega

| Etapa | Comparação isolada | Evidência para avançar |
|---|---|---|
| 0 — baseline | Mesma build e sequência sem mudar visual; frio/quente, vídeo parado/ativo | Pacote/hash, modelo, firmware, drawable, GL, versão SDL, configuração e métricas com método descrito. |
| 1 — contratos de componentes | Foco/mola atual vs nova variante; D-pad e ponteiro | Vídeo em câmera lenta e uso normal, nenhum foco perdido, ação imediata, sem regressão em movimento reduzido. |
| 2 — preparação de texto/arte | Abertura atual vs pré-aquecimento limitado | Rótulos completos, p95/p99 de abertura e input melhores ou equivalentes, sem pico novo de memória/backlog. |
| 3 — material/cache/batching | Uma otimização por execução | Mesmo enquadramento/alfa/cores, custo de GPU/CPU identificado, recursos liberados após fechar a tela. |
| 4 — capacidade | Perfis essencial/equilibrado/amplo no mesmo aparelho | Fallback preserva texto/ações; não oscila, não grava preferência inesperada, suporta memória pressionada. |
| 5 — compositor | Damage/pacing apenas se extensões e integração permitirem | Sem regiões antigas, vídeo coberto, quadro preto, tearing ou degradação de input após suspensão. |

Executar na **C9 física** e em pelo menos uma LG de menor RAM e uma geração posterior que sejam alvos reais. Registrar o modelo/firmware exatos; “webOS” sozinho não caracteriza o driver. C9 é o dispositivo de regressão histórica, não uma prova universal. Não ligar a otimização para todos os aparelhos porque funcionou no desktop.

Roteiro por variante: iniciar com cache frio; navegar 100 mudanças de foco, incluindo repetição e reversões; abrir/fechar Ajustes e busca; trocar tema/fonte; carregar catálogo longo; pressionar memória com artes; exibir controles e legendas sobre vídeo; alternar janela/hero/fullscreen quando suportado; Home/retorno; repetir com cache quente e movimento reduzido. Executar sessão contínua de ao menos 20 minutos para observar crescimento e degradação, sem tratar esse tempo como prova de ausência de vazamentos.

Entregar captura da aparência, comparação temporal, distribuições e contadores de recursos por cenário. Artefatos de Mac/protótipo servem à revisão visual e de fluxo; resultados de TV incluem gravação real do controle/tela e logs da mesma build. A decisão de adotar uma variante deve citar o benefício observado e qualquer regressão. Se não houver medida confiável de GPU/apresentação, declarar o limite e concluir apenas sobre o que foi medido.

## Registros históricos que ajudam a escolher o primeiro experimento

- `gfx.c` registra queda para 34 fps e pior quadro de 50 ms na C9 com antigo efeito ambiente em 26/09/2026, motivando o FBO 320×180. Isso prioriza evitar regressão de preenchimento fullscreen; não é benchmark executado nesta pesquisa.
- `text.c` registra medidas e relatos que explicam a evolução de duas para quatro linhas e depois para um orçamento temporal de 24 ms. Pré-aquecimento precisa resolver tanto o pico quanto a percepção de texto incompleto.
- `perfiltv.c` registra pressão de memória em LGs menores e deixa explícito quando morte por OOM é hipótese. O novo plano deve manter essa mesma separação entre correlação de RSS, causa do encerramento e evidência de sistema.

Primeira implementação recomendada após a pesquisa: instrumentação mínima, preparação de texto das telas de Ajustes e um componente de foco compartilhado. Em seguida, medir na C9 antes de ampliar material, batching ou adaptação. A ambição visual depende de resposta previsível, tipografia estável e continuidade; efeitos entram quando esses contratos já têm margem medida.
