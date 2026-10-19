# Ilha do relógio — inventário e proposta

**Decisão posterior do dono:** mockup rejeitado. As quatro evoluções abaixo não estão na fila de integração. Preservado como inventário/histórico; a ilha existente e seu bugfix de identidade permanecem. Continuidade em [roadmap geral](../../ROADMAP.md).

Proposta UX local, solicitada pelo dono em 02/10/2026. O inventário abaixo foi feito na release 1.7.2 `fc711f6b` (referências de linhas desse snapshot). A correção de identidade foi integrada depois em `b71471bc`. Este documento e o mockup não alteram o runtime ou os pacotes. O lote de pacotes `fc711f6b` passou a intermediário durante a revisão final de isolamento do cartão descrita abaixo. Nenhuma TV foi controlada ou usada como evidência de desempenho.

A recomendação é fazer da ilha o ponto de continuidade da sessão: mostrar quando a retomada está pronta, manter essa ação estável e oferecer contexto somente ao expandir. O teto de retenção permanece em **dois minutos**.

## O que já existe

| Entrada / estado | De onde vem, e o que recebe | Apresentação e saída | Evidência no código |
|---|---|---|---|
| Relógio | Hora local do aparelho em `HH:MM`; consulta no máximo uma vez por segundo | Pílula 52 px de altura; troca do minuto com transição 320 ms | `src/ilha.c:342`, `src/ilha.c:398`, `src/layout.h:600` |
| Aviso curto | Chave, tipo, ícone, texto, duração e indicação de atalho | Um ativo e quatro esperando; chave repetida substitui/renova; fila cheia descarta o mais antigo da fila; padrão 4 s | `src/ilha.c:97`, `src/ilha.c:124`, `src/ilha.h:28` |
| Cores de aviso | Tipos INFO, OK, ERRO e ACENTO | Sucesso verde, erro coral, demais seguem o realce. O tipo não muda a prioridade da fila | `src/ilha.c:329` |
| Central de avisos | Contagem de avisos não lidos; recomendações, agenda, atualização, ocorrência de encerramento e outros tipos pertencem à central | “N aviso(s) novo(s)” e atalho; abrir a central retira a chave da ilha | `src/avisos.c:797`, `src/avisos.c:847`, `src/avisos.c:865` |
| Lembrete de canal | Frase e ícone de sino | Fora do player passa pela ilha; sobre o player usa uma pílula própria | `src/guialembrete.c:131` |
| Atividade | Texto e progresso 0–1 ou indeterminado; renovação a cada quadro | Ponto discreto; se houver número, percentual e linha de progresso; desaparece sem renovação por 400 ms | `src/ilha.c:151`, `src/ilha.c:299`, `src/ilha.c:424` |
| Atualização em segundo plano | Estado e percentual do download/instalação quando o cartão próprio fechou | “Baixando a atualização…”; ao concluir, aviso de 7 s para reiniciar o app | `src/atualizacao.c:1093` |
| Lista de canais | `guia_atualizando_lista()` | “Atualizando a lista de canais…”; é o mesmo slot de atividade | `src/app.c:3697` |
| Conta indisponível | `sync_addons_fora()`: cache disponível ou nenhuma cópia | Aviso uma vez por queda, 9 s; informa o uso dos addons salvos ou a espera pelo servidor; retirado quando volta | `src/app.c:2003`, `src/sync.h:178` |
| Cartão de retomada VOD | IMDb, filme/série, temporada/episódio, nome, sinopse, capa, logo, arte, progresso e minutos restantes | Mesma regra da Home: a partir de 1% e antes do percentual de conclusão. Um único cartão, sem persistência própria; sai após 30 min sem tecla/clique, nova sessão que o substitui ou Fechar | `src/ilhacart.c:46`, `src/ilhacart.c:105`, `src/home.c:5271` |
| Cartão de estreia | Mais recente aviso de agenda ainda não lido; título/episódio, arte e “hoje” | Consulta local a cada 1 s; abrir o título marca o aviso como lido; Marcar como visto dispensa o cartão/aviso, sem significar que o episódio foi assistido | `src/ilhacart.c:79`, `src/ilhacart.c:105`, `src/ilhacart.c:72` |
| Alternância de cartões | Retomada e estreia presentes | Alterna a cada 6 s, somente com relógio permitido na tela | `src/ilha.c:170`, `src/ilha.h:73` |
| Modal expandido | Cópia do cartão selecionado, congelada ao abrir | Arte 480×270, logo/título, episódio, sinopse curta e progresso; Retomar/Assistir, Detalhes, Fechar/Marcar como visto; passagem para Salvos | `src/ilha.c:200`, `src/ilha.c:535`, `src/ilha.c:551` |
| Saída do filme | Sessão interrompida elegível e preferência “Ao sair do player” na Home | Fecha detalhe/menu sem animação concorrente e faz o quadro pousar na mini capa | `src/app.c:3219`, `src/ilha.c:658` |
| Retenção do backend | Pausa confirmada de VOD válido; conta, perfil, URL e episódio de origem | Retoma sem novo load/seek se a identidade ainda corresponde. Descarta em 120 s, troca de perfil/conta/vídeo, pausa perdida ou falha do backend | `src/player.c:253`, `src/player.c:1541`, `src/player.c:1586`, `src/player.c:1601` |

