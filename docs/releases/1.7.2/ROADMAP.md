# Auditoria da 1.7.2: logs, issues e trabalhos de outras sessões

**Mapa entre versões:** [roadmap geral — 1.7.2, 1.7.3 e 1.8](../../ROADMAP.md). Este arquivo conserva a auditoria detalhada da candidata.

## Atualização posterior — 02/10/2026 21:39 BRT

O dono rejeitou o mockup da ilha; as quatro evoluções propostas ficam fora da fila e não são dependência de release. A ilha existente e o bugfix de identidade integrado permanecem. A nova consulta GitHub encontrou23issues abertas: #223 (tela preta Android/TCL, relato1.7.0), #224 (LG65UT7300LA/webOS25, falha de arranque1.7.1) e #225 (LG50NANO75SPA, falha após atualização1.7.1) foram adicionadas à triagem de arranque/login. #224/#225 não trazem logs. Não inferir causa única nem correção pela conferência dos pacotes. Destinos futuros agora estão explícitos no roadmap geral;1.7.3 é proposta, plugins/P2P na1.8 seguem a direção já escolhida.

Revisão de 02/10/2026, com evidências coletadas entre 20:34 e 20:48 BRT e status final dos pacotes atualizado nesta rodada. Trabalho local em `release/1.7.2`, `/private/tmp/nv-172`, sobre a 1.7.1 publicada (`2bc9659b`). Fonte auditada inicialmente: `fc711f6b`; nova candidata após isolamento da ilha: `b71471bc20e36806de9665145e4898c5e8e67884`. O escopo pode receber escolhas posteriores do dono; este documento recomenda encaminhamentos e não autoriza novos merges ou publicação.

A prioridade é terminar a candidata já aprovada, validar os cenários reais e resolver defeitos reproduzidos. Fechar todas as issues não é critério de qualidade: há pedidos de produto, agradecimento, portabilidade e relatos que ainda dependem de dados. Código integrado, pacote gerado e comportamento observado na TV são evidências distintas.

Referências locais: [plano aprovado](PLANO.md), [validação focada e casos das TVs](VALIDACAO-LOCAL.md), [pacotes e origem das builds](PACOTES.md), [notas em rascunho](NOTAS.md). O handoff externo da 1.7.1 foi lido; as decisões posteriores de retenção de dois minutos e Ajustes funcionais prevalecem sobre suas alternativas iniciais.

## Atualização após a análise da ilha

O [inventário e mockup da ilha](ILHA-PROPOSTA.md) revelaram defeito P2 confirmado por callchain: cartão, modal/pedido e conteúdo animado não estavam todos ligados à conta/perfil. A sessão retida já bloqueia a identidade anterior; o fallback do cartão podia usar o título/episódio antigo no contexto novo. Correção focada `b71471bc` aprovada com ASan/UBSan e revisão independente, sem adicionar as funcionalidades propostas no mockup. Invalida também modal/pedido/arte/voo e rejeita instância antiga mesmo quando o novo perfil vê o mesmo episódio; reempacotamento concluído: três plataformas PASS e 12 checksums aprovados. O conjunto de 13 anexos `fc711f6b` passou na conferência técnica, mas está preservado como intermediário após o fix; a nova candidata deriva de `b71471bc`. A auditoria anterior abaixo mantém seus horários e evidências.

## Inclusão posterior do screensaver

`8a49956` autorizado pelo dono para 1.7.2. Integrado em `857f83e3`, ativação corrigida em `11835ffa`; teste focado e i18n PASS. API ainda sem confirmação física; novo empacotamento pendente. O lote `b71471bc` auditado abaixo não inclui esse fix. Detalhes em VALIDACAO-LOCAL.md.

## Estado confirmado nesta revisão

