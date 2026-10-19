# Nuvio: direção de UX, movimento e renderização

Revisão de 02/10/2026. Pesquisa ligada ao código da 1.7.1, com implementação de Ajustes em worktree isolado. A inspiração Apple é uma referência de clareza, continuidade e acabamento. A experiência precisa funcionar com D-pad, Magic Remote, pouca memória e vídeo em superfícies diferentes.

## A experiência que queremos construir

O app responde à primeira tecla, deixa claro onde o próximo OK atuará e mantém o contexto quando a pessoa volta. O movimento conecta dois estados reconhecíveis. Texto e foco continuam nítidos enquanto arte, iluminação e materiais cedem qualidade quando o aparelho precisa.

Uma assinatura visual convincente pode vir de cinco experiências pequenas e muito bem resolvidas:

| Experiência | O que a pessoa percebe | Como construir e avaliar |
|---|---|---|
| Foco que acompanha a intenção | Cada tecla tem resposta; o realce desliza sem deixar o controle “para trás” | Seleção lógica imediata, alvo de animação substituível, sem fila de movimentos. Ensaiar repetição e inversão do D-pad. |
| Abertura que conserva o contexto | O cartão selecionado conduz à página, e Voltar recupera sua posição | Arte pronta, identidade e foco preservados; sem decodificação pesada no primeiro quadro. Preparação tem geração e cancelamento. |
| Materiais contidos | A tela tem profundidade e luz, com leitura clara | Uma superfície tonal e realce local; reutilizar arte/FBO pequeno. Manter texto e bordas fora de qualquer redução de resolução. |
| Ilha como continuidade da sessão | Sair de um episódio produz uma retomada evidente, sem roubar a Home | Pílula → foco → painel → destino. Retomada e estreia têm ordem estável e ações distintas. A animação atual usa arte estática; não prometer vídeo PiP. |
| Ajustes confiáveis | Explorar é seguro, cancelar é previsível e o padrão está visível | Busca local, escolha pendente, OK para aplicar, Back para cancelar, dependência explicada e restauração individual. |

O acabamento depende também do que se evita: iniciar trailers por foco passageiro; alternar cartões enquanto a pessoa lê; mover o alvo de OK com resposta assíncrona; animar layout inteiro por uma mudança de valor; multiplicar blur, sombras e véus de tela inteira.

## Ilha: priorizar a ação relevante

A [proposta da ilha](ilha-relogio-ux.md) descreve o comportamento atual e os estados propostos. Minha recomendação é começar com relógio, sessão interrompida e uma operação iniciada pela pessoa. Estreias podem conviver na expansão. O compacto mostra um estado principal; o painel revela as ações relacionadas.

O primeiro experimento deve substituir a alternância automática de cartões por uma ordem estável, tornar o acesso evidente por D-pad e preservar o foco ao recolher. Teclas coloridas continuam atalhos opcionais. Sinopse, busca, conta, controles de áudio/legenda e diagnóstico bruto pertencem às telas que conseguem explicá-los.

Ideias posteriores que merecem protótipo: temporizador para encerrar a sessão; resultado de uma ação com Desfazer; um próximo episódio explicitamente escolhido; progresso de uma atualização solicitada. Cada ideia precisa provar por que a ilha é o melhor lugar e definir expiração, cancelamento, troca de perfil e colisão com reprodução. Não estão implementadas nem aprovadas como novos recursos por este documento.

## Uma qualidade visual, caminhos técnicos diferentes

| Plataforma | Prioridade concreta | Experimento que decide |
|---|---|---|
| Samsung `.wgt` | Cadência do navegador, tempo de WebAssembly, memória explícita de texturas e compatibilidade WebGL 1 | Medir intervalos de `requestAnimationFrame`, tarefas longas disponíveis e uploads. Comparar material local em baixa resolução com a superfície atual. |
| Samsung `.tpk` | Pacing entre host/framework e contexto EGL; efeitos ajustados à GPU | Medir espera do contexto, CPU de desenho e swap por API/Tizen. Preservar um único dono do contexto; comparar níveis existentes sem culpar shader por atraso do host. |
| LG webOS | Primeiro quadro legível, texto preparado e composição correta com vídeo separado | Comparar entrada fria/quente de sheets, fila de rasterização e uploads; validar alfa e retângulo do vídeo. Arte pode alimentar material, vídeo nativo não é uma textura GL disponível. |
| Android TV | Cadência e latência do thread SDL, `SurfaceView`/Media3 e adaptação existente | Capturar Perfetto com cenas equivalentes; separar app, compositor e vídeo. Só avaliar Swappy/Choreographer após provar que o pacing atual é o gargalo. |

Detalhes, fontes oficiais e matrizes de aparelhos: [Samsung](ux-render-samsung.md), [LG](ux-render-lg.md), [Android](ux-render-android.md). Esses documentos são pesquisas e propostas; não demonstram ganho de FPS, consumo ou fluidez em aparelhos.

## Ordem de execução recomendada

1. **Concluir o contrato de interação.** Revisar os Ajustes nativos, busca, volta, falha, dependência, restauração e leitura no sofá. Completar localização e testar com controles físicos antes de publicar.
2. **Medir uma linha de base reproduzível.** Mesma build, dados e cenas em cada TV. Capturar distribuição de quadros, resposta da tecla, rasterização, uploads e memória. Separar abertura fria/quente, repouso, rolagem, inversão de foco e vídeo ativo.
3. **Construir três cenas de acabamento.** Foco/scroll, sheet de Ajustes e ilha de retomada. Cada cena compara estado atual e proposta com movimento normal/reduzido, arte disponível/ausente e texto longo.
4. **Otimizar o gargalo observado.** Preparação incremental de texto/arte se CPU; materiais pequenos/cached se preenchimento; protocolo de apresentação se pacing. Mudar uma causa por experimento e manter uma referência visual.
5. **Expandir componentes aprovados.** Propagar a mesma gramática para Home, detalhes, busca e player; não reescrever o renderer inteiro para obter consistência visual.

A janela de um quadro é derivada da cadência efetiva medida, não da frequência anunciada do painel. Por exemplo, 60 quadros/s dá aproximadamente 16,67 ms: é uma referência aritmética, não uma garantia de entrega do Nuvio. Os limites p95/p99 e a tolerância a atraso devem ser definidos depois da linha de base, com folga para mídia e entrada.

## Critério de qualidade

O resultado precisa ser convincente com arte real, textos longos, várias teclas rápidas e uma TV que já está reproduzindo vídeo. Uma captura bonita isolada não prova isso. Registrar modelo, firmware, pacote/hash, resolução real, controle e preferência de movimento em toda avaliação.

Antes de ampliar um efeito, conferir: a primeira tecla continua respondendo; foco e texto continuam legíveis; Back retorna ao lugar correto; nenhuma ação depende de esperar animação; memória estabiliza ao repetir a tarefa; o modo reduzido mantém a informação. Para leitor de tela, é necessária integração semântica específica: desenhar texto em GLES não cria uma árvore acessível automaticamente.

## Estado desta entrega

Aplicados no código: árvore de Ajustes por intenção, avançados por categoria durante a sessão, busca local e retorno à consulta, seletor sem gravação por navegação, diferenças de fábrica, restauração individual e dependências com retorno. Os testes e as capturas são de host, com fixtures.

Continuam como próximas entregas: prévia sobre a Home real com reversão temporizada, presets locais de desempenho, mudanças na ilha e otimizações do renderer. Os documentos definem o trabalho; esses recursos ainda não estão implementados nem medidos em TV.
