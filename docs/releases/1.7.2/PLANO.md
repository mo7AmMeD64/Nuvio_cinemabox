# Plano aprovado para 1.7.2 — 02/10/2026

Somente local, em `release/1.7.2` (`/private/tmp/nv-172`), base 1.7.1 `2bc9659b`. Dono autorizou executar esta proposta e escolheu a reorganização funcional dos Ajustes; peles A/B, ícones de apoiador e social continuam fora. Testes focados + i18n; comportamento nas TVs testado pelo dono. Nenhum push/tag/publicação/comentário nesta rodada.

## Inclusão posterior — screensaver webOS

Pedido explícito do dono: incluir `8a4995617bb4a43810a46e064dbfd7a61540146e` na 1.7.2. Cherry-pick local `857f83e3`; ajuste de integração `11835ffa`. A chamada original estava no caminho de falha de registro LS2. Agora assina no callback do load válido, uma vez por ciclo do barramento, e não bloqueia novas tentativas após falha de transporte.

Responde ao estado Active usando o timestamp recebido sem truncar/arredondar. Bloqueia screensaver somente com mídia efetivamente tocando; libera em pausa, fim, erro ou ausência de mídia. Ignora estados não Active, timestamps ausentes/inválidos/longos e callback após encerramento. Reset permite assinar no próximo ciclo do backend.

**Estado da candidata:** código atualizado; testes focados e i18n PASS. Validação física e novo empacotamento pendentes. Pacotes `b71471bc` são anteriores ao screensaver. Detalhes em VALIDACAO-LOCAL.md.

## Já integrado

`agente/abrirrapido` (inclui `ilhavolta` e `i221`) e `agente/i216`. Nova ilha, fontes parciais, espera configurável pelos addons, fonte da última sessão com fallback, toque/arrastar e login por e-mail. Retenção decidida em dois minutos, commit `1bf49696`; teste 119/120 s passou. Nove testes focados aprovados. Na instalação anterior desta rodada, a TCL recebeu o APK estável 1.7.1; outra sessão relata instalação posterior de APK social. O pacote atual no aparelho não foi reconsultado. Envio e validação física da 1.7.2 terão registro próprio.

Conferência da fonte guardada já roda em paralelo (`app.c:tocarFonteGuardada`); não contabilizar como trabalho futuro nesse caminho. Conferência paralela da abertura normal foi adiada nesta rodada; a sonda foi endurecida, mas cancelamento por chamada ainda é requisito para ampliar a concorrência.

## Execução local desta rodada

