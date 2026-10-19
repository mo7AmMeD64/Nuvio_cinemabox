# Samsung TV: plano de qualidade visual e fluidez do renderer

**Pesquisa em 2026-10-02.** Este documento recomenda trabalho para Samsung Tizen. Ele não altera o renderer. Separei o que os pacotes e fontes garantem, o que já foi medido e o que precisa de TV para validar.

## Direção

O caminho para um resultado mais refinado é gastar pixels onde eles mudam a leitura e manter o restante do quadro simples. O app já tem uma base forte para isso: shaders GLES 2 por modo, animação independente da taxa de quadros, imagens em cache, fundos congelados e redução adaptativa de efeitos no `.tpk`. A oportunidade agora é medir cada estágio no aparelho e usar uma política por capacidade que preserve texto e foco nítidos.

Eu manteria a composição visual atual como padrão. Qualquer nova luz, desfoque ou camada translúcida precisa substituir trabalho existente ou ficar restrita a uma região pequena. Um efeito que adiciona outro quad de tela inteira pode parecer barato no código e caro na GPU da TV.

## Dois renderers Samsung

| Pacote | Caminho gráfico | Compatibilidade relevante | Evidência atual |
|---|---|---|---|
| `.wgt` | C/WebAssembly + SDL no Chromium da TV, tela em canvas WebGL. O shell pede WebGL 1 e o código usa shaders GLSL ES 1.00. | `required_version="5.5"`: Tizen 5.5 ou posterior. A tabela oficial mapeia Tizen 5.5 a Chromium M69, Tizen 6.0 a M76, Tizen 6.5 a M85, Tizen 8 a M108 e Tizen 10 a M130. WebGL e `requestAnimationFrame` aparecem como suportados, mas shader, memória e taxa real ainda dependem do aparelho. [Especificações do Web Engine da Samsung](https://developer.samsung.com/smarttv/develop/specifications/web-engine-specifications.html) | A compatibilidade declarada é documentação. O shell já registra contexto, versão WebGL/GLSL, tamanho máximo de textura, alpha e NPOT; isso não mede a velocidade sustentada do canvas na TV. |
| `.tpk` 6+ | Host .NET abre `GLWindow`; o app C desenha em GLES 2 com um único contexto EGL alternado entre o fio do framework e o fio do app. | Tizen 6.0/API8; 6.5–7/API9; Tizen 8+/API11. O host configura espera e swap de modo diferente por API. | Tizen 6.0 em AU7000 foi confirmado como funcional. Em Tizen 9 há relatos e logs de primeiro quadro seguido de interrupção pelo lançamento normal; o início via Apps2Samsung funciona. O motivo de janela continua hipótese. [README do `.tpk`](../tizen-tpk/README.md) |
| `.tpk` 4/5.5 | Host `TVGLApplication`/`OnUpdate`; a `.so` compartilha EGL pelo mesmo contrato de alternância. | Pacote separado para Tizen 4.0–5.5. A carga da `.so` própria em TVs 4/5 ainda enfrenta restrição UEP e está em investigação. | A rota de instalação/carregamento precisa ser comprovada por modelo antes de qualquer conclusão de renderização. [README do `.tpk`](../tizen-tpk/README.md) |

O Tizen e o Chromium mudam por ano de TV; a tabela oficial lista Tizen 5.5/M69 em 2020, 6/M76 em 2021, 6.5/M85 em 2022, 7/M94 em 2023, 8/M108 em 2024, 9/M120 em 2025 e 10/M130 em 2026. O `.wgt` deve continuar no subconjunto que o app realmente compila e testa. O `.tpk` cria GLES 2; funções de ES3 ou extensões entram somente depois de confirmar versão/extensão em runtime. [Web Engine Specifications](https://developer.samsung.com/smarttv/develop/specifications/web-engine-specifications.html) · [Khronos OpenGL ES Registry](https://registry.khronos.org/OpenGL/index_es.php)

## O que o código já faz

- [gfx.c](../src/gfx.c) mantém programas de fragmento pequenos por modo e combina arte, cantos, vinheta, realce e misturas quando isso evita uma segunda passada. Há comentários com medições que mostram a sensibilidade de uma Mali-G71 a camadas de tela inteira; esses números são de uma C9 e não devem ser tratados como resultado Samsung.
- [gpunivel.c](../src/gpunivel.c) identifica modelo, Tizen, renderer e versão GL. No `.tpk`, mede FPS e tempo de espera de GPU contra CPU em janelas de quatro segundos na Home carregada. O modo automático pode retirar efeitos leves e depois efeitos mínimos; o desenho interno em 720p permanece forçado porque o texto ficou borrado nos testes citados pelo código.
- [main.c](../src/main.c) já separa evento, atualização, desenho, limpeza, swap, rasterização de texto, uploads e fill estimado. O rastro por quadro sai quando habilitado. A animação recebe `dt` limitado para recuperação de suspensão, enquanto a métrica mantém o tempo real.
- [anim.h](../src/anim.h) usa molas retargetáveis que dependem de `dt`, mola crítica sem overshoot para rolagem e uma política global de movimento reduzido. Em uma mudança de foco, o alvo pode ser imediato e a superfície pode continuar deslizando.
- [tpk.c](../src/tpk.c) impede que dois fios usem o mesmo contexto EGL ao mesmo tempo. No API8 o host espera o app concluir; no API9+ ele limita a espera e desliga o intervalo de swap conforme a configuração atual do host. Isso precisa ser avaliado por API, sem impor um timer global que atrase tecla ou player.
- [tizen-shell.html](../tools/tizen-shell.html) já monitora intervalos de `requestAnimationFrame`, tarefas longas quando `PerformanceObserver` existe, resolução, DPR, WebGL 1, shader, textura máxima e NPOT. O observador é opcional; o próprio código captura sua ausência.

O limite de preenchimento é real e já aparece em decisões existentes: vidro usa cor translúcida sem blur de fundo; o fundo de detalhe funde arte e vinheta num shader; cartões desfocados são gerados em alvos de 96×54 e armazenados em cache; a luz ambiente é pré-assada em textura pequena. Preserve esse padrão de composição e cache. [gfx.h](../src/gfx.h) registra explicitamente por que o vidro não cria cópia e blur por painel.

## Prioridades

### 1. Fechar o perfil por aparelho antes de trocar efeitos

Em cada execução, registre uma assinatura curta: modelo e versão Tizen, pacote/API, renderer/vendor/versão GL, tamanho do canvas ou janela, DPR, textura máxima, precisão de fragmento, extensões de descarte e NPOT. No `.wgt`, acrescente o resultado de testar `webgl2` apenas como diagnóstico; não mude o renderer para WebGL 2 por causa da presença em Chromium desktop. No `.tpk`, mantenha GLES 2 como contrato comum e trate recursos superiores como opcionais. A Samsung expõe capacidades OpenGL ES e formatos de textura via SystemInfo, mas a decisão final deve continuar sendo teste de capacidade e não ano do aparelho. [SystemInfo: capacidades do dispositivo](https://developer.samsung.com/smarttv/develop/api-references/tizen-web-device-api-references/systeminfo-api/getting-device-capabilities-using-systeminfo-api.html)

### 2. Medir distribuição de quadros, não só FPS médio

Acrescente ao relatório agregado p50/p95/p99 do intervalo entre quadros, quadros que perdem a próxima janela de apresentação e tempo da tecla ao primeiro quadro que mostra o foco novo. Registre separadamente:

- `.wgt`: intervalo de `requestAnimationFrame`, tempo do loop WebAssembly, atualização, desenho e pausa longa atribuída ao navegador quando o observador existir;
- `.tpk`: callback do host, espera pelo contexto, `glClear`, CPU de desenho e espera de swap;
- ambos: bytes e número de uploads, texto rasterizado, fill por modo e número de retângulos.

Defina o orçamento a partir da cadência que a TV realmente entrega, não de 60 Hz presumidos. Para uma cadência medida `f`, a janela nominal é `1000/f` ms; publique o tempo por estágio e a fração de quadros que excede essa janela. O valor atual de 45 FPS continua sendo o gatilho histórico do adaptativo do `.tpk`, não uma promessa de que toda TV apresenta 60 FPS.

No `.tpk`, não mova EGL ou GL para um terceiro fio. Faça decodificação, normalização e preparação de dados em CPU fora da seção do contexto; limite a fila para não transformar uma rolagem rápida em upload acumulado. O próprio protocolo do app exige que somente um fio seja dono do contexto.

### 3. Escalonar qualidade sem borrar texto

Priorize a seguinte ordem quando a medição mostrar GPU limitada:

1. Remover realces decorativos grandes e luzes de tela inteira antes de alterar resolução. O nível automático já segue essa direção.
2. Renderizar fundos, sombras suaves e previews em alvos reduzidos, mantendo texto, linhas de foco e controles na resolução nativa.
3. Reusar snapshots de telas que não mudam e regenerar blur só quando a arte, o tamanho ou a cena mudar. Evitar uma cópia da tela para cada painel.
4. Só avaliar resolução global menor como modo de acessibilidade/último recurso, com texto fora do alvo reduzido. O 720p integral já foi rejeitado visualmente nos testes referidos pelo código.

No blur, preserve duas passadas separáveis e os FBOs existentes. Se uma nova superfície translucida exigir blur, gere uma textura de fundo em baixa resolução uma vez para o conjunto de painéis visíveis, e atualize somente depois de movimento/rolagem assentarem. O fallback deve ser a superfície colorida translúcida atual. Nada no doc presume que a GPU Samsung aceite o custo sem medição.

### 4. Manter navegação rápida mesmo com animação

O foco deve responder no primeiro quadro disponível. Anime a superfície e o conteúdo depois do foco lógico, sem deixar a seleção esperando a mola. Preserve retargeting quando chegam várias teclas D-pad e movimento reduzido como caminho direto ao alvo. Compare curvas usando quadros carimbados do vídeo e timestamps de entrada, em mais de uma taxa de quadros. Se o intervalo entre quadros piorar, evite alongar a duração da animação para esconder o problema; reduza apenas efeitos caros e preserve o deslocamento e contraste de foco.

### 5. Controlar memória e custo de texturas

Mantenha o cache limitado por bytes e tamanho de exibição, libere FBOs e snapshots quando saírem de uso e não carregue imagens maiores que a maior área onde serão exibidas. Um frame RGBA de 1920×1080 ocupa aproximadamente 8,3 MB decimal antes de cópias e buffers adicionais; 4K ocupa cerca de 33 MB. A Samsung ressalta que tamanho comprimido do JPEG não é memória de imagem decodificada e que texturas WebGL não são liberadas pelo GC: a aplicação deve removê-las explicitamente. [Web App Memory Optimization Guide](https://developer.samsung.com/smarttv/develop/guides/web-app-memory-optimization-guide.html)

Teste memória durante abertura/fechamento repetido de detalhe, Spotlight, guia e player, além de uma sessão longa de rolagem. No `.wgt`, conte canvas, texturas e caches JS. No `.tpk`, conte texturas/FBOs e cache C, reconhecendo que buffers de vídeo e compositor da TV ficam fora dessa contagem.

## Matriz de validação

| Coorte | Caminho | Pergunta principal |
|---|---|---|
| Tizen 5.5/M69 | `.wgt` mínimo declarado | WebGL 1, shaders atuais e escalonamento do canvas mantêm texto/foco nítidos? |
| Tizen 6.0/M76 | `.wgt` e `.tpk` API8, em execuções separadas | O loop do canvas e o `GLWindow` sustentam a cadência nativa? Tecla e áudio continuam responsivos sob carga? |
| Tizen 6.5–7/API9 | `.tpk` API9 | A política de espera/swap mantém quadro novo sem fila de apresentação ou atraso de foco? |
| Tizen 8+/API11 | `.wgt` e `.tpk` API11 | O renderer melhora sem depender de capacidades ausentes nos modelos antigos? O launch path está resolvido antes do perfil de FPS? |
| Tizen 4.0/Mali-400 | `.tpk` legado somente depois de a instalação/UEP estar resolvida | Os efeitos mínimos em 1080p dão ganho que o custo de fill não entrega? |

Para cada linha: TV física, firmware anotado, pacote/hash anotado, Home vazia e carregada, rolagem contínua, sequência de foco rápido, abrir/fechar painéis, mudança de tema, player aberto/fechado e teste de 15 minutos. Faça aquecimento igual, três execuções por cenário e compare mediana e pior janela; registre também a imagem do texto e do foco. O emulador ajuda a depurar código, mas não substitui o device: a Samsung avisa que suas medições podem divergir da TV real e que recursos dependem do hardware. O Web Inspector oferece Timeline e Profiles, mas o FAQ diz que o Dynamic Analyzer não faz profiling de memória em TVs físicas e que testes automatizados não são suportados; preserve o rastro interno para observar o app em execução real. [Web Inspector](https://developer.samsung.com/smarttv/develop/getting-started/using-sdk/web-inspector.html) · [Recursos do Web Inspector](https://developer.samsung.com/smarttv/develop/getting-started/using-sdk/web-inspector/web-inspector-features.html) · [Application Testing Q&A](https://developer.samsung.com/smarttv/develop/faq/application-testing.html)

## Fontes e limite de conclusão

Fontes técnicas externas, consultadas em 2026-10-02:

- [Samsung Web Engine Specifications](https://developer.samsung.com/smarttv/develop/specifications/web-engine-specifications.html) — motor por ano de TV e suporte da plataforma Web.
- [Samsung Web App Memory Optimization Guide](https://developer.samsung.com/smarttv/develop/guides/web-app-memory-optimization-guide.html) — memória de imagens, canvas, WebGL e ciclo de vida de recursos.
- [Samsung Web Inspector](https://developer.samsung.com/smarttv/develop/getting-started/using-sdk/web-inspector.html) e [Web Inspector Features](https://developer.samsung.com/smarttv/develop/getting-started/using-sdk/web-inspector/web-inspector-features.html) — inspeção disponível e seus limites.
- [Samsung SystemInfo API](https://developer.samsung.com/smarttv/develop/api-references/tizen-web-device-api-references/systeminfo-api/getting-device-capabilities-using-systeminfo-api.html) — consulta de capacidades de OpenGL ES e formatos.
- [Khronos OpenGL ES Registry](https://registry.khronos.org/OpenGL/index_es.php) e [EGL 1.5 Specification](https://registry.khronos.org/EGL/specs/eglspec.1.5.withchanges.pdf) — contrato de API, shaders e apresentação EGL.

**Garantido por código/documentação:** os perfis de pacote, o canvas WebGL 1 atual, a criação do contexto GLES 2, as medições internas já descritas e as regras de movimento reduzido.

**Medido, mas específico:** os números dentro de comentários do renderer pertencem aos modelos e sessões ali nomeados. Os registros Tizen 6 e Tizen 9 no README comprovam funcionamento, travamento ou caminho de lançamento conforme escrito; não comprovam uma taxa de quadros para todos os modelos.

**Ainda precisa de Samsung física:** distribuição de tempos por API/firmware, custo relativo de cada shader, orçamento estável de uploads/memória, benefício de targets reduzidos, latência tecla→foco e validação de qualquer mudança em frame pacing. Escolha as otimizações pelos dados dessa matriz.