**Prioridade real:** aviso > atividade > cartão > relógio (`src/ilha.c:806`). Não há arbitragem entre atividades: elas compartilham um slot, e a última renovação determina o conteúdo. O Guia atualiza seu texto no desenho do app e pode substituir a mensagem anterior de uma atualização em andamento. Avisos temporários podem aparecer com o relógio desativado; cartões exigem que o relógio possa aparecer.

O relógio/cartões são permitidos na Home pronta e sem camadas que ocupem seu lugar (`src/app.c:3583`). Avisos/atividade usam guardas mais amplas: usuário dentro do app, fora do login/escolha de perfil/registro e **fora do player** (`src/app.c:3693`). A ilha não é o OSD do vídeo.

### Interações que já funcionam

- Na Home livre, AZUL ou CH+ com cartão abre o modal; sem cartão, AZUL abre Salvos, crescendo da pílula quando ela existe. Debounce de 400 ms evita repetição do firmware (`src/app.c:1531`).
- Modal: esquerda/direita escolhem botões, OK aciona, Voltar recolhe; direita após o último botão ou AZUL novamente leva a Salvos. A ação entrega uma cópia do mesmo título/episódio (`src/ilha.c:219`, `src/ilha.c:244`). O modal **já congela sua seleção**, mesmo que o cartão de fundo alterne.
- Magic Remote: clique na pílula abre, hover foca botões, clique fora recolhe. O roteamento do ponteiro também atende toque quando disponível (`src/ilha.c:274`, `src/ilha.c:624`, `src/ilha.c:919`).
- Retomar tenta primeiro a sessão retida; caso não esteja disponível, abre o título/episódio pelo fluxo normal. Fechar descarta também o backend retido (`src/app.c:2273`, `src/app.c:2287`).
- Salvos nasce do retângulo medido pela ilha/modal; durante essa expansão a ilha mede, mas deixa de desenhar. Não há dois painéis sobrepostos (`src/app.c:2261`, `src/ilha.c:181`, `src/ilha.c:894`).

## Como se adapta hoje

| Dimensão | Comportamento implementado | Implicação para evoluir |
|---|---|---|
| Resolução | Canvas lógico 1920×1080; renderer recebe tamanho real do alvo. Pílula 52/64 px, texto de aviso truncado após 760 px; modal 1120×414, limitado às margens laterais | Não há um layout móvel específico na ilha. A proposta usa o mesmo canvas escalado; legibilidade real em 720p ainda precisa de TV |
| Posição | Home padrão usa coluna do conteúdo à esquerda; layout Dinâmica usa topo direito. Ajuste Esquerda/Direita/Automática. Esquerda na Dinâmica fica ao lado do menu. Guia ancora seus avisos à direita | Conservar um único ponto de ancoragem e crescer para dentro da tela, sem cobrir a pílula do menu (`src/ilha.c:288`, `src/ilha.c:840`) |
| Tema | Realce e tinta vêm de Ajustes. Vidro é superfície translúcida simples; opção sólida usa cor escura | Evitar acrescentar blur. O vidro atual da ilha não faz captura/FBO do fundo (`src/gfx.c:1886`) |
| Movimento | Mola da pílula `w=10,z=.72`; modal `w=7.5,z=.8`. Troca de texto por opacidade, texto não acompanha o repique. Política/ajuste reduzido elimina voo/molas | Preserve a preferência de movimento, foco imediato e transições interrompíveis (`src/ilha.c:310`) |
| Voo VOD | 560 ms, curva fechada criticamente amortecida; proporção preservada e só vira capa no trecho final. Arte/veio local e cover; pulso da superfície320ms, sem tremer texto | Sem captura de quadro, cópia de framebuffer ou nova imagem por frame (`src/ilha_voo.h:19`, `src/ilha.c:740`) |
| LG webOS | Vídeo em plano de hardware; voo usa still/fundo já disponível | A imagem de saída pode representar a obra em vez do quadro exato. C9 é referência de orçamento, não evidência de um novoFPS |
| Android | Quando retido, dissolve a partir do plano de vídeo pausado em 150 ms. PixelCopy anteriormente demorou 603–724 ms na TCL, por isso não é usado nesse caminho | Usar recursos que já estão preparados; não tornar snapshot requisito de saída (`src/ilha.c:86`, `src/app.c:3233`) |
| Samsung WGT | Atalho visual representa rocker CH+. Runtime C/WASM usa o mesmo renderer/ilha | Novas funções devem continuar usando o contrato existente de vídeo e eventos; mockup de navegador não prova desempenho do WGT |
| Samsung TPK | Mesmo core nativo e desenho; ramo atual do helper de atalho usa tecla azul genérica | Não presume recursos novos do host .NET nem muda suas capacidades. Adequar indicação do controle ao alvo é uma melhoria de representação, separada do fluxo |
| Entrada por controle | Android indica CH+, WGT rocker CH+, demais tecla azul; escolha do cartão e três ações por navegação horizontal | Não depender de botão colorido em controles sem cores (`src/salvosintro.c:170`) |