- GitHub, consulta somente leitura às 20:34 BRT: **20 issues abertas e nenhuma PR aberta**. Foram relidos os últimos comentários de #158, #195, #201, #217, #222, #144, #192 e #190. #212/#213/#215 já foram tratadas pela sessão da 1.7.1; não duplicar respostas.
- D1, consulta somente leitura às 20:37 BRT: **118 registros rotulados 1.7.1**. Uma segunda captura às 20:39 BRT já tinha **119 registros e 118 corpos distintos**. Não são 118 ou 119 sessões/aparelhos independentes. Não havia registro rotulado 1.7.2 em nenhuma das duas consultas.
- Inventário Git amplo: **29 refs com ao menos um patch sem equivalência exata**, incluindo as **17 branches do inventário anterior**, duas features recentes e dez refs históricas/experimentos/preservações. Esses grupos se sobrepõem em commits; não somar seus patches como trabalho novo.
- `abrirrapido`/`ilhavolta`/`i221`, `i216` e `ajustesux` já estão na release. Fila durável de addons (`d8402e54`) e temporários explícitos no SSD (`fc711f6b`) foram fechados nesta rodada; a revisão e os testes focados da fila passaram. O conjunto dessa fonte foi preservado como intermediário. Os 13 anexos atuais passaram na geração/conferência e nos checksums consolidados a partir de `b71471bc`, conforme PACOTES.md. A revisão de ferramenta `06d915f6` complementa o guard standalone TPK, consumindo toda a lista para evitar SIGPIPE; fixture com 12 mil entradas e conferência robusta dos ZIPs finais passaram. Ela não altera o runtime; está incluída na fonte da nova candidata.
- Não houve push, tag, publicação, comentário, deploy de servidor nem controle/instalação em TV por esta auditoria.

## Logs: o que mudou e o que ainda não está demonstrado

O recorte anterior de 65 registros continua válido como auditoria anterior; a tabela abaixo descreve a captura de 20:39 BRT. Os dados brutos permaneceram privados fora do Git. As consultas D1 retornaram `rows_written=0`, `changes=0` e `changed_db=false`.

| Plataforma rotulada | Registros 1.7.1 | Registros com HTTP 5xx | Ícone do celular ausente | Sem despedida | Marcador fatal explícito |
|---|---:|---:|---:|---:|---:|
| Android | 67 | 11 | 0 | 31 | 0 |
| Samsung WGT (`tizen`) | 7 | 6 | 0 | 0 | 0 |
| Samsung TPK | 31 | 11 | 5 | 1 | 0 |
| LG webOS | 14 | 2 | 0 | 9 | 0 |
| **Total** | **119** | **30** | **5** | **41** | **0** |

Um registro pode conter várias ocorrências e várias tentativas. Ausência de `FATAL EXCEPTION`, `Fatal signal`, `Segmentation fault`, `RuntimeError:` ou `Aborted` não comprova estabilidade; sessão sem despedida não comprova crash. Nem todo HTTP 5xx é do Nuvio: nas linhas novas de rede, a rota/origem nem sempre está identificada. Não atribuir todos os 30 registros ao `api.nuvio.tv` ou à sincronização.

| Evidência | Interpretação e próximo passo |
|---|---|
| 13 registros adicionais após o recorte anterior têm HTTP 5xx; há 502/504 em WGT/TPK e 503/504 em Android | Mantém prioridade de resiliência, backoff e mensagem de falha. Separar conta, addons, arte, trailers e fornecedor antes de corrigir uma rota. A proteção do cliente não demonstra recuperação do serviço remoto. |
| Cinco registros TPK com `aj_smartphone.png` ausente; nenhum adicional após o recorte anterior | Fallback embutido já integrado. O registro 18898 demonstra núcleo 1.7.1 por memfd, host api11 e arte ausente, mas não informa a versão exata do pacote antigo. Confirmar atualização de núcleo sobre host anterior no aparelho. |
| Android 19066: seis erros de player 3003; 19107: 2004 e 3003; 19164: 3003 | São falhas reais de reprodução a investigar, não marcadores de crash. Os dois primeiros têm marcador `[tv]` 1.7.1; o corpo de 19164 não tem banner de versão. Distinguir tentativa no diagnóstico Live TV de filme aberto e conferir fonte/Content-Type/cabeçalhos/fallback. Não presumir três usuários diferentes. |
| Recorte 16626–16630, recebido às 06:49 UTC, horário local informado 09:49, núcleo 1.7.0 | Cenário compatível com #158: HLS obtém segmentos, proxy encerra em um ou dois segundos; conexão subsequente termina sem segmentos. A TV chega a `loadCompleted`/`currentTime`/`endOfStream`. Corresponde ao mecanismo da segunda conexão corrigido na 1.7.1. Falta confirmação de playback contínuo na versão atual. Associação por horário e cenário é provável; o corpo não identifica literalmente o modelo C4. |
| Busca de logs LG deste dia por UK6540 sem correspondência | #211 continua sem firmware/log de arranque identificado. Ausência no recorte não demonstra que o aparelho funciona ou que não houve falha. |