- `2d6f2eb9`: operações do catálogo protegidas, atualização tardia confere identidade do título; histórico muda por conta/perfil e rejeita respostas antigas do Trakt/extras. ASan/UBSan e ThreadSanitizer focados passaram.
- `49ad2fbc`: ícone do celular embutido no núcleo, mesma arte Lucide do pacote, um upload por contexto. Registro 18898 comprova núcleo 1.7.1 carregado por memfd junto do host api11 e arte ausente; não informa versão exata do pacote antigo. Teste normal passou; tentativa ASan com SDL travou antes de main e não vale como aprovação.
- `d8402e54`: fila durável de addons por conta/perfil; HTTP500 seguido de reinício restaura a edição antes da rede. Confirmação remove somente o snapshot exato; falhas de leitura/remoção preservam a retentativa e bloqueiam um pull antigo. Logout limpa snapshots e temporários; oito entradas alocadas sob demanda, sem acesso ao disco por quadro. Syncordem e contaoffline com ASan/UBSan e revisão independente passaram. Falha de gravação conserva a edição em RAM e registra o problema; não promete durabilidade sem armazenamento funcional.
- `fc711f6b`: templates de temporários respeitam o disco escolhido em Android/ARM/TPK/conferência Samsung; exclusão e guard final alcançam snapshots de conta e arquivos temporários. A fonte final `b71471bc` inclui essa correção em todas as plataformas.
- `bff916b4`: snapshot de addons no fio principal, revisão/ack por edição e perfil; falha 500 mantém alteração local, não aplica pull anterior ao push e não perde edição feita enquanto o push responde. Retentativa usa ritmo/backoff existente. Syncordem/offline/limites com ASan/UBSan passaram. Nenhum deploy no servidor; causa remota de HTTP500 não demonstrada.
- `f9083b10`: reorganização funcional `ajustesux` mesclada por escolha do dono. Defaults e enum persistido conservados; espera pelos addons continua na seção Reprodução, padrão cinco segundos. `61ab2263` comprova os quatro prazos, escopo local, entrada única em Reprodução → Escolha da fonte e resultado da busca sem duplicação. Dados, interação, seções, padrões, perfis e i18n passaram; 12 cenas nativas da UX foram capturadas e revisadas no Mac. Foco claro com texto escuro, categoria ativa e textos auxiliares legíveis; teclas simuladas cobrem confirmar/cancelar/Voltar/restaurar/busca/avançados. Essas provas não substituem controle físico e painel das TVs.
- `4412ae0c` / `1b74378d`: Android recebe posição local válida durante a preparação, com ack por sessão e fallback; o ack inicial entra na telemetria da abertura. Testes distinguem posição válida, só percentual, episódio/perfil, ao vivo e sessão retida; Kotlin/NDK compilam. Tempo da pré-busca é corrigido somente no log.
- Conferência paralela genérica da primeira mídia adiada: o GET Range pode criar arquivos/cobrar cota no debrid, e o caminho atual não cancela a transferência. A retomada guardada já abre em paralelo. Corrigir o contrato da sonda antes de ampliar essa concorrência.
- `5fcae85c`: sonda nativa valida HTTP2xx, cabeçalhos, transporte e URL sem truncamento; descarta corpo e corta no teto. WGT não copia corpo ao heapWASM e preserva autorização inconclusiva quando XHR não consegue cabeçalho que AVPlay manda. Focos nativos ASan/UBSan, políticaWGT, três regressões de fonte, sintaxeTPK4/6 e i18n passaram. XHR síncrono ainda não oferece cancelamento/timeout por chamada.
- Proporção TPK: recorte da origem desativado por padrão por limite do backend; não reintroduzir ROI fora da tela que já expôs a Home da Samsung. Sem alteração .NET; #195 ainda depende do cenário/aparelho/fonte do relator.

## Achado posterior da ilha

O inventário solicitado pelo dono confirmou isolamento incompleto do cartão de retomada e suas cópias de UI/pedido durante troca de conta/perfil. O player retido já bloqueia a identidade antiga; o cartão/fallback ainda podia expor o título/episódio anterior. Correção `b71471bc` aprovada com ASan/UBSan e revisão independente: identidade/instância validadas antes de evento, desenho e fallback; invalida cartão, modal, pedido e voo, inclusive após ociosidade. Avisos/estreias e sessão em queda de API são conservados. Pacotes `fc711f6b` preservados como intermediários; nova geração concluída a partir desse fix, com 13 anexos e checksums aprovados. Mockup e novas funcionalidades continuam propostas separadas.

## Execução e pendências