As ilhas em resolução diferente no mockup são a mesma geometria lógica escalada. Não há reprodução, AVPlay, uMS ou ExoPlayer no HTML.

## Quatro evoluções recomendadas

| Prioridade | Proposta | Dado/evento reutilizado | Ganho e custo |
|---|---|---|---|
|1 | **Retomada com estado honesto e prioridade estável** | Sessão retida confirmada + identidade do filme/episódio/conta/perfil; progresso do cartão | “Pronto” apenas enquanto confirmada; depois120s fica “Ponto salvo”. VOD retido ganha prioridade sobre estreia. Foco/expansão congelam alternância. Uma consulta local barata; não manter decoder além de 2 min |
|2 | **Pendência local/offline discreta** | Queda já detectada e fila durável de addons da 1.7.2 | Símbolo pequeno, só quando há falha com edição pendente; explicação ao expandir. Não exibir sucesso antes de ACK. Requer uma consulta somente leitura da pendência do perfil, hoje privada em `sync.c:235`; sem polling extra |
|3 | **Preparação após confirmar Assistir** | Pedido de fontes já iniciado, etapas e cancelamento do fluxo do título | “Preparando o filme” pode acompanhar o trabalho sem prender a pessoa num loading enorme; Cancelar/Escolher fonte reutilizam superfícies existentes. Sem falsa porcentagem, sondagem ao hover ou GET adicional em debrid |
|4 | **Comando do canal em miniatura** | `player_mini_ativo()`, identidade fixa do canal, programa já carregado, restauração/Guia/fechamento | Uma entrada para Voltar ao canal, Guia e Fechar miniatura. Mesmo fluxo/decoder; não tentar VOD retido e canal ao vivo simultaneamente (`src/player.h:138`, `src/player.c:1732`) |

A primeira evolução é a mais valiosa e deve vir sozinha antes das demais. A segunda aproveita a correção de persistência já feita. Preparação e PiP precisam de contratos explícitos de evento/ação antes de entrar no runtime, para não virar novas fontes de corrida.

### Regras de produto da proposta

1. Uma sessão dona da ilha, uma ação principal. Avisos informam sem tomar o foco. Erros que afetam essa sessão prevalecem; sucesso rotineiro não a substitui.
2. A disponibilidade “Pronto” deriva de identidade e pausa confirmadas, nunca apenas da existência de um cartão. Tempo restante de retenção pode aparecer ao expandir se necessário, sem countdown frenético em repouso.
3. O filme retido não alterna com estreia a cada 6 s. Foco/expansão preservam o cartão e a ação; outras entradas ficam acessíveis depois da ação principal.
4. Em120s o backend é liberado e o cartão mantém somente o ponto/progresso. O texto não promete retorno instantâneo. A regra atual de 30 min de inatividade do cartão continua independente.
5. Conta/perfil trocado deve invalidar também o cartão, não só o backend. Esse requisito foi implementado em `b71471bc`, após o inventário confirmar a falha por callchain; regressão focada com ASan/UBSan e revisão independente passaram. Não é reprodução de vazamento em TV.
6. A ilha não rouba espaço do player, do teclado ou do QR. Deve retirar sua camada diante dessas superfícies. Atividades concorrentes precisam ser arbitradas por sessão/tipo, não por ordem de chamadas.
7. Nunca expor URL, credencial, nome de conta, endereço de LAN ou código de pareamento no estado compacto.

### Ideias consideradas e deixadas para depois