Na referência oficial do [Media3 PlaybackException](https://developer.android.com/reference/androidx/media3/common/PlaybackException), 3003 corresponde a container/funcionalidade de container não suportado; 2004 a status HTTP inesperado. Isso classifica a falha, não identifica sua causa neste app. O zero que acompanha esses eventos é argumento auxiliar da ponte; não transforma 3003 em sucesso. Reproduzir com amostra autorizada e verificar a versão real do núcleo, pacote, ponte e alvo antes de alterar parser ou player.

O agregador [#192](https://github.com/iqui27/nuvio-native-legacy/issues/192) mistura versões, eventos de diagnóstico, fallback ASS, códigos auxiliares e encerramentos. Seus rótulos de regressão/crash são hipóteses; o roadmap usa eventos brutos e confirmações do relator.

## Prioridades para terminar a candidata

P0 impede aceitar esta candidata; P1 exige investigação ou prova antes de declarar aquele cenário resolvido; P2/P3 é produto, manutenção ou plataforma futura. Um P1 sem reprodução não implica merge especulativo.

| Prioridade | Origem: issue/log/sessão/branch | Evidência / hipótese | Decisão de merge | Dependência e validação |
|---|---|---|---|---|
| P0 | Fila offline; sessão atual; `d8402e54` | Edição de addons após push500 antes só sobrevivia em RAM; precisava sobreviver ao reinício | **Sim agora: integrado**, não repetir merge | ASan/UBSan e revisão passaram. Pacotes finais devem excluir snapshot e temporário pessoal. Conta/perfil A→B→A, ACK exato, edição durante POST, disco indisponível, logout. |
| P0 | Build local; `fc711f6b` | `mktemp` sem template no Mac extraiu no disco interno mesmo com TMPDIR no SSD | **Sim agora: integrado** | Gerar os 13 anexos da mesma fonte; versão, assinatura, ABI, configuração, hashes e ausência de arquivos pessoais. Candidata intermediária `32e70e48` não vale como final. |
| P0 | Dono: C9, Samsung e TCL | Teste de host e pacote não comprovam renderer/player/controle da TV | **Depende de prova física**, sem patch presumido | Matriz abaixo; registrar fonte/build/aparelho/firmware/tempo. Não publicar enquanto os casos exigidos pelo dono estiverem pendentes. |
| P1 | [#158](https://github.com/iqui27/nuvio-native-legacy/issues/158), logs 09:49 | Pausa após segundos, proxy com segunda conexão, resposta 1.7.1 já enviada | **Já integrado na 1.7.1**; novo patch depende de reprodução atual | C9 com live contínuo, troca de canal e diagnóstico; confirmação no LG C4 do relator continua pendente. Não fechar pelo teste da C9 sozinho. |
| P1 | [#221](https://github.com/iqui27/nuvio-native-legacy/issues/221), `i221` dentro de `abrirrapido` | Samsung Tizen5 esperava 20–25s por fontes; entrega incremental já integrada | **Já integrado** | Samsung TPK4/5 e 6+, addon rápido+addon lento, Play automático e prazo padrão de 5 s. WGT: fluidez da UI durante requisição. |
| P1 | Retorno ao filme/ilha, `abrirrapido`/`ilhavolta`, posição inicial Android | Retenção119/120 s e preparação com ponto salvo passam em fixtures; ganho real não medido | **Já integrado**, manter dois minutos | C9/TCL/Samsung antes e depois120s; fonte vencida, fallback, episódio/perfil, HDR e passagem por outro app. Comparar marcos da mesma mídia/aparelho. |
| P1 | Erros Android novos 19066/19107/19164 | 3003/2004 durante tentativas de mídia; causa não demonstrada | **Depende de reprodução**, não mesclar um player alternativo | Classificar diagnóstico/live/VOD, identificar media/cabeçalhos/HTTP sem expor URL pessoal, verificar container, mensagem e próxima fonte na TCL. |
| P1 | Catálogo/histórico; [#197](https://github.com/iqui27/nuvio-native-legacy/issues/197), parte de [#195](https://github.com/iqui27/nuvio-native-legacy/issues/195) | Corridas/identidade foram corrigidas; catálogo vazio, escolha automática ao rolar e configurações ainda podem ter causas distintas | **Correção de consistência já integrada**; restante depende de cenário | Mesma conta/perfil/addon e configuração do relator; sete fileiras, rolar sem selecionar; troca de perfil durante resposta. ASan/TSan focados aprovados. |
| P1 | [#195](https://github.com/iqui27/nuvio-native-legacy/issues/195), proporção | Relator confirmou trailer corrigido e agora relata proporção95% dos casos | **Depende de prova**, não reintroduzir canários antigos | Samsung TPK6.5+/4/5 com a mesma mídia e cada modo. ROI fora da tela já expôs Home; recorte da origem permanece limitado pelo backend. |
| P1 | [#188](https://github.com/iqui27/nuvio-native-legacy/issues/188) | Áudio com Home atrás do player em TPK antigo; correção single-window já publicada | **Já integrado**; patch só com log/build atuais | Samsung S90D/cenário do relator; trailer → filme, voltar de segundo plano, plano de vídeo/GL. |
| P1 | [#211](https://github.com/iqui27/nuvio-native-legacy/issues/211) | UK6540 volta à entrada no arranque; versão/firmware/log desconhecidos | **Depende de dados**, não marcar resolvida | Log de bootstrap, versão do IPK instalado, firmware e alvo; comparar normal/highcache se pertinente. C9 não substitui prova no UK6540. |
| P1 | ASS embutida/sonda/HTTP; logs e cliente atual | Fallback de legenda não significa falha de vídeo; sonda pode ter efeitos de rede/cota | **Endurecimento da sonda integrado**; paralelo genérico adiado | MKV com ASS em faixa embutida na C9, TPK e TCL. HTTP 403 com cabeçalhos, redirecionamento, timeout e fonte vencida; WGT preserva decisão de AVPlay quando XHR não pode enviar cabeçalho. |
| P2 | [#144](https://github.com/iqui27/nuvio-native-legacy/issues/144) | Cantos inferiores cortados e cintilação no Continuar assistindo; Ajustes são outro ponto da issue | **Ajustes integrados**; cintilação depende de reprodução | Mesmos cartazes/layout/escala; navegar verticalmente para CW, capturar quadro e perfil GPU na TV. Não contabilizar redesign dos Ajustes como correção do poster. |
| Fora da fila | Mockup da ilha; agente `island_ux_mockup` | Dono rejeitou o mockup e pediu continuidade no roadmap | **Não integrar as quatro evoluções propostas** | Preservar artefato como histórico. Ilha existente e correção de identidade permanecem; não são uma nova proposta visual. |
| P2 | [#216](https://github.com/iqui27/nuvio-native-legacy/issues/216), `i216` | Toque/perfis e login manual, antes ausência de suporte | **Já integrado** | Android celular/tablet para toque e IME; TCL para regressão do D-pad/login. Simulação de ponteiro no Mac não comprova toque físico. |
| P2 | [#202](https://github.com/iqui27/nuvio-native-legacy/issues/202) e [#209](https://github.com/iqui27/nuvio-native-legacy/issues/209) | Contraste e localização publicados; UX funcional teve12 capturas host aprovadas | **Já integrado/publicado**, confirmar cenário | C9: acento matching sem branco sobre branco; descrição/título de série em português, TMDB+addon e perfil corretos. |
| P2 | [#217](https://github.com/iqui27/nuvio-native-legacy/issues/217) | Quatro melhorias de hostLinux; autor já foi convidado a mandar PRs, nenhuma aberta | **Depende da contribuição e revisão** | Não duplicar pacote de patches. GNU/BSD sed, fallback ffmpeg, subset TPK e NV_WEBOS: Linux/Mac builds, preprocessing por alvo, C9 e TPK api11. Não exige deploy. |

## Issues de produto e administração

| Issue | Encaminhamento concreto | Quando considerar |
|---|---|---|
| [#222 Discord](https://github.com/iqui27/nuvio-native-legacy/issues/222) | Há implementação nova `feat/discord-presenca`; o diagnóstico inicial de inviabilidade está superado por esse trabalho, mas ainda falta prova nas TVs e revisão do contrato/credenciais/OAuth. Nenhum merge autorizado nesta rodada. | Versão posterior ou ampliação explícita de escopo, após matriz de plataforma e retomada/desvinculação por perfil. |
| [#201 TopN](https://github.com/iqui27/nuvio-native-legacy/issues/201) | Relator confirmou primeira fileira corrigida. Mais itens Top10/12/18/24 é escolha de produto e layout, separada do bug. | Definir limite/paginação e custo em TVs antigas antes de implementar. |
| [#195 pedidos](https://github.com/iqui27/nuvio-native-legacy/issues/195#issuecomment-5960258625) | Thumbnails na barra, logo do título, ajuste de legenda e media server precisam triagem própria; não agrupá-los à correção de proporção. | Proposta com custo/compatibilidade e validação por recurso. Plugins seguem na 1.8. |
| [#172 ideias visuais](https://github.com/iqui27/nuvio-native-legacy/issues/172) | Ajustes funcionais escolhidos e integrados; outras ideias do fork não foram presumidas como aprovadas. | Revisão seletiva de fluxo/legibilidade, baseada em captura atual e medida de GPU. |
| [#171 P2P](https://github.com/iqui27/nuvio-native-legacy/issues/171) | Usar `agente/p2p2` como candidato futuro, não a branch antiga. Disco, cancelamento e TPK real ainda limitam aceitação. WGT sem socket fica fora. | **1.8**, decisão já tomada. |
| [#134 plugins](https://github.com/iqui27/nuvio-native-legacy/issues/134) | Usar `agente/plugins2`, não a versão antiga. C9 tem medidas anteriores, mas TPK/WGT real e scraper travado ainda não validados. | **1.8**, com cancelamento por chamada, limite de memória/tempo e perfis. |
| [#135 VIDAA](https://github.com/iqui27/nuvio-native-legacy/issues/135) | Port experimental sem prova em aparelho real; não generalizar resultados de Chrome desktop. | Frente independente, com testador/aparelho/versão de Chromium. |
| [#190](https://github.com/iqui27/nuvio-native-legacy/issues/190#issuecomment-5956133326) | Relator confirmou título/episódio resolvido. Resta administração da issue, sem novo patch. | Só com autorização de resposta/fechamento. |
| [#155](https://github.com/iqui27/nuvio-native-legacy/issues/155) | Agradecimento; não há defeito a corrigir. | Nenhuma dependência de release. |
| [#192](https://github.com/iqui27/nuvio-native-legacy/issues/192) | Melhorar critérios de triagem e separar usuário/sessão/tentativa/diagnóstico/versão real antes de abrir novos bugs. | Manutenção de observabilidade; nenhuma nova postagem nesta rodada. |

## Sessões e unidade correta de integração

Os títulos Codex abaixo foram preservados exatamente como retornados pelo app. Estado do chat e timestamp da lista não provam que um patch novo foi feito: foi conferida a relação do Git. As sessões Social/Discord não apareceram como chats identificáveis na lista consultada; o registro externo no M3 e os commits permitem inventariar seu trabalho, sem inventar título ou autorizar contato.

| Sessão / trabalho | Estado observado e evidência | Unidade de integração | Recomendação |
|---|---|---|---|
| **Otimizar e corrigir o código** — chat atual | Fonte final `b71471bc`; fila `d8402e54` e isolamento da ilha fechados, 13 anexos finais aprovados; agentes `queue_finish`/`queue_review` pertencem a esta rodada | Commits locais da release; pacote final deriva da fonte congelada | Pacotes fechados; faltam os gates físicos. Não criar merge fictício para trabalho feito diretamente na release. |
| **Review docs/ajustes-ux.md** | Último turno retornado confirma selo jade/check branco em `2fbcf53b`; worktree limpa às 20:39; branch é ancestral da release | `agente/ajustesux`, já mesclada em `f9083b10` | Não mesclar de novo nem tratar o selo como patch solto. Outras propostas do chat exigem commit/prova própria. |
| **Polir a UI para nível profissional** | Turnos retornados são históricos: relatam UI/fontes/What’s New enviados em `fc2025a`; lista mostrava `notLoaded` e atualização recente | Commits históricos, já incorporados na base; não há patch novo demonstrado por esse retorno | Evitar duplicação. Uma data recente na lista não torna a proposta histórica uma feature pendente. |
| Sessão externa Social, nome não identificado pelo app | `feat/social` em `46e9b24f`, commit 20:00 BRT, worktree limpa; agrega `socialsrv`+`reacao`+`socialui` | **`feat/social` é agregadora**;23 patches novos, incluindo os22 das três branches e a cola | Fora da 1.7.2 por decisão atual. Se aprovado depois, revisar/mesclar uma vez a agregadora, preservando a release mais nova. |
| Sessão externa Discord, nome não identificado pelo app | `feat/discord-presenca` em `6b6bfe14`, commit 19:48 BRT, worktree limpa; um patch sobre1.7.1 | Patch/branch Discord, com alterações C/rede/build/ajustes e arquivos OAuth por perfil | Fora do escopo atual; depende de revisão de integração e prova real. Não substituir a release por árvore antiga nem copiar pacote de outra sessão como1.7.2. |

**Coexistência com servidor e TVs de outras sessões.** A consulta somente leitura `wrangler deployments list` confirmou o Worker `nuvio-recomendacoes` com versão `23314845-8ba9-4471-83ca-1e6c713db44f` a 100%, publicado às 20:06 BRT. Isso coincide com o registro externo do Social. A fonte desta release ainda tem o servidor anterior: um deploy indiscriminado daqui pode retirar as rotas novas. Não há necessidade de deploy para os pacotes 1.7.2 e nenhum foi feito aqui.

O registro externo no M3 também relata APK Social instalado na TCL e aprovação do dono; essa instalação não foi checada nesta auditoria. Portanto, o pacote atualmente presente na TCL não está comprovado como o APK estável puro. O que está demonstrado nesta rodada é que **nenhuma build 1.7.2 desta release foi instalada ou validada na TCL**. Para teste futuro, conferir pacote/núcleo/host reais antes de interpretar o resultado. O relato externo de Discord diz instalação Android e uma build C9 anterior ao rebase, sem presença observada na TV; não equivale a validação desta candidata.

## Destino posterior da Glass UI

O dono indicou `feat/glass-ilha` para 1.8. Ponta `c5269ed4`, baseada em `feat/social` (`46e9b24f`), com 12 commits não-merge de camada visual adicionais. Inclui Fontes, menu/rail, Salvos/Social, menus, Spotlight, Agenda, Biblioteca e Perfil/stats; não é apenas relógio. Sem merge na 1.7.2. Revisar delta visual após Social, preservar os fluxos mais novos e validar por plataforma. Conteúdo/dependências no [roadmap geral](../../ROADMAP.md). O mockup HTML rejeitado continua fora. Esse é um acréscimo ao inventário histórico abaixo, não uma atualização retroativa da contagem 29.

## Inventário de branches soltas

Método: ancestralidade e `git cherry` contra a fonte da release. `+` significa patch sem equivalência exata; `-` significa equivalência de patch, mesmo sem o mesmo hash. Patch adaptado pode aparecer como `+` apesar de correção semanticamente presente. Worktree limpa significa apenas ausência de alterações sem commit no momento da leitura. Não significa teste ou autorização de merge.

### As 17 branches do inventário anterior

| Branch | Patches `+` | Decisão e motivo |
|---|---:|---|
| `agente/ajustesvisual` | 2 | **Não agora**: A/B fora; Ajustes funcionais já escolhidos. |
| `agente/guiaunicode` | 1 | **Não**: gênero Unicode já corrigido por adaptação na 1.7.1; não reverter leitor/busca mais novos. |
| `agente/i195` | 1 | **Não agora**: canário de janelas antigo; single-window publicado. Preservar até conferir obsolescência. |
| `agente/i195b` | 1 | Mesmo encaminhamento; não combinar estratégias antigas de janela. |
| `agente/i195c` | 2 | Mesmo encaminhamento; dependia de host/api e canário específico. |
| `agente/i195d` | 3 | Mesmo encaminhamento; canário6 precede correção final. |
| `agente/i203` | 1 | Mesmo encaminhamento; host antigo, sem novo defeito reproduzido. |
| `agente/icones` | 5 | **Outra versão/decisão**: atividade-alias afeta launcherAndroid; exige aceitação e prova próprias. |
| `agente/p2p` | 6 | **Não**: supersedida por `p2p2`; preservar, não mesclar. |
| `agente/p2p2` | 13 | **1.8**: disco/cancelamento/TPK real; WGT não suporta motor socket. |
| `agente/plugins` | 14 | **Não**: supersedida por `plugins2`. |
| `agente/plugins2` | 21 | **1.8**: já compartilha #221; não duplicar fontes incrementais. Provar TPK/WGT e cancelamento. |
| `agente/reacao` | 6 | **Outra versão/decisão**: já ancestral de `feat/social`; não merge isolado duplicado. |
| `agente/socialsrv` | 7 | **Outra versão/decisão**: ancestral de `feat/social`; servidor externo já avançou, deploy separado. |
| `agente/socialui` | 9 | **Outra versão/decisão**: ancestral de `feat/social`; worktree agora limpa. LG/Samsung ainda sem prova do Social novo. |
| `feat/tizen4-coop` | 5 | **Não agora**: port/canário histórico; pacote TPK4/5 atual tem contrato distinto. |
| `feat/vidaa` | 7 | **Frente independente**: precisa TV real; não bloquear a 1.7.2 das plataformas estáveis. |

### Duas features novas desde o inventário anterior

| Branch | Patches `+` | Decisão |
|---|---:|---|
| `feat/social` | 23 | **Depende de ampliação de escopo**; agrega três branches acima, não são23 patches adicionais a elas. Cliente antigo/servidor novo deve coexistir; não redeployar servidor anterior. |
| `feat/discord-presenca` | 1 | **Depende de ampliação de escopo/revisão**. Mac teve prova de protocolo segundo o commit; TV ainda não comprova vinculação, presença, retomada e desvinculação. |

### Dez refs históricas / experimentos / preservações

| Ref | Patches `+` | Encaminhamento |
|---|---:|---|
| `canario/tpk-audio-foco` | 1 | Preservar; canário antigo sem solicitação atual de merge. |
| `canary/tpk-youtube-retoma` | 1 | Preservar; não reintroduzir estratégia antiga de áudio/resource-conflict sem reprodução. |
| `experimento/universo-2026` | 36 | Revisão futura por patch. Mistura exploração de dados/UX/corridas antigas; não merge amplo. |
| `nacl/app-completo` | 2 | Rota experimental antiga; não substituir TPK4/5 atual. |
| `spike/nacl-tizen45` | 1 | Compartilha experimento NaCl; não somar como implementação independente. |
| `webos3` | 5 | Há ainda um patch equivalente `-`; alvo antigo precisa revisão específica, não merge geral. |
| `worktree-agent-a03fb2e226f125a82` | 2 | Mesmo conteúdo de `nacl/app-completo`; preservação, não feature nova. |
| `worktree-agent-a53ac8d8d82f17106` | 1 | WIP preservado de agente antigo; conteúdo não auditado, não mesclar/apagar. |
| `worktree-agent-a62e3f13ac3f493e5` | 1 | WIP preservado de agente antigo; conteúdo não auditado, não mesclar/apagar. |
| `worktree-agent-af383a462c2f04823` | 1 | Mais um patch equivalente `-`; correção antiga de catálogo/limite precisa comparação semântica antes de uso. |

As refs `abrirrapido`, `ilhavolta`, `i221`, `i216`, `ajustesux`, `offline`, `i212`, `i213a`, `i213b` e `quedas171` têm sua integração conferida por ancestralidade/patch; não pertencem à fila de merges. Registros worktree marcados `prunable` indicam checkout ausente, não autorização para prune ou apagar commits. Nenhuma worktree de outra sessão foi modificada.

## Sequência recomendada e dependências

1. **Candidata local autorizada concluída.** Fila durável, scripts e isolamento da ilha estão em `b71471bc`; geração, conferência e checksums dos 13 anexos passaram, com hash de origem igual. Usar esse conjunto na rodada física. Pacotes `32e70e48` e `fc711f6b` ficam identificados e separados como intermediários.
2. **Dono faz a rodada curta das TVs.** Conferir package/core/host antes do teste; uma pessoa/agente por TV. Observar duas fronteiras, 119/120 s, fonte vencida, perfil, Ajustes, live e legenda. Registrar falhas com etapa e horário, sem repetir suíte longa.
3. **Investigar os P1 pendentes por cenário.** Android 3003/2004, Samsung proporção/catálogos, LG startup UK6540 e CW cintilação devem ganhar reproduções específicas. Alterar apenas a causa demonstrada; voltar aos focados da área alterada e ao pacote afetado. Não juntar hipóteses diferentes numa correção ampla.
4. **Reavaliar escopo com o dono.** Social e Discord têm trabalho concreto novo, mas continuam fora desta candidata. O mockup da ilha foi rejeitado e saiu da fila. Se escolhidos, revisar cada unidade sobre a release atual, conservar enums/defaults/identidade/gerações e refazer a matriz afetada. A agregadora Social evita três merges redundantes. Não mexer em servidor ativo como efeito colateral.
5. **Preparar 1.8.** Plugins e P2P exigem cancelamento por chamada, limite de disco/memória e prova por backend. A primeira abertura de fonte em paralelo também depende de sonda cancelável e entendimento do efeito do GETRange sobre o debrid; a fonte guardada já tem caminho paralelo.
6. **Publicação e respostas só depois.** Após resultados físicos e autorização do dono, usar notas e tabela de arquivo por sistema, publicar anexos conferidos e então responder/administrar issues realmente resolvidas. Nenhuma autorização de envio é inferida do pedido de roadmap.

## Matriz curta por plataforma

| Plataforma | Casos necessários | Aceitação / limites |
|---|---|---|
| **LG C9 — IPK normal/highcache** | Ilha/retorno119/120 s; fonte expirada; live Xtream contínuo e troca de canal; MKV com ASS **em faixa embutida**; perfil/logout durante sync; contraste/localização | Sem novo load na retenção válida; após prazo pipeline liberado e fallback válido; live sem corte da segunda conexão; memória compatível com aparelho. C9 não certifica UK6540 ou C4. |
| **Samsung WGT** | Fontes incrementais, UI durante addon lento/timeout, HTTP 403 com cabeçalhosAVPlay, Ajustes/busca/Voltar, perfil/fallback | Chrome69, orçamento de heap, sem travar navegação indevidamente. XHR síncrono ainda tem limites de timeout/cancelamento; não afirmar garantia ausente. |
| **Samsung TPK4/5** | #221 no Tizen 5; núcleo por memfd, atualização sobre host anterior/ícone, proporção/modos, saída/retorno e trailer → filme | ABI sem TLS/PT_TLS/relocaçõesTLS, DT_HASH presente e arte essencial disponível. Build não prova carga na TV; host4/5 precisa aparelho dessa geração. |
| **Samsung TPK6+ /6.5 /api11** | Plano single-window, proporção#195, áudio sem vídeo#188, retorno de segundo plano, fonte/cabeçalho, Ajustes e ícone | Distinguir host/API e núcleo. Não reintroduzir ROI que revela Home; backend pode não oferecer todos os recortes. Uma Samsung moderna não certifica todas as APIs. |
| **Android/TCL — APK dual ABI** | Ponto salvo aceito na preparação/fallback, live sem seek, HDR/troca deapp, retenção, fonte vencida,3003/2004, perfis | Medir pedido→fontes→URL→pronto→primeiro quadro→ponto salvo na mesma mídia; sem seek duplicado e sem resposta de perfil antigo. Confirmar pacote presente após builds de outras sessões. |
| **Android celular/tablet** | Toque, arrastar, seleção de perfil, login por e-mail, teclado, tamanho/orientação |  #216 exige toque real. Teste D-pad da TCL não substitui esse caso. |

Não há medição de latência/fps/memória da nova candidata em TV nesta auditoria. Aproveitar o hardware exige orçamento e medidas por backend; pré-cache genérico não é automaticamente seguro nem torna nova mídia instantânea.

## Resultado esperado da próxima decisão

Candidata 1.7.2 com origem e anexos verificáveis, casos essenciais testados pelo dono, defeitos restantes identificados por cenário e roadmap das features separado da estabilização. Social/Discord só entram após escolha explícita e prova; plugins/P2P permanecem na 1.8. Nenhuma contagem de issue ou ausência de marcador fatal será usada para prometer 100% de correção.
