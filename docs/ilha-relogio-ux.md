# Ilha do relógio e miniplayer: proposta de UX

**Estado:** proposta para revisão; nenhuma mudança de produto implementada por este documento.
**Escopo:** inspeção estática do código nativo deste branch e referências públicas de plataforma, em 02/10/2026. Não é validação em TV.

## Decisão de produto

Manter a ilha como uma superfície pequena de estado e retomada. Ela deve responder, em poucos segundos e sem abrir outra tela, “o que está acontecendo?” e “qual é a próxima ação?”. O relógio é o estado neutro; avisos, tarefas em andamento e cartões de retomada ocupam temporariamente o mesmo espaço. Expandir mostra detalhes e ações diretamente relacionadas ao cartão visível. A ilha não deve virar um mini dashboard com atalhos, conteúdo recomendado e dados de conta.

Tratar a Dynamic Island do iPhone como referência de **continuidade visual e transição entre compacto e expandido**, não como modelo literal de tamanho ou gesto. A TV é compartilhada, vista à distância e operada por direcional. A orientação de TV do Android usa aproximadamente 3 m como distância típica e recomenda limitar texto e complexidade; o guia da Apple para tvOS também destaca arte legível à distância, foco visível e uso natural do controle. [Apple: Designing for tvOS](https://developer.apple.com/design/human-interface-guidelines/designing-for-tvos/), [Android: Design for TV](https://developer.android.com/design/ui/tv/guides/foundations/design-for-tv).

## O que existe no código

| Estado ou capacidade | Evidência no branch | Limite observado |
|---|---|---|
| Relógio discreto | `ilha.c` desenha `HH:MM`; `relogioCabe()` libera a ilha principalmente na Home pronta, sem camadas que a cobririam. Há posição automática/esquerda/direita e exceção no Guia. | O relógio permanente depende de Ajustes e de a tela permitir sua presença. Avisos podem aparecer fora da Home, mas não sobre o player. |
| Avisos curtos | `ilha_avisar()` mantém o aviso atual e fila de quatro; mesma chave atualiza no lugar. A validade começa no primeiro quadro visível. | A fila elimina o aviso mais antigo ao lotar. Um aviso que chega pode preemptar atividade e cartões. |
| Atividade transitória | `ilha_atividade()` renova por quadro e expira após cerca de 400 ms sem atualização. Pode mostrar progresso numérico. | É um canal temporário, sem histórico acessível na própria ilha. Uma tarefa renovada continuamente ocupa a posição principal. |
| Cartão de sessão interrompida | `ilhacart_player_saiu()` cria cartão com título, episódio, arte, progresso e minutos restantes. Só existe se o retorno ao item for válido. Expira após 30 min sem tecla. | Não mantém vídeo nem áudio em segundo plano; é uma lembrança visual com ação para iniciar de novo. |
| Lembrete de estreia | Uma estreia pendente pode ocupar o cartão, desde que o relógio esteja habilitado. Abrir a página do mesmo título marca o aviso como visto. | É ligado ao relógio habilitado; não é uma central geral de lembretes. |
| Alternância de dois cartões | Sessão e estreia alternam automaticamente a cada seis segundos. | Não há seletor de cartões compactos nem indicador explícito de que há outro cartão à espera. |
| Modal do cartão | Azul/CH+ ou clique na pílula expande o mesmo painel; há Retomar/Assistir, Detalhes e Fechar/Marcar como visto. Setas movem o foco; Voltar recolhe. “Salvos” fica depois do último botão/direita. | Azul e CH+ são atalhos contextuais, não um caminho universal evidente. O código de desenho do modal não cria semântica nativa de leitor de tela. |
| Minimizar a sessão para a ilha | Ao sair no meio, com relógio ligado e opção configurada, o player fecha, a Home volta e a arte disponível encolhe até a capa do cartão. Animações reduzidas fazem a troca direta. | A moldura animada é uma imagem estática (still/fundo), não o vídeo ao vivo. O plano de vídeo da LG não pode ser lido de volta pelo renderer. |
| Custo visual | Desenho leve da pílula; o voo usa arte e cartaz já acessados, sem FBO/tela cheia adicional. | Custo e estabilidade em outras GPUs, resoluções, escalas e versões do SDL não foram comprovados aqui. |

Referências de código: [ilha.h](../src/ilha.h), [ilha.c](../src/ilha.c), [ilhacart.c](../src/ilhacart.c), [app.c](../src/app.c), [ajustes.c](../src/ajustes.c), [layout.h](../src/layout.h), [relogio.c](../src/relogio.c). O relógio interpolado de playback em `relogio.c` é um sincronizador do tempo de vídeo; não é o relógio civil exibido pela ilha.

## Hierarquia proposta

Usar quatro níveis com uma única pergunta por nível:

1. **Pílula compacta — perceber.** Mostrar hora quando não há estado relevante. Para cartão persistente, mostrar uma identificação curta do título e estado (“Retomar · T1E3 · 32 min”) com um sinal visual de que há ação. Para tarefa, mostrar verbo + estado/progresso. Para aviso, mostrar frase curta e ícone; evitar frase de marketing ou segundo assunto.
2. **Entrada focada — reconhecer o alvo.** No controle remoto, a pessoa deve conseguir chegar à pílula e ver foco explícito antes de ativar. O foco não deve iniciar reprodução. A pílula compacta não tenta encaixar título, episódio, sinopse, progresso e ações juntos.
3. **Painel expandido — decidir.** Para uma sessão, mostrar poster/still, título, episódio, posição retomável, tempo restante e ações Retomar, Detalhes e Dispensar. Para estreia, mostrar episódio, data e Assistir/Detalhes/Marcar como visto. Explicar requisitos quando a ação falhar; confirmação só quando a consequência for difícil de desfazer.
4. **Destino — concluir.** Retomar abre o mesmo item e posição válida; Detalhes abre o título; Salvos vai ao painel de salvos; dispensar remove apenas aquele cartão. O retorno preserva a tela, a seção e o foco de origem sempre que o destino não substitui a tela.

O conteúdo deve sobreviver às mudanças de apresentação: hora e título não devem desaparecer e reaparecer sem motivo no mesmo expandir/recolher. A recomendação da Apple para Live Activities também pede informação compacta e útil, continuidade dos elementos entre layouts, poucos controles e transições curtas. Isso ajuda como princípio; os contratos e limites de ActivityKit/Dynamic Island não se aplicam ao renderer C do Nuvio. [Apple: Live Activities](https://developer.apple.com/design/human-interface-guidelines/live-activities/).

## Modelo de estados e interrupções

O código hoje escolhe uma saída principal nesta ordem: aviso > atividade recente > cartão persistente, se a ilha couber > relógio. A proposta deve tornar a política visível e evitar que trabalho silencioso esconda indefinidamente algo que a pessoa precisa resolver:

| Prioridade proposta | Exemplo | Comportamento |
|---|---|---|
| 1. Ação que pede atenção agora | Erro recuperável de reprodução, confirmação necessária | Pode interromper por pouco tempo; texto diz o problema e uma ação possível. Nunca abrir modal sozinho. |
| 2. Sessão que a pessoa acabou de interromper | Player fechado no meio | Manter como cartão principal até ação, substituição por sessão mais recente ou expiração explicada. Atualizações sem mudança real não reiniciam animação. |
| 3. Progresso de operação iniciada pela pessoa | Atualização iniciada, sincronização solicitada | Mostrar enquanto muda; ao concluir, converter uma vez em confirmação curta. Operações passivas não devem ocupar a pílula por renovação artificial. |
| 4. Lembrete futuro | Estreia | Pode aparecer como segundo cartão acessível pela expansão; não disputar com a retomada por alternância automática. |
| 5. Estado neutro | Relógio | Base estável quando não há algo útil a fazer. |

Se dois cartões persistentes coexistirem, a proposta é mostrá-los juntos só na expansão, em no máximo duas linhas selecionáveis, com ordem estável: sessão interrompida primeiro, lembrete depois. Remover a rotação silenciosa a cada seis segundos: ela muda o conteúdo sob o olhar da pessoa, dificulta leitura, descoberta e navegação, e não oferece uma forma clara de recuperar o cartão que acabou de sair. Se o espaço compacto não comportar uma segunda indicação sem reduzir legibilidade, manter um único cartão e oferecer a lista na expansão.

Cada operação tem ciclo explícito: `inativa → em andamento → concluída | falhou | cancelada`. Atualização de percentagem pode mudar a barra, mas não deve gerar vários avisos de conclusão. Se outra atividade chega, o cartão de retomada fica disponível; ao cessar a atividade, ele volta sem zerar seu contexto. Uma falha prioritária sai da posição compacta após prazo curto, mas continua recuperável no destino relevante quando isso existir. A central de avisos mantém sua própria fila; a ilha não deve prometer histórico que a central não conserva.

## Interações no controle remoto

Apple recomenda comportamento consistente com foco e gestos já conhecidos no tvOS; Android TV também pede que todo controle visível seja navegável com D-pad, com direção previsível e retorno claro. [Apple: Focus and selection](https://developer.apple.com/design/human-interface-guidelines/focus-and-selection/), [Apple: Remotes](https://developer.apple.com/design/human-interface-guidelines/remotes/), [Android: Navigation on TV](https://developer.android.com/design/ui/tv/guides/foundations/navigation-on-tv).

Proposta inicial de contrato, sujeita a teste com o mapeamento real de cada controle:

| Contexto | Direcional/OK | Voltar |
|---|---|---|
| Home, pílula fechada | Direcionais chegam à pílula se ela estiver disponível; foco marcado sem iniciar ação. OK abre detalhes do estado atual. | Não deve ser a única forma de achar a pílula. |
| Painel do cartão | Cima/baixo percorrem ações e cartões; esquerda/direita mantêm vizinhanças coerentes; OK ativa apenas a ação focada. | Fecha o painel e retorna à mesma Home e ao ponto de foco anterior. |
| Modo de reprodução | Não sobrepor a ilha no player enquanto a sessão está ativa. | No comportamento atual, Voltar fecha conforme o player; a ilha só recebe sessão após a saída. |
| Minimizando | Não aceitar uma segunda ativação que abra outro destino antes de o pouso terminar. | Se a tela de origem ficar inválida ou entrar em outra camada, cancelar a animação e terminar em estado coerente. |

Azul/CH+ pode continuar como atalho opcional, depois de validar que não colide com funções do aparelho e que existe alternativa completa por D-pad. Hoje CH+ ganha o modal quando há cartão; a tecla Azul alterna entre esse modal e Salvos conforme o tempo segurado. Esse gesto de 400 ms precisa de indicação e ensaio em cada controle. Ações essenciais não devem depender de tecla colorida, repetição do firmware, clique do Magic Remote ou segurar OK.

Com animações reduzidas, usar o mesmo estado final em uma troca imediata. O foco deve pousar no cartão ou destino resultante; animação nunca pode ser condição para a ação funcionar. O app deve absorver teclas durante uma transição estrutural ou encaminhá-las deterministicamente depois dela, sem duplicar ação.

## O que cabe no cartão, e o que fica fora

**Cabe no compacto:** hora; estado em poucas palavras; um indicador de progresso; uma capa pequena só quando ajuda a identificar um título. Mostrar um dado por vez além do título. Texto de horário/progresso deve sobreviver a fonte grande, tradução longa e modo de contraste.

**Cabe na expansão:** título e episódio; arte em uma região; posição de retomada, progresso e tempo restante; até três ações primárias com nomes inequívocos; no máximo dois cartões persistentes; erro e requisito da ação. A sinopse longa deve ser omitida ou limitada a poucas linhas. O nome do perfil, conta, identificador, servidor, endereço ou chave não deve aparecer.

**Fora da ilha:** controles de transporte contínuos (seek, volume, faixa), teclado, credenciais, busca, catálogo, sugestões/recomendações, configuração extensa, diagnóstico técnico bruto, histórico completo, controles gerais do sistema, atualização forçada, PiP alegado e reprodução de áudio em segundo plano sem suporte real. Esses itens precisam de uma superfície com mais espaço e um contrato de retorno próprio.

Um aviso técnico pode explicar uma falha, mas levar ao fluxo que resolve a causa; não encaixar um menu de diagnóstico inteiro. O card de estreia nunca deve contar como “assistido” só por aparecer ou receber foco. A ação atual “Marcar como visto” permanece explícita.

## Distância, foco e acessibilidade

Texto compacto precisa ser lido de longe num relance. A documentação Android recomenda tipografia maior e limitar blocos de leitura para a distância de TV; Apple recomenda que todos os elementos relevantes possam receber foco e que o indicador seja consistente com a plataforma. [Android: Typography for TV](https://developer.android.com/design/ui/tv/guides/styles/typography), [Apple: Focus and selection](https://developer.apple.com/design/human-interface-guidelines/focus-and-selection/).

- Manter frase compacta em uma linha e truncar no limite com uma expansão que revela o nome completo; nunca reduzir a fonte para resolver uma string longa.
- Usar verbo e estado em texto. Cor e ícone complementam, mas não substituem “Baixando”, “Erro” ou “Retomar”.
- Diferenciar aviso informativo, sucesso e falha por texto, ícone e contraste, não apenas matiz.
- No painel, área de foco grande, margem suficiente e rótulos completos. O layout atual usa botões na base do cartão; medir foco remoto, safe area, overscan e legibilidade nas resoluções de 720p/1080p/4K.
- Respeitar movimento reduzido sem retirar informação. Não usar brilho pulsante ou animação repetida como única indicação de urgência.
- Não há, nesta inspeção, evidência de nomeação/accessibility tree da ilha desenhada manualmente. Sem essa camada, o leitor de tela da TV pode não ler hora, estado, progresso ou rótulos. A solução depende da integração específica de Tizen, Android, Apple TV ou SDL e precisa de auditoria por plataforma.

## Privacidade e convivência

A TV é compartilhada. A pílula pode exibir automaticamente título, episódio, arte, percentual e tempo restante de uma sessão interrompida; a expansão inclui sinopse. Tratar isso como informação visível a terceiros, mesmo que não contenha credenciais. O guia oficial Android para TV explicita o caráter compartilhado do aparelho e a necessidade de personalizar a exibição de informação pessoal; a Apple também recomenda evitar informação sensível em superfícies sempre visíveis. [Android: Design for TV](https://developer.android.com/design/ui/tv/guides/foundations/design-for-tv), [Apple: Live Activities](https://developer.apple.com/design/human-interface-guidelines/live-activities/).

Proposta: manter relógio neutro até a pessoa sair do player; ocultar sinopse no compacto; avaliar uma opção local “Ocultar detalhes da retomada” que deixe apenas “Vídeo pausado” e a ação Retomar. Não introduzir rastreamento, telemetria ou persistência nova com esta superfície. Cartões permanecem na memória desta sessão e mantêm as regras já existentes de expiração; qualquer extensão além disso precisa definir exclusão, troca de perfil e logout.

## Plataforma, vídeo e custo

O voo atual não é PiP: o player encerra, a Home aparece e uma arte estática encolhe até a capa. Esse limite está ligado ao caminho de vídeo LG descrito em `ilha.c`; não anunciar “continuar assistindo em miniatura” como vídeo ao vivo. O som também termina quando `player_encerrar()` é chamado. Picture-in-picture real, áudio de fundo, canal de vídeo ou composição de plano de hardware dependem de API e comportamento provados na plataforma/TV, além do foco e da tecla Voltar dessa plataforma.

Preservar a regra de custo atual enquanto não houver medição nova: sem captura de frame por GPU, sem FBO de tela inteira, arte já cacheada, transição limitada à imagem e retângulos do voo. Usar placeholder se a arte não existir; não iniciar busca de rede só para animar a saída. A C9/Mali-G71 é o teto mencionado pelo código para desenho geral, mas não é resultado medido desta proposta. Validar memória de texturas, quadros perdidos e tempo de fechamento em cada renderer.

## Estados e casos que precisam de desenho/teste

| Cenário | Resultado esperado proposto |
|---|---|
| Sessão interrompida com relógio ativo | Player fecha; destino Home; animação opcional da arte ao cartão; foco repousa em estado acionável. |
| Relógio desativado ou posição indisponível | Sai pelo fluxo conhecido da página do título; sem voo para um alvo invisível. |
| Título concluído | Não cria cartão de retomada; segue o avanço normal de episódio já existente. |
| Cartão some enquanto expansão está aberta | Fechar/atualizar o painel com aviso curto e foco válido; não deixar ações apontando para sessão antiga. |
| Nova sessão interrompida chega com outra já pendente | Nova vira a principal; definir explicitamente se a anterior substitui ou entra na lista, sem rotação oculta. |
| Estreia e retomada coexistem | Ambas continuam acessíveis e não alternam sozinhas; ação de uma não dispensa a outra. |
| Aviso/erro chega durante modal | Não roubar foco nem trocar o cartão em que o modal foi aberto. Mostrar após fechar, exceto risco que exija interrupção. |
| Rede cai ou arte não carrega | Estado e ações continuam disponíveis com placeholder; não deixar a expansão vazia nem bloquear Retomar. |
| Sai para outra tela durante voo/modal | Encerrar ou completar de forma determinística sem desenhar sobre o destino e sem deixar estado preso. |
| Perfil troca/logout | Definir o que pode continuar na ilha e limpar cartões associados ao usuário conforme o limite de perfil já existente. Validar em TV. |
| Animação reduzida, baixa taxa de quadros ou app retomado após pausa | Pousar no estado final em um quadro; não usar um `dt` grande para saltar ou travar animação. |
| Fonte grande/tradução/RTL | Layout refluído, sem cortar ação, hora ou estado principal; testar todos os idiomas com texto mais longo. |

## Plano de validação

Antes de alterar código, aprovar o modelo de prioridade e a decisão sobre múltiplos cartões. Depois testar quatro tarefas: notar retomada; retomar o episódio exato; dispensar sem marcar como visto; encontrar e abrir a estreia enquanto há retomada. Comparar Home com relógio ligado/desligado e com cada posição.

Fazer uma matriz de remotes por plataforma: setas, OK, Voltar, CH+/Azul se existirem, repetição de tecla física, Magic Remote/pointer, pausa do app e retorno de app. A captura `tests/ilha2_shot.c` cobre estados de renderização e `tests/ilha_shot.c` cobre relógio/avisos; são fixtures de captura, não validação de foco, acessibilidade, comportamento de aparelho, vídeo PiP ou desempenho de produção.

Coletar no dispositivo: taxa de quadros durante o voo/modal; pico de textura/memória; latência da tecla; erro de alvo e de ação; entendimento de que a miniatura é imagem estática; leitura a distância em luz normal; resultado do leitor de tela. Registrar modelo, SO/firmware, resolução, build exato e controle. Não inferir suporte de uma TV pelo nome da plataforma nem pela captura de desktop.

### Condições para avançar

1. Produto aprova prioridade, duração e destino dos cartões persistentes.
2. Dono consegue achar a ilha e percorrer cada ação só com D-pad/OK/Voltar, sem conhecer uma tecla colorida.
3. Retomar inicia o título/episódio/posição certos; descartar e marcar como visto continuam distintos.
4. Nenhuma tarefa passiva oculta indefinidamente uma retomada; avisos não roubam o foco de uma decisão em andamento.
5. Legibilidade, movimento reduzido, safe area, privacidade e acessibilidade estão verificados por build/dispositivo.
6. O custo do voo é aceitável no menor hardware suportado; vídeo ao vivo só entra se houver prova específica de API, composição, áudio, controle e retorno.

## Fontes de plataforma consultadas

- [Apple Human Interface Guidelines — Designing for tvOS](https://developer.apple.com/design/human-interface-guidelines/designing-for-tvos/)
- [Apple Human Interface Guidelines — Focus and selection](https://developer.apple.com/design/human-interface-guidelines/focus-and-selection/)
- [Apple Human Interface Guidelines — Remotes](https://developer.apple.com/design/human-interface-guidelines/remotes/)
- [Apple Human Interface Guidelines — Live Activities](https://developer.apple.com/design/human-interface-guidelines/live-activities/)
- [Android Developers — Design for TV](https://developer.android.com/design/ui/tv/guides/foundations/design-for-tv)
- [Android Developers — Navigation on TV](https://developer.android.com/design/ui/tv/guides/foundations/navigation-on-tv)
- [Android Developers — Typography for TV](https://developer.android.com/design/ui/tv/guides/styles/typography)