- **Digitar pelo celular:** útil como confirmação “Texto recebido”, vinculada ao dono do campo, depois de fechar o painel existente. Evitar QR/código permanente na ilha ou uma segunda sessão de entrada. As APIs `celb_aberto/dono/pegar` e `celular_estado` já fornecem o contexto; implementação não proposta para esta release (`src/celbotao.h:45`, `src/celular.h:53`).
- **Busca/Spotlight:** poderia abrir por ação contextual quando não há sessão, mas não duplicar a barra de busca ou competir com Salvos. O Spotlight já é sua própria superfície (`src/spotlight.h`).
- **Próximo episódio:** apenas se o pós-player já tiver o próximo episódio validado; evitar iniciar pedido/precache ao revelar a ilha. Candidato posterior à estabilização da retomada.
- **Rede/FPS/codecs, feed social, chuva de badges, notícias e cronômetros:** não priorizados. Aumentam ruído ou custo e exigem dados externos; a ilha deve ajudar a assistir, não virar um painel de diagnóstico.

## Mockup entregue

Arquivo standalone, assets incorporados, sem dependência de rede:

`/Users/hrocha/.codex/visualizations/2026/10/02/01a0fcd2-c8be-7d62-9157-c53cb050229f/ilha/index.html`

Prévia local enquanto o servidor estiver ativo: `http://127.0.0.1:4187/index.html`. Também abre diretamente pelo arquivo. Imagens ilustrativas reutilizam assets locais do protótipo de Ajustes; não há dados de conta/TV.

- Home com ilha compacta e expandida.
- Cenas: retomada pronta, depois de 2 min, conta indisponível, preparação, canal em miniatura e estreia.
- Controle externo ao quadro da TV: setas/OK/Voltar, CH+/AZUL, seleção da plataforma, superfície sólida e movimento reduzido.
- Teclado: setas, Enter, Esc e C (atalho); foco das ações muda sem animar. O voo simulado usa 560 ms e transform/opacity; não usa backdrop-filter.
- A proposta compacta organiza título e metadado em duas linhas, com 84 px de altura; modal 432 px para acomodar pendência quando existir. Esses tamanhos **não são os atuais 52/64/414 px** e precisam ser avaliados em TV antes de implementar.
- Retomar e Voltar àHome simulam a continuidade visual. Detalhes/Salvos/Fontes/Guia anunciam o destino já existente; não navegam para telas que este mockup não implementa.

Capturas no mesmo diretório:

| Arquivo | Conteúdo |
|---|---|
|`ilha-home-compacta.png` | Home, cartão compacto e disponibilidade pronta |
|`ilha-home-expandida.png` | Arte, estado confirmado, progresso e ação principal em foco |
|`ilha-ponto-salvo.png` | Depois do teto de retenção; sem promessa de retomada pronta |
|`ilha-conta-offline.png` | Explicação de addons pendentes ao expandir |
|`ilha-preparando.png` | Preparação sem percentual fictício e ações para escolha/cancelamento |
|`ilha-canal-miniatura.png` | Cartão de controle e canal em PiP |
|`ilha-prototipo-controles.png` | Visão completa com painel externo de cenas/controle |

### Conferência realizada

Playwright/Chrome local, carga direta `file://`: zero erros JS e zero pedidos HTTP externos. A navegação horizontal troca ações, Esc recolhe, C abre; Retomar abre a simulação do player; saída completa o voo. Movimento reduzido não inicia o voo; 600 px de viewport não produz rolagem horizontal. Relatório `QA.json` e script reproduzível `capture.cjs` no diretório do artefato. Duas capturas foram inspecionadas visualmente: Home expandida e offline. Capturas e testes são **mockup de host**, sem aprovação visual do dono ou prova de desempenho nas TVs.

Antes de integrar qualquer evolução: validar a linguagem/tamanho com o dono; testar identidade após troca de conta/perfil, fim 120 s, pause ACK perdido, concorrência de aviso/atividade, campos/QR, controle sem CH+ e PiP; comparar capturas nativas em C9/Samsung/Android. Sem migração do backend, nova rede por quadro ou aumento do teto de retenção.

## Correção posterior ao inventário

O defeito de identidade do cartão foi corrigido em `b71471bc`, sem implementar as quatro evoluções propostas. Valida conta/perfil antes de eventos/atualização e a instância antes do fallback; limpa cartão, modal, pedido, arte e voo antigos, inclusive após inatividade. Avisos/estreia ficam preservados. Fixture ASan/UBSan e revisão independente passaram; o novo lote foi gerado da mesma fonte e passou nas três plataformas e nos checksums. A validação física continua pendente. O HTML/capturas continuam uma proposta visual e não mudaram com esse bugfix. Detalhes e comando de regressão em VALIDACAO-LOCAL.md.