1. **Consistência — concluída no código.** `2d6f2eb9` protege leituras/escritas no vetor publicado sem mutex recursivo e rejeita atualização cujo índice mudou de título. O mapa de assistidos agora muda na fronteira efetiva de conta/perfil; geração capturada antes da rede impede resposta atrasada de repovoar a identidade nova. Mesma identidade conserva provas locais/de outras fontes. Testes cobriram A→B, perfil, logout, A→B→A, fallback de progresso, 6.400 inserções em oito threads e publicação concorrente do catálogo. ASan/UBSan e ThreadSanitizer aprovados; validação física continua pendente.
2. **Autoatualização TPK e arte — fallback implementado.** Cinco registros 1.7.1 não encontram `aj_smartphone.png`; os quatro pacotes completos 1.7.1 contêm esse arquivo. Um registro comprova núcleo atualizado por memfd, host api11 e arte ausente, sem revelar a versão exata do pacote antigo. `49ad2fbc` inclui o ícone essencial no núcleo e evita tentativas de arquivo ausente por quadro. Conferir o resultado numa Samsung com host anterior permanece teste físico; a hipótese não foi generalizada a todos os registros.
3. **Sincronização e indisponibilidade — proteção do cliente concluída.** Dois registros Android trazem push de addons HTTP500; também há 521/522/429 em operações Nuvio. `bff916b4` preserva edição local após falha, descarta ack de outro perfil, mantém edição feita durante o POST e conserva o backoff existente. A fila durável de `d8402e54` acrescenta restauração entre processos e proteção contra erro transitório de leitura no boot e no ACK. Testes focados com ASan/UBSan e revisão independente passaram. A causa remota dos 500 não foi demonstrada; nenhum servidor foi alterado ou publicado. Não contabilizar isso como recuperação comprovada do serviço externo.
4. **Abertura de filme — Android e telemetria implementados; ganho físico pendente.** A preparação recebe posição válida, com confirmação por sessão e fallback sem seek duplicado. Pedido→fontes→URL→pronto→primeiro quadro→ponto salvo pode ser comparado na mesma fonte/aparelho. Ganho de 3–6 s citado no handoff é estimativa/medição anterior, não resultado desta implementação. Conferência paralela genérica da primeira mídia foi adiada por efeitos do GET Range no debrid e ausência de cancelamento por chamada. A fonte da última sessão já usa o caminho paralelo integrado.
5. **Pré-busca MKV — diagnóstico corrigido.** O log deixa de produzir tempo unsigned falso quando `agora` antecede o timestamp criado no mesmo quadro. A espera de até 4 s permanece; ela não foi reduzida como parte da correção de telemetria. Conferir legenda ASS embutida na mídia real ainda é teste físico específico.
6. **Relatos visuais/reprodução — evidência física pendente.** Proporção (#195), cintilação de Continuar assistindo (#144), catálogos ausentes (#195/#197) e localização (#209) precisam do cenário/fonte/configuração/log atual. Não afirmar que todos têm a mesma causa. #158 já recebeu resposta da 1.7.1; aguarda confirmação de reprodução contínua do relator, sem nova resposta nesta rodada.
7. **UX — escolha e merge concluídos.** O dono escolheu `ajustesux`; `f9083b10` integrou a reorganização funcional, conservando enum/defaults e #221. `ajustesvisual` A/B, ícones de apoiador e recursos sociais ficam fora da 1.7.2. Não combinar automaticamente as alternativas visuais nem alterar launcher Android sem escolha e prova próprias.
8. **Pacotes locais concluídos após isolamento da ilha; TVs pendentes.** Versão 1.7.2 alterada localmente em `32e70e48`. LG normal/highcache, WGT, quatro TPKs, dois .so, APK dual ABI e dois JSONs Homebrew foram gerados do núcleo `fc711f6b`, com arte/configuração/assinatura e 13 anexos/checksums conferidos. A ferramenta TPK recebeu conferência adicional `06d915f6`, sem mudar o runtime. Esse conjunto foi reclassificado como intermediário após o achado da ilha. Todos os pacotes foram reconstruídos da mesma fonte `b71471bc`, com conferências das três plataformas e 12 checksums PASS; resultados e hashes ficam em PACOTES.md. Nenhuma instalação desta candidata foi feita. Dono testa C9, TCL e Samsung: antes/depois de dois minutos, fonte vencida, addon lento, troca de perfil, login/toque, proporção e Xtream. Publicação continua dependendo de autorização posterior.

## Evidências dos logs

Atualização somente leitura às 20:37 BRT de 02/10/2026: 118 registros rotulados 1.7.1 (66 Android, 31 TPK, 14 webOS, sete WGT); zero rotulados 1.7.2. Há 30 registros com HTTP5xx, cinco com ícone ausente, 41 sem despedida e nenhum marcador fatal explícito no agregado. Contagens são registros, não sessões ou crashes comprovados. O [roadmap](ROADMAP.md) cruza esta consulta mais recente com issues, branches e sessões; o recorte anterior de 65 registros abaixo fica como histórico da primeira auditoria.

Consulta somente leitura de 65 registros 1.7.1 disponíveis: 40 Android, 11 TPK, 14 webOS; 64 corpos diferentes. Isso não equivale a 64 sessões independentes. 17 registros contêm HTTP5xx; cinco TPKs têm ícone ausente. 36 contêm encerramento sem despedida (27 Android, nove LG), que não prova crash. Nenhum marcador explícito `FATAL EXCEPTION`, `Fatal signal`, `Segmentation fault`, `RuntimeError:` ou `Aborted` encontrado neste recorte. Não declarar estabilidade plena nem regressão a partir das contagens. Nenhum registro 1.7.2 foi usado como validação da nova build. O snapshot completo posterior de 20:39 BRT tem 119 registros/118 corpos, sendo 67 Android; os detalhes e erros de mídia novos estão no ROADMAP.md.

Relatórios automáticos #192 confundem fallback ASS, código zero e sessão sem despedida com erro/crash em algumas conclusões. Validar sessão, versão real do núcleo e versão do pacote/host antes de priorizar cada evento. Dados privados permanecem fora do repositório.

## Otimização por plataforma

- Android: posição inicial Media3, coerência do plano de vídeo/HDR ao sair e voltar, UI adequada à GPU da TCL, toque sem acionamento duplicado; memória limitada e duas ABIs.
- LG: pipeline único, pausa confirmada antes da retenção, corrida catálogo/histórico, comportamento de duas conexões do proxy Xtream; respeitar capacidade de TVs antigas.
- TPK: recursos atualizados junto do núcleo, proporção/single-window e retorno ao app; ABI4/5 sem TLS e com DT_HASH. Mudança no host .NET demanda evidência específica no aparelho.
- WGT: fontes parciais/cache limitado, compatibilidade Chrome69 e ausência de bloqueio da UI; não tentar P2P com sockets nativos indisponíveis.

## Branches soltas

A consulta ampliada desta rodada encontrou refs e sessões novas além das 17 branches ativas deste inventário inicial. Encaminhamento atualizado, incluindo social/Discord e preservações históricas, está no [roadmap](ROADMAP.md). Nenhum merge adicional desses recursos foi autorizado ou executado.

Inventário revisto por ancestralidade + `git cherry` contra `32e70e48`: 17 branches com patches sem equivalência exata. `agente/ajustesux` está integrada e tem zero patches pendentes; aparece abaixo apenas para registrar seu encaminhamento concluído. Contagem não indica que todo patch é funcionalmente novo; cherry-pick adaptado pode aparecer como não integrado. As outras worktrees não foram alteradas nem reavaliadas quanto a mudanças sem commit.

| Branch | Patches sem equivalência exata | Encaminhamento |
|---|---:|---|
| `agente/ajustesux` | 0 | Integrada em `f9083b10`; validação focada e 12 capturas aprovadas no host |
| `agente/ajustesvisual` | 2 | Alternativas A/B fora da 1.7.2; direção funcional `ajustesux` já escolhida |
| `agente/guiaunicode` | 1 | Conserto de gênero Unicode já presente; não reverter busca Spotlight mais nova |
| `agente/i195` | 1 | Canário/porte antigo; preservar e confirmar obsolescência, não mesclar nem apagar agora |
| `agente/i195b` | 1 | Canário/porte antigo; preservar e confirmar obsolescência, não mesclar nem apagar agora |
| `agente/i195c` | 2 | Canário/porte antigo; preservar e confirmar obsolescência, não mesclar nem apagar agora |
| `agente/i195d` | 3 | Canário/porte antigo; preservar e confirmar obsolescência, não mesclar nem apagar agora |
| `agente/i203` | 1 | Canário/porte antigo; preservar e confirmar obsolescência, não mesclar nem apagar agora |
| `agente/icones` | 5 | Fora da 1.7.2; exige decisão e prova do launcher Android |
| `agente/p2p` | 6 | Versão antiga; não mesclar |
| `agente/p2p2` | 13 | 1.8; requer validação de plataforma/cancelamento/disco |
| `agente/plugins` | 14 | Versão antiga; não mesclar |
| `agente/plugins2` | 21 | 1.8; requer validação de plataforma/cancelamento/disco |
| `agente/reacao` | 6 | Fora da 1.7.2; outra sessão, preservar sem merge nesta rodada |
| `agente/socialsrv` | 7 | Fora da 1.7.2; outra sessão, preservar sem merge nesta rodada |
| `agente/socialui` | 9 | Fora da 1.7.2; avançou desde a inspeção inicial, preservar sem merge |
| `feat/tizen4-coop` | 5 | Canário/porte antigo; preservar e confirmar obsolescência, não mesclar nem apagar agora |
| `feat/vidaa` | 7 | Canário/porte antigo; preservar e confirmar obsolescência, não mesclar nem apagar agora |

Socialsrv/socialui têm sete/nove patches sem equivalência exata na revisão atual; socialui tinha seis na inspeção inicial. Na inspeção inicial também havia mudanças sem commit em socialui; essa worktree não foi reaberta nesta revisão. Não assumir estabilidade por número de commits. Nenhuma branch social foi mesclada nesta rodada.

## Issues abertas consultadas

Reconsulta GitHub somente leitura após o merge dos Ajustes: mesmas 20 issues abertas, com números e timestamps iguais ao snapshot inicial desta rodada; nenhum bloqueador novo de escopo identificado. A inspeção inicial não encontrou PR aberta; PRs não foram reconsultadas nesta revisão. #212/#213/#215 já estavam fechadas e #158 respondida em 1.7.1 por outra sessão. Não refazer respostas.

| Issue | Encaminhamento |
|---|---|
| [#222](https://github.com/iqui27/nuvio-native-legacy/issues/222) | Pedido Discord RP: estudar viabilidade/dependência de aplicativo auxiliar; fora do bloqueio de bug TV |
| [#221](https://github.com/iqui27/nuvio-native-legacy/issues/221) | Integrada: addons chegam incrementalmente; merge dos Ajustes preserva prazo de cinco segundos e busca. Confirmação do dono em Samsung pendente |
| [#217](https://github.com/iqui27/nuvio-native-legacy/issues/217) | Portabilidade Linux: sed, conversor de arte, seleção de pacotes e NV_WEBOS; sem PR na inspeção inicial, coordenar antes de duplicar contribuição |
| [#216](https://github.com/iqui27/nuvio-native-legacy/issues/216) | Integrada: toque/arrastar/login; falta validação do dono em dispositivo apropriado |
| [#211](https://github.com/iqui27/nuvio-native-legacy/issues/211) | Startup UK6540: sem log/firmware; aguardando evidência, não declarar corrigida |
| [#209](https://github.com/iqui27/nuvio-native-legacy/issues/209) | Correção publicada; confirmar comportamento/localização no cenário atual |
| [#202](https://github.com/iqui27/nuvio-native-legacy/issues/202) | Correção publicada; 12 capturas da nova UX revisadas no host, com texto escuro no foco claro. Conferir painel/controle físico na TV |
| [#201](https://github.com/iqui27/nuvio-native-legacy/issues/201) | Usuário confirmou primeira fileira; pedido de mais itens TopN é decisão de produto separada |
| [#197](https://github.com/iqui27/nuvio-native-legacy/issues/197) | Catálogo depende de addon/perfil/retorno vazio; pedir cenário/log atual se persistir |
| [#195](https://github.com/iqui27/nuvio-native-legacy/issues/195) | Trailer confirmado corrigido; novo relato inclui proporção e catálogos, além de pedidos de recursos |
| [#192](https://github.com/iqui27/nuvio-native-legacy/issues/192) | Agregador: revisar eventos brutos, não tomar rótulo automático como diagnóstico |
| [#190](https://github.com/iqui27/nuvio-native-legacy/issues/190) | Usuário confirmou resolução; tarefa administrativa pendente |
| [#188](https://github.com/iqui27/nuvio-native-legacy/issues/188) | Correção anterior; confirmar mesmo cenário no TPK atual |
| [#172](https://github.com/iqui27/nuvio-native-legacy/issues/172) | Reorganização funcional dos Ajustes integrada; demais pedidos visuais não foram presumidos como atendidos. Medir fluidez nas TVs |
| [#171](https://github.com/iqui27/nuvio-native-legacy/issues/171) | P2P nativo reservado 1.8 |
| [#158](https://github.com/iqui27/nuvio-native-legacy/issues/158) | Resposta 1.7.1 já enviada; aguardar playback contínuo do relator |
| [#155](https://github.com/iqui27/nuvio-native-legacy/issues/155) | Agradecimento, nenhum defeito para corrigir |
| [#144](https://github.com/iqui27/nuvio-native-legacy/issues/144) | UX/paridade e relato de cintilação no Continuar assistindo: reproduzir |
| [#135](https://github.com/iqui27/nuvio-native-legacy/issues/135) | VIDAA experimental, fora de1.7.2 |
| [#134](https://github.com/iqui27/nuvio-native-legacy/issues/134) | Plugins reservados1.8 |

## Critérios para fechar o escopo

Dados de outro perfil não aparecem; atualização TPK não depende de arte inexistente; falhas Nuvio conservam dados e fila; retomada e fallback funcionam; formato de vídeo e legendas não regridem. Medir melhoria de tempo por etapa na mesma fonte e aparelho, sem prometer instantâneo na abertura de mídia nova. Testes do código alterado + i18n e testes curtos do dono; não repetir suíte longa.
