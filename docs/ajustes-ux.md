# Ajustes: pesquisa e proposta de UX

Base da pesquisa: commit `399c1953`, então no ramo `release/1.6.6`. O nome
do ramo não identifica a versão do app: esse commit já sucede o commit de
versão 1.7.0. Revisão da proposta: **02/10/2026**. Estado: **primeira entrega nativa implementada em worktree isolado; sem validação
com pessoas/controle físico**. A pesquisa histórica abaixo permanece identificada
pela base original; o estado atual está no bloco a seguir.

Os rótulos e wireframes abaixo são provisórios; os textos finais de tela e
suas traduções vêm depois. O inventário descreve a base indicada, não uma
medição de produção atual. A referência local `release/1.7.1` foi consultada
em `be9e413e`; a implementação depois incorporou `eba014d9` da 1.7.1. A
base integrada é o merge `2ab3ccc8`. Não usar o checkout principal divergente como se fosse
a continuação desta base.

O pedido do dono: "repensar em como as pessoas usam e mexem nas configs, para
entender tudo o que cada coisa faz, para deixar do jeito da pessoa". A troca de
visual da 1.6.x foi rejeitada ("o problema tá igual, só tá com outro design").
Este documento tenta explicar por que o problema continua igual e o que mudaria
de fato.

## Implementação nativa em 02/10/2026

O catálogo agora contém **183 opções em 11 categorias**, com **61 avançadas**.
`AJ_SAIDA_PLAYER` entrou em Reprodução como básico depois do inventário de 182.
A opção GPU continua condicionada a TPK/Android; no Mac/LG a lista compilada
não a inclui. IDs, chaves e índices dos valores persistidos foram preservados.

- Navegação em blocos fixos; avançados lembrados por categoria durante a sessão.
- OK abre seletor; setas percorrem o candidato sem gravar; OK aplica uma vez;
  Voltar descarta. O valor salvo tem marca própria, independente do foco.
- Busca dedicada local, com acentos/sinônimos, caminho e opção indisponível;
  abre por ID, revela avançados e Voltar recupera consulta e resultado.
- Diferenças comparadas com o mesmo inicializador imutável de fábrica, sem
  atribuir autoria à pessoa. A lista mantém posição após restaurar uma linha.
- Restauração individual com destino/alcance e Cancelar selecionado; exclui
  ações, credenciais, leituras, perfil pesquisável e chaves em arquivo próprio
  como limite de fileiras. A ordem de fileiras permanece no fluxo existente.
- Dependências explicadas; quando há um requisito editável, OK leva até ele e
  Voltar recupera a opção. Nenhum requisito é ativado silenciosamente.
- Falhas detectadas no arquivo principal mantêm o valor anterior e o editor;
  mudança concorrente invalida o candidato. O feedback confirma aplicação
  local; não alega confirmação do servidor ou descarga concluída de IDBFS.

### Acabamento alinhado ao mockup

A primeira captura nativa ficou visualmente mais pesada que o protótipo; após
esse retorno do dono, a composição foi revista usando o mockup como referência.
Título menor, coluna central mais larga, linhas de 80 px, valores em texto,
foco com contorno preciso e sem halo, superfícies azuladas e controle compacto
de avançados recuperam a hierarquia da proposta. Alcance e padrão ficam em
linhas próprias no painel explicativo. As dicas do controle ficam no rodapé.

Amostras de arte usam a mesma imagem local do protótipo, marcada como
“Exemplo ilustrativo”; não reproduzem trailer, não chamam a rede e não simulam
uma prévia reversível sobre a Home real. Se a arte não estiver no pacote, o
painel retorna à ilustração esquemática existente. As superfícies de Ajustes
são opacas para leitura, inclusive com Vidro ativado para a Home; a cor de
realce escolhida continua respeitada. Nenhuma mudança no shader global.

[Capturas nativas e verificação](reviews/ajustes-ux/README.md).

Fontes: `src/ajustes_ux_tela.inc`, `ajustes_ux_dados.inc`,
`ajustes_ux_interacao.inc`, `ajustes_ux_desenho.inc`, `ajustes.c`,
`spotlight.c/.h` e despacho de `app.c`.

Validação de host: catálogo, busca/defaults/privacidade, cenários D-pad e
persistência, padrões, perfis, modo seguro, idiomas e Spotlight. Capturas
nativas usam fixtures sem conta. Isso não é prova de controle físico,
acessibilidade semântica, desempenho ou persistência em TV.

As etapas 2A/2B (prévia na tela real, prazo e presets de desempenho) e 3
continuam propostas. O protótipo abaixo demonstra algumas delas, mas elas
não foram transferidas ao renderer nativo nesta entrega. Textos novos têm
EN/ES e fallback para os outros idiomas; revisão linguística completa está
pendente. [Direção e pesquisas por plataforma](ux-direcao.md).

## Protótipo para experimentar

A proposta tem um [protótipo local navegável](prototypes/ajustes-ux/README.md)
com Trailers, Idiomas e Desempenho: 18 opções, busca, diferenças do padrão,
folhas de escolha e experimento reversível. Valores e sincronização são
simulados; a demonstração não altera o app ou a conta. Os testes de navegador
registrados no README não substituem validação com pessoas ou em TV física.

## Resultado da revisão

A direção continua válida: organizar por intenção, explicar consequências
e permitir experimentar com segurança. A proposta precisava fechar o
comportamento, reduzir promessas sem evidência e dividir melhor a entrega.

| Prioridade | Lacuna encontrada | Decisão desta proposta |
|---|---|---|
| Alta | Setas mudam valores e também precisam navegar entre colunas | Navegar não altera valores; OK abre edição; painel de ajuda não recebe foco |
| Alta | Diferença do padrão é atribuída à pessoa sem histórico | Usar “Diferente do padrão”; origem/data só quando registradas |
| Alta | Prévia usa caminhos que já gravam e sincronizam | Separar rascunho, aplicação e confirmação; cancelar recompõe o estado anterior |
| Alta | Perfil “só desta TV” inclui preferências sincronizáveis | Restringir perfis a uma lista comprovadamente local; não mudar gosto automaticamente |
| Alta | A Fase 1 promete alterar só tabelas, mas inclui busca e estado novo | Separar navegação, busca e comparação/restauração em entregas próprias |
| Alta | Logs de alteração são tratados como prova de descoberta e causalidade | Manter limites da amostra e validar tarefas com pessoas |
| Média | Onze seções e três colunas são tratadas como solução comprovada | Prototipar e comparar custo em teclas, legibilidade e entendimento |

Contextos de uso a validar: quem quer apenas assistir e corrigir um incômodo;
quem personaliza a aparência; quem tenta resolver lentidão ou falha. São
hipóteses de uso, não personas obtidas em entrevistas. A avaliação é
qualitativa: não há base para atribuir uma nota numérica de usabilidade ou
prometer redução percentual de tempo.

---

## 1. Inventário (medido no código)

Fonte: `src/ajustes.c` (enum `OpcaoId`, `OPCOES[]`, `CHAVE[]`, `valor[]`,
`TELA[]`, `ajudaOpcao`, `efeitoOpcao`, `desenhaPrevia`, `somenteDesteAparelho`).
Contagem feita por script sobre o fonte, não à mão.

| | |
|---|---|
| Opções no enum `AJ_*` | **176** |
| Por tipo | 127 escolhas (97 binárias, das quais 90 usam Ligado/Desligado), 10 números, 33 ações, 6 só leitura |
| Ajustes com valor (escolha + número) | 137: 76 podem ir para a conta, 61 ficam só nesta TV |
| Chaves com "-" (não gravam) | 41 (ações, leituras, credenciais em arquivo próprio) |
| Estrutura da tela | 9 categorias › 13 grupos recolhíveis + 9 rótulos fixos › 176 linhas |
| Profundidade | Ajustes › categoria › grupo › linha › (folha/teclado/sub-tela). Ex.: "Trailer do cartaz em foco" = Ajustes › Layout › Foco no pôster › 3ª linha |
| Linhas por categoria | Layout **76**, Integrações 41, Conteúdo 14, Avançado 13, Reprodução 10, Aparência 9, Conta 6, Sobre 4, Trakt e Simkl 3 |
| Maior grupo | "Conteúdo da Home": 20 linhas misturando fileiras, barra lateral, destaque, rótulos, notas e o fundo da escolha de perfil |
| Têm frase de ajuda própria | 175 de 176 (falta só `AJ_SALVOS_DEST`, que cai no texto genérico) |
| Têm a linha "o que muda na prática" (`efeitoOpcao`) | 14 |
| Têm prévia desenhada (`desenhaPrevia`) | 8 opções, 5 desenhos distintos — todos **esquemáticos** (retângulos cinza), não a tela de verdade |

Enum, tipos, categorias, elegibilidade conta/local e cobertura do mapa foram
recontados nesta revisão. As contagens de qualidade da ajuda e de prévias
continuam sendo avaliações da pesquisa original. As 176 linhas incluem
`AJ_GPU_EFEITOS`, visível apenas em builds TPK/Android; nos demais são 175
linhas e 12 em Avançado. Elegibilidade à conta não comprova sincronização.

**Qualidade da ajuda.** A quantidade de texto não é o problema; o tipo de
texto é. Lendo as 175 frases, uma a uma:

- ~25 repetem o rótulo ou ficam no jargão sem dizer o que a pessoa vê:
  "Controla o arredondamento dos cantos dos pôsteres", "Quanto a borda do
  cartaz em foco acende", "Use Reduzidas para movimentos mais discretos",
  "Escolhe o próximo episódio a partir do mais avançado marcado como
  assistido", e os 8+11 toggles de nota com a mesma frase.
- Várias não dizem o que o **Desligado** faz. Ex.: Dolby Vision ("Preferência
  para fontes compatíveis") — desligar evita fonte DV? deixa tocar sem DV? A
  pessoa com TV sem DV precisa exatamente dessa resposta.
- Pelo menos uma é enganosa por omissão: "Largura do item" — o próprio
  `ajustes.h` avisa que no layout Moderna essa preferência **não muda o
  tamanho** do cartaz; a ajuda não diz.
- As longas (Layout da home: 377 caracteres; Limite de fileiras: 315)
  explicam bem, mas viram parágrafo no painel e ninguém lê do sofá.

**Mesmo assunto espalhado em vários lugares** (a causa mais forte de "não
achei"):

| Assunto | Onde está hoje |
|---|---|
| Idioma | Aparência › Idioma (interface) · Reprodução › Idiomas (áudio, legenda) · Integrações › TMDB › Idioma dos metadados · estilo da legenda só dentro do player |
| Trailer | Layout da Home (destaque, som, espera) · Foco no pôster (cartaz em foco) · Página de detalhes (automático, som, botão, qualidade, proporção, fonte) — 10 linhas em 3 grupos |
| Notas | Conteúdo da Home › Avaliações gerais · Integrações › MDBList (8 fontes) · Integrações › Notas no título (11 fontes) |
| Arte | Layout da Home (Background do hero, Destaque com outra arte) · Pôsteres personalizados (7) · Arte do addon (4) · Integrações › TMDB › Arte localizada · fanart.tv |
| Ficha do título | Página de detalhes (Priorizar metadados externos **e** Usar sempre o Cinemeta, que se sobrepõem) · TMDB (13 toggles e 1 idioma) |

**Por que a troca de visual não resolveu.** A árvore atual é a do app web
(`SECTION_META` de `settingsScreen.js`), que organiza por **onde o ajuste mora
no código** (Layout, Integrações, Avançado), não pelo que a pessoa quer fazer.
"Layout" sozinho tem 43% de todas as linhas. Redesenhar a mesma árvore mantém
os mesmos caminhos de 4 níveis, os mesmos assuntos espalhados e as mesmas
prévias que não mostram a tela da pessoa.

O que já existe e serve de base (não reinventar):

- `TELA[]` é **separada do enum** desde a 1.4.4: reorganizar a tela não toca
  `valor[]`, `CHAVE[]` nem o arquivo.
- `inativa()` já conhece as dependências entre opções (e diz o porquê).
- `seguro.c` já faz "experimentar e voltar sozinho se cair" para 10 ajustes
  arriscados.
- `ajustes_abrir_na_cor/fonte/layout/vidro` já são atalhos que pousam o foco
  numa linha (usados pelos cartões de novidades).
- O menu de segurar OK no cartaz (`ctxmenu.c`) já tem "Estilo da fileira": um
  ajuste contextual que funciona.
- `perfiltv.c` (`PtvPerfil`) dimensiona recursos como texturas, rede e largura
  de arte; não implementa os novos conjuntos visuais da §4.6. `gpunivel.c`
  já oferece medição de GPU como entrada para diagnóstico.
- O Spotlight (`spotlight.c`) busca títulos, pessoas, canais, catálogos e
  addons — **não busca ajustes**.

### 1.1 Diferenças verificadas depois da base

Comparação local em 02/10/2026, ancorada em commits, sem executar o app:

| Medida | `399c1953` (pesquisa) | `be9e413e` (release/1.7.1 consultada) |
|---|---:|---:|
| Opções no enum | 176 | 182 |
| Escolhas / números / ações / leituras | 127 / 10 / 33 / 6 | 130 / 11 / 35 / 6 |
| Com valor: elegíveis à conta / locais | 76 / 61 | 76 / 65 |
| Grupos recolhíveis | 13 | 14 |

Acrescentar ao mapa, preservando enum e persistência existentes:

| ID novo | Destino proposto | Tratamento |
|---|---|---|
| `AJ_SELO_VISTO` | Tela inicial › Fileiras | Básico; escolha local |
| `AJ_SEEKR_LIGADO`, `AJ_SEEKR_FITA` | Reprodução › Miniaturas na barra de tempo | Básicos; escolhas locais |
| `AJ_SEEKR_CHAVE`, `AJ_SEEKR_TESTAR` | Mesmo bloco de miniaturas | Básicos; configurar/testar serviço; chave protegida, fora da comparação de valores |
| `AJ_SEEKR_AJUSTE` | Mesmo bloco de miniaturas | Avançado; correção local de tempo |

O mapa-base continua com 116 básicas e 60 avançadas; incluir esse delta leva
a **121 básicas e 61 avançadas** antes de agrupamentos e filtros de plataforma.
Isso é cobertura de opções, não quantidade final de linhas visíveis.

A normalização regional de idioma TMDB (`pt-br` → `pt`) já está no código da
tag `v1.7.0` (`b07bb926`). A suspeita histórica da §2.1 não deve virar trabalho
duplicado; vínculo com #209 e resultado em TV continuam não comprovados aqui.
O Spotlight também mudou: integrar no código da versão-alvo, não copiar o
arquivo da pesquisa. `seguro.c` e `perfiltv.c` não mudaram entre a base e
`be9e413e`, portanto os limites técnicos descritos nas §§4.3/4.6 permanecem.

---

## 2. Uso real

### 2.1 Registros das TVs (D1, só agregados)

Recorte da pesquisa original: ~9,4 dias de registros, **265 aparelhos
reportados** (webOS 162, Tizen .wgt 73, .tpk 49, VIDAA 5, Android 3, Mac 1).
Inclui as TVs de teste do dono e só quem tem envio de registro ligado.

**Limite da evidência:** os subtotais por plataforma somam **293**, não 265.
Pode haver sobreposição, mas ela não está demonstrada. Faltam neste documento
as datas exatas da janela, a consulta e a regra de deduplicação; os números
abaixo foram preservados como relato histórico e **não foram reconsultados
nesta revisão**. Não somar plataformas nem generalizar percentuais para toda
a base até reconciliar essa diferença. Anexar apenas consulta e agregados,
sem identificadores de aparelhos.

**Não existe hoje uma linha de log por ajuste mudado.** O que dá para medir:

| Sinal | Resultado |
|---|---|
| `[seguro] em prova` (os 10 ajustes arriscados) | **105 aparelhos (40%)** mudaram ao menos um. Vidro **93** · tema imersivo 35 · limite de fileiras 31 · interface 4K 26 · qualidade da imagem 24 · trailer no destaque 22 · itens por fileira 16 · P2P 13 · memória de imagens 11 · trailer do cartaz 4 |
| `[seguro] revertido` (o app caiu e desfez) | 11 aparelhos: vidro 5, imersivo 2, 4K 2, qualidade 1, P2P 1 |
| `[fileiras] escolha local: limite N` (último por aparelho, 201) | 130 no padrão 7 · 24 em 16 (o padrão antigo, suspeito que não foi escolha) · **47 (23%) escolheram outro**, de 3 a 40 (6 no máximo, 40) |
| `[tex] teto de N MB pedido em Ajustes` | 35 aparelhos fixaram a memória de imagens (14 no máximo, 512 MB) |
| `[gpu-modos] lento` (90 aparelhos, só sessões lentas) | vidro ligado em 54 (60%), tema dinâmico em 37 (41%), layout Dinâmica 17, Padrão 3 |
| `[ajustes] blob da conta` (último arranque, 200) | em 120 a conta mudou algum ajuste da TV; 69 aparelhos sobem ajustes da TV para a conta |
| `nao reconhecido; mantido` | `tmdb_language` com valor que a TV não conhece em 16+ aparelhos (`pt-br` 7, `ro` 4, `el`, `es-419`, `ru`, `ar`, `uk`); `selected_theme=CUSTOM/DARK` 3 |

Leitura:
- Entre os **10 ajustes monitorados pelo modo seguro**, vidro tem mais
  aparelhos com mudanças e reversões registradas. Isso não prova que seja o
  mais alterado entre todos os ajustes, nem que tenha causado as quedas.
  Vidro ligado em 60% dos aparelhos com sessões lentas é associação, sem
  grupo de comparação. Recomendação por TV e experimentação são hipóteses
  justificadas para testar, não demanda ou efeito causal já demonstrados.
- Quase um quarto mudou o número de fileiras — ajuste de "como fica a
  minha home", hoje no 1º grupo de Layout, ok.
- Em 120/200 aparelhos do recorte, o último arranque observado aplicou
  mudanças da conta. Isso pode ser sincronização esperada; o log não prova
  conflito ou frustração. Mostrar origem e alcance ajuda a explicar o estado.
- Achado original fora do escopo, **suspeito, não provado**: `tmdb_language` "pt-br"
  da conta não casa com a lista `W_TMDB_LING` (que tem "pt"), então a TV
  ignora o idioma de metadados escolhido no web. Pode estar por trás do #209
  (títulos em inglês com português escolhido). O fallback regional já foi
  tratado no código posterior (§1.1); falta comprovar a relação com o relato
  e o comportamento no aparelho, não reimplementar a normalização.

### 2.2 Issues do GitHub (204 lidas pelo título, ~50 abertas e lidas inteiras)

Síntese herdada da pesquisa original; as issues não foram relidas nesta
revisão. Os números identificam relatos, não confirmam que cada falha ainda
exista na versão atual. Antes de usar uma issue como critério de correção,
registrar seu link, versão afetada, estado atual e reprodução.

| Padrão | Issues |
|---|---|
| **Pediu algo que já existia / não achou** | #124 (trailer no destaque já existia), #127 ("Separar futuros" existia), #57 (o dono pergunta se a pessoa conhece "Escolher a fonte ao reproduzir"), #183 (o dono indica Ajustes › Layout › Fonte do trailer), #201 ("Is there a setting I'm missing?" — o estilo por fileira está no segurar OK), #71 (linha de leitura confundida com ajuste) |
| **Ajuste que não fazia o que dizia / efeito invisível** | #160 ("Catálogos do destaque" nunca fez nada), #162 ("Local do Descobrir" sem efeito), #133 e #177 (desfocar não aplicava), #199 (mexeu em "não exibidos" e na fonte do CW, nada mudou), #205 (mudar a fonte do CW "conserta" a fileira), #202 (texto branco no branco na própria tela de Ajustes) |
| **Não confia que salvou** | #85, #129, #149 (voltavam ao padrão ao reabrir) |
| **Idioma em vários lugares** | #3, #8, #150, #209 — a pessoa escolhe português "no app e no TMDB" e não entende por que a sinopse vem em inglês |
| **A tela em si** | #11 ("cluttered"), #75 (pediu cabeçalhos selecionáveis para pular a lista), #144 ("mainly the settings are really bad") |
| **Pedidos de personalização** | #95, #162, #163, #90, #142, #104, #47, #91, #175 |
| **Trailer (o tema mais recorrente)** | 13 issues: #15, #60, #82, #86, #123, #124, #136, #140, #168, #178, #183, #195, #204 |

Nas respostas, o dono precisa escrever caminhos de 4 níveis
("Settings › Layout › Poster focus › Trailer on the focused poster"): sinal
direto de que achar é o problema, não entender a frase.

---

## 3. Referências verificadas e decisões para o Nuvio

Fontes primárias consultadas em 02/10/2026. Elas sustentam princípios; não
demonstram que todas as TVs usam a mesma árvore ou o mesmo gesto de edição.

| Fonte | Princípio documentado | Aplicação proposta |
|---|---|---|
| [Android — Design for TV](https://developer.android.com/design/ui/tv/guides/foundations/design-for-tv) | Leitura à distância, pouca densidade e resposta clara ao D-pad | Testar a ficha no sofá; não encolher texto para caber em três colunas |
| [Android — TV navigation](https://developer.android.com/training/tv/get-started/navigation) | Navegação previsível, foco visível e acesso aos controles pelo direcional | Definir transições, retorno e restauração de foco antes do desenho final |
| [Apple — Focus and selection](https://developer.apple.com/design/human-interface-guidelines/focus-and-selection/) | Foco e ativação têm papéis distintos; evitar mudanças inesperadas de foco | Percorrer um ajuste não deve ativá-lo nem salvar uma escolha |
| [Kodi — Settings](https://kodi.wiki/view/Settings) | Níveis Básico, Padrão, Avançado e Especialista controlam opções visíveis | Testar divulgação progressiva sem tornar busca ou recuperação dependentes do nível |

Painel sobre a tela real, busca por sinônimos e restauração contextual são
decisões de projeto a validar no Nuvio. A regra de duas camadas descreve a
navegação **seção → opção**; seletores, conexões de serviços e confirmações
podem acrescentar passos. Medir o caminho completo, não só profundidade.

---

## 4. Proposta

### 4.1 Arquitetura por intenção

Onze seções são a **hipótese inicial**, não uma quantidade validada. O mapa
da base cobre 176 opções, sem repetição nem ausência, com **116 básicas e 60
avançadas**, recontadas nesta revisão. O total de linhas depende de
agrupamentos e da plataforma. A previsão anterior de ~85 não era uma contagem
reproduzível e deixa de ser meta; medir se as pessoas concluem suas tarefas.
O delta posterior à base deve entrar no mapa antes da implementação (§1.1).

Agrupar notas ou itens da barra pode facilitar escolhas relacionadas, mas
acrescenta uma folha e teclas. Credenciais e provedores são fluxos de conexão,
não múltipla escolha; não agrupá-los apenas para baixar a contagem. Cada
opção terá um destino canônico e referências cruzadas quando houver duas
expectativas razoáveis, como trailer do cartaz em “Cartazes” e “Trailers”.

```
 Buscar nos ajustes            (abre o Spotlight já filtrado em Ajustes)
 Diferente do padrão (12)      (permanece acessível quando vazio)
 ─────────────────────────────
 1  Tela inicial               estilo · fileiras · Continuar assistindo · barra lateral
 2  Cartazes e arte            cartaz em foco · forma · de onde vem a arte
 3  Trailers                   onde toca · som · fonte
 4  Página do título           o que aparece · notas · dados do título
 5  Reprodução                 escolha da fonte · imagem e som · durante e depois
 6  Idiomas e legendas         interface · áudio · legenda · metadados
 7  Aparência                  cor · vidro · fonte · animações · relógio
 8  TV ao vivo                 provedor · se o canal não abre
 9  Contas e serviços          conta e perfis · Trakt e Simkl · chaves · P2P
10  Desempenho desta TV        perfil pronto · resolução · imagens · diagnóstico
11  Sobre e ajuda              versão · atualizar · registros
```

Mapa (prefixo `AJ_` omitido; *itálico* = Avançado):

| Seção › bloco | Opções |
|---|---|
| 1 › Estilo | HOME_LAYOUT, LANDSCAPE, HERO, HERO_CHEIO, HERO_CATALOGOS, *HERO_TRANSICAO* |
| 1 › Fileiras | FIL_ORDEM, FIL_LIMITE, ITENS_FILEIRA, OCULTAR_NLANC, NOTAS_HOME, *ROTULOS, NOME_ADDON, SUFIXO_TIPO* |
| 1 › Continuar assistindo | CW_LIGADO, CW_OK, CW_ESTILO, CW_THUMB, CW_BLUR_PROX, CW_ORDEM, CW_NAO_EXIBIDOS, *CW_FURTHEST* |
| 1 › Barra lateral | RAIL_MODERNA, RAIL, MENU_EXPLORAR/GUIA/AGENDA/PERFIL (uma linha), *RAIL_BLUR, DESCOBRIR* |
| 1 › Escolha de perfil | PS_FUNDO |
| 2 › Cartaz em foco | EXPANDIR, EXPANDIR_ATRASO, BORDA_FOCO, *GRAD_CLASSICO* |
| 2 › Forma | LARGURA_DP, RAIO_DP, PROF, *PROF_BORDA, PROF_BRILHO, PROF_COBERTURA, PROF_POSTERS/CW/EPS/ELENCO/TRAILERS* |
| 2 › De onde vem a arte | HERO_FUNDO, HERO_ARTE_DIF, POSTER_PROV, *ADDON_POSTER/FUNDO/LOGO, COL_ARTE_CONTA, POSTER_INST/TOKEN/EXTRA/CHAVE/MODELO/TESTAR* |
| 3 › Onde toca | HERO_TRAILER, FOCO_TRAILER, DET_TRAILER_AUTO, DET_TRAILER |
| 3 › Som e imagem | HERO_TRAILER_SOM, DET_TRAILER_SOM, TRAILER_FONTE, *HERO_TRAILER_ESPERA, TRAILER_QUAL, TRAILER_ASPECTO* |
| 4 › O que aparece | DET_VEU, DET_DATA_CHEIA, DET_BLUR_NAO_VISTOS |
| 4 › Notas | NT_* (11, uma linha "Quais notas"), MDB_LIGADO, *MDB_TRAKT…MDB_MAL* |
| 4 › Dados do título | TMDB_LIGADO, *TMDB_ARTE…TMDB_CW (12), DET_META_EXT, DET_SO_CINEMETA* |
| 5 › Escolha da fonte | FONTE_MANUAL, FONTE_AUTO, FONTE_REPOR, *SELOS_CORES* |
| 5 › Imagem e som | QUALIDADE, DV, ATMOS |
| 5 › Durante e depois | PAUSA_OVERLAY, CW_CONCLUIDO |
| 6 | IDIOMA, AUD_LINGUA, LEG_LINGUA, TMDB_IDIOMA (+ atalho para o estilo da legenda do player) |
| 7 | TEMA, COR_LOGO, VIDRO, FONTE_UI, ANIM, RELOGIO, RELOGIO_POS, *VIDRO_CONTORNO* |
| 8 › Provedor | XTREAM_SERVIDOR/USUARIO/SENHA/CONTA/LIMPAR, STALKER_PORTAL/MAC/LIMPAR, EPG_PAIS |
| 8 › Se o canal não abre | LIVETV_DIAG, LIVETV_RES, LIVETV_ESPERA, *LIVETV_FORMATO, LIVETV_MODO, LIVETV_PROXY* |
| 9 › Conta e perfis | PERFIL_ATIVO, SYNC, ADDONS, ADDONS_PRINCIPAL, PERFIL_PESQ, PERFIL_EDITAR, SAIR |
| 9 › Trakt e Simkl | TRAKT, SIMKL, SALVOS_DEST, CW_FONTE |
| 9 › Chaves | DEBRID_RD/TB/PM/AD/AD_TESTAR, MDB_CHAVE, FANART_CHAVE (uma linha que abre a lista) |
| 9 › P2P | *P2P_LIGADO, P2P_URL, P2P_TESTAR* |
| 10 | (perfil pronto, novo), RESOLUCAO, QUALIDADE_IMG, GPU_EFEITOS (.tpk/Android), NAV_RAPIDA, DIAGNOSTICO, VELOCIDADE, ESPACO, *TEX_MB* |
| 11 | VERSAO_I, ATUALIZAR, ENVIAR_LOG, ENVIO_AUTO |

Outras regras da arquitetura:

- **Linhas de leitura saem da lista** (Versão, Perfil, Sincronização, Conta
  Xtream, Memória usada): viram o cabeçalho da seção. Resolve #71.
- **Avançada:** recolhida por preferência, mas encontrada na busca e em
  “Diferente do padrão”. “Mostrar avançados” fica no topo da lista, alcançável
  sem percorrer todas as opções. Busca pode revelar uma linha temporariamente.
- **Dependente:** continua visível, com motivo e ação para abrir o requisito,
  mesmo em outra seção. Não sumir com “Som do trailer” só porque o trailer
  foi desligado; isso impediria entender a relação entre os dois.
- **Incompatível com a plataforma:** pode sair da lista padrão, mas a busca
  explica a indisponibilidade. Não oferecer botão sem efeito. TV ao vivo sem
  provedor mostra uma entrada para conectar um provedor e explica o requisito.
- Manter a ordem estável; se um filtro remover a linha em foco, ir para a
  linha válida mais próxima ou o cabeçalho. Foco e seleção não dependem só
  de cor. Um valor avançado já alterado nunca desaparece da recuperação.
- **Duas linhas que se sobrepõem viram uma** com valores claros: "Priorizar
  metadados externos" + "Usar sempre o Cinemeta" → uma escolha de 3 valores
  (o mapeamento para as duas chaves continua por baixo, nada muda no arquivo).

### 4.2 A tela e o contrato do controle remoto

O esboço de três colunas é uma alternativa de protótipo. Só as duas primeiras
participam da navegação; a ficha à direita é informativa. Os controles de
alterar, restaurar e abrir requisito ficam na folha acessível por OK.

```text
┌─────────────────────┬────────────────────────────────┬──────────────────────────┐
│ Buscar              │ Trailers                       │ No cartaz em foco        │
│ Diferente do padrão │ Mostrar avançados: Não         │                          │
│                     │                                │ Ao parar num cartaz,     │
│ Tela inicial        │ ONDE TOCA                      │ mostra o trailer quando  │
│ Cartazes e arte     │ No destaque do topo  Ligado    │ os requisitos permitem.  │
│ Trailers            │ > No cartaz em foco  Ligado  › │                          │
│ Página do título    │ Na página do título  Ligado    │ Desligado: mantém a foto │
│ Reprodução          │ Botão de trailer     Ligado    │                          │
│ Idiomas e legendas  │                                │ Vale: neste perfil*     │
│ …                   │ SOM E IMAGEM                   │ Padrão: Desligado        │
│                     │ Som no destaque      Desligado │                          │
│                     │ Fonte do trailer     Automático│ OK: opções e detalhes   │
└─────────────────────┴────────────────────────────────┴──────────────────────────┘
```

`*` O escopo e o estado de sincronização são calculados para a chave e o
valor efetivos, não escritos de forma fixa no desenho. A folha detalha os
requisitos exatos; para trailer no cartaz, a base aceita expansão **ou**
formato paisagem e o modo seguro pode suspender o efeito.

| Contexto | Direcionais | OK | Voltar |
|---|---|---|---|
| Coluna de seções | ↑/↓ percorrem; → entra na lista | Entra na lista, preservando última linha válida | Retorna à tela de origem |
| Lista de opções | ↑/↓ percorrem; ← volta às seções; → não altera valor | Abre folha de edição ou o fluxo identificado na linha | Volta às seções; em entrada por busca/atalho, retorna à origem |
| Folha de escolha | ↑/↓ percorrem valores e ações; ✓ marca o valor salvo | Confirma o candidato ou executa a ação explicitamente escolhida | Descarta candidato e devolve foco à linha |
| Folha numérica | ←/→ ajustam o candidato; ↑/↓ alcançam Aplicar/Cancelar | Aplica apenas quando Aplicar está em foco | Descarta candidato |
| Folha de seleção múltipla | ↑/↓ percorrem itens e Aplicar/Cancelar | Marca candidato; só Aplicar confirma o conjunto | Descarta todas as mudanças da folha |
| Teclado/diálogo subordinado | Conforme o componente | Confirma apenas esse componente | Fecha uma camada, sem saltar até a home |

Toda linha editável, inclusive liga/desliga, usa OK para entrar na edição.
Isso custa uma ativação extra; comparar com a tela atual nas tarefas da §7.
Não mudar gestos entre linhas sem indicação visível. Ações como sair da conta
ou conectar serviço não simulam seletores de valor. Não depender de voz,
tecla colorida ou segurar OK para acessar uma função essencial.

Ao voltar, preservar consulta, rolagem, seção e identificador da linha, não
apenas seu índice na lista filtrada. Se a origem deixou de existir, pousar
no ancestral válido mais próximo. Atualizações assíncronas não roubam foco.

**Ficha curta:** efeito do valor atual, consequência da alternativa,
requisito quando necessário, alcance e padrão. Texto completo e ações ficam
na folha. Não colocar controles desenhados no painel sem caminho pelo D-pad.
Não usar só cor para distinguir foco, seleção, erro e indisponibilidade.

Comparar três colunas com duas colunas e descrição ampliada na folha. Em
720p ou com tradução longa, priorizar leitura e reduzir conteúdo simultâneo;
não reduzir a fonte para encaixar tudo. Validar foco visível, margens,
contraste em todos os temas e uso com animações reduzidas. Uma imagem de
prévia só entra se explicar o efeito; texto continua suficiente sem ela.

### 4.3 Prévia e experimento: aplicar sem perder a escolha anterior

Há três estados distintos: **candidato**, **em teste** e **confirmado**.
Percorrer valores não grava arquivo, não sobe para a conta e não conta como
alteração confirmada. Voltar, trocar de perfil ou fechar o fluxo descarta o
candidato. Se não houver prévia segura, mostrar uma explicação ou miniatura;
não simular cancelamento enquanto a mudança já foi sincronizada.

1. **A própria tela, em ajuste rápido.** Painel lateral sobre a superfície
   onde o efeito é percebido, com largura e densidade a testar. A cena de
   fundo pode ler valores temporários de opções elegíveis. Antes de habilitar
   um ajuste, verificar efeitos colaterais: alguns acessos remontam fileiras,
   carregam arte ou alteram estado fora de `valor[]`. “Aplicar” confirma;
   “Cancelar” recompõe valor e efeitos. Reusar texturas com vida útil garantida;
   não manter outra cópia da tela nem depender de uma imagem congelada para
   afirmar que é prévia ao vivo.
2. **Miniatura com arte real.** Reusar recursos disponíveis do cache, sem
   exigir downloads; se faltarem, usar uma amostra identificada como exemplo.
   Mostrar antes/depois para opções em que isso ajuda. A miniatura não prova
   fluidez nem compatibilidade na TV.
3. **Experimento que pode ser revertido.** Para alterações que exigem remontar
   ou podem deixar a tela inutilizável, mostrar previamente o conjunto afetado
   e as ações Manter/Voltar. O prazo inicial de 15 s é hipótese de teste e só
   começa após a tela utilizável aparecer; considerar tempo maior para leitura.
   Há também um prazo de aplicação, iniciado ao aplicar: se a tela não ficar
   pronta dentro desse limite, reverter sem esperar a confirmação humana.
   Definir esse limite com medições por plataforma; em travamento do processo,
   a recuperação persistida deve atuar no próximo arranque.
   Voltar, expiração ou falha recompõem o conjunto inteiro. Só a confirmação
   autoriza publicar a preferência; nenhuma mudança parcial deve escapar.

**Dependência de implementação:** `mudarValorDireto()` já aplica efeitos,
grava e sinaliza sincronização. Não pode ser chamado a cada foco como se fosse
uma API de prévia. É necessária uma camada temporária e um caminho único de
confirmação, com captura dos valores anteriores e tratamento de falhas.

`seguro.c` protege contra queda: confirma após 180 s ou saída limpa e tenta
reverter no arranque seguinte após falha. Isso **não equivale** à confirmação
humana com prazo de 15 s; `AJ_HOME_LAYOUT` sequer integra a lista de riscos
nessa base. Coordenar os dois mecanismos: registrar transação, perfil e lote;
impedir que estabilidade ou saída limpa confirmem um experimento não aceito;
preservar recuperação após reinício e não expulsar mudança pendente do diário.
O modo seguro continua com prioridade sobre o experimento e deve explicar
quando suspender um efeito.

### 4.4 Busca nos ajustes

- O Spotlight ganha o grupo Ajustes e um modo exclusivo aberto por “Buscar”
  na tela de configurações. Nesse modo, consultar somente índice local; não
  disparar busca de títulos, pessoas ou addons nem misturar seus recentes.
- Indexar rótulo, valores não sensíveis, ajuda e sinônimos revisados por idioma
  (legenda/subtitle/CC, dublado, trailer, lento/travando). Não indexar senha,
  token, URL de provedor, nome de perfil ou conteúdo digitado em credenciais.
- Priorizar correspondência exata de rótulo, depois sinônimo e ajuda; manter
  ordenação estável. Mostrar caminho, valor atual e estado de disponibilidade.
- Incluir avançadas. Ao abrir uma, revelar temporariamente a linha e informar
  o motivo; isso não liga todos os avançados. Resultado dependente permite
  chegar ao requisito; resultado incompatível explica por que não funciona
  neste aparelho. Nunca conduzir a uma lista vazia ou à linha errada.
- Generalizar os atalhos em `ajustes_abrir_em(op)`, com destino canônico e
  contexto de retorno. Voltar recupera resultados e consulta anteriores.
- Consulta vazia oferece temas locais; sem resultados, explicar como tentar
  outro termo e manter acesso às seções. Busca funciona sem rede e sem voz.
  Integrar a entrada de texto já existente na versão-alvo, sem reinstalar a
  implementação antiga do Spotlight.

### 4.5 “Diferente do padrão”, origem e restauração

A primeira entrega compara preferências configuradas/salvas com **padrões da
versão atual, aplicáveis à plataforma**. Apresentar separadamente o efeito
vigente e suspensões pelo modo seguro, dependências ou capacidade da TV; não
confundir idioma “Automático” com o idioma resolvido nem ocultar um valor
salvo porque seu efeito está temporariamente suspenso. Diferença não prova
autoria, data ou problema. O nome “O que você mudou” fica descartado enquanto
esses fatos não existirem.

- Criar uma referência imutável de padrões; `valor[]` é estado mutável. Se
  houver padrão recomendado para o aparelho, nomeá-lo separadamente. Atualizar
  o app pode mudar a comparação sem que ninguém tenha alterado a preferência.
- Mostrar opções avançadas alteradas e estado vazio explicativo. Excluir ações,
  leituras e credenciais da comparação/restauração automática. Opções compostas
  mostram as duas chaves e preservam combinações legadas sem conversão silenciosa.
- Informar duas coisas diferentes: **alcance** (aparelho ou preferência do
  perfil) e **estado da gravação/sync**. “Salvo nesta TV”, “Aguardando
  sincronização” e “Sincronizado” exigem evidência do caminho real. Falha de
  gravação precisa aparecer; não prometer salvamento só porque o valor mudou.
- Não deduzir exportação somente de `somenteDesteAparelho()` ou chave `"-"`:
  há exceções por valor, chave ausente no blob e conta/rede sem suporte. Uma
  chave compartilhável não está necessariamente publicada.
- “Restaurar padrão” mostra o valor de destino e o alcance antes da confirmação.
  Em preferência sincronizável, explicar que a mudança pode chegar às outras
  TVs e ao app web do mesmo perfil. Aplicar pelo fluxo normal de efeitos e
  sincronização; offline, indicar pendência. Reabrir confirma persistência local.
- “Usar valor da conta” é outra operação, só disponível com valor remoto
  conhecido. Não inventar essa ação sem manter a referência remota. Credenciais,
  logout e reset completo continuam em fluxos próprios.
- Restauração por seção fica para depois da restauração por linha; deve listar
  o lote, omitir ações/segredos, tratar falha parcial e permitir desfazer de
  forma coerente com o escopo compartilhado.

**Histórico é uma entrega posterior.** Para valores preexistentes, origem e
data são desconhecidas; não deduzi-las pela diferença do padrão nem pelo
horário de carregamento. Novos registros distinguem confirmação local, conta,
migração, recomendação e recuperação. Preferências de perfil e do aparelho
têm escopos diferentes: respeitar `dePerfil()`, não gravar todo histórico em
um arquivo por perfil. Definir limpeza no logout e preservar as preferências
locais que hoje sobrevivem à saída.

### 4.6 Desempenho desta TV: recomendações e conjuntos de mudanças

Substituir a tabela universal Leve/Equilibrado/Máximo por uma proposta
revisável para **este aparelho**, com o que muda, o que é preservado e por quê.
“Máximo” não equivale a qualidade garantida; aceitar sinal 4K não comprova
que a interface terá bom desempenho em 4K.

A primeira versão considera apenas uma lista comprovadamente local:
resolução da interface, qualidade/memória de imagens, limite de fileiras,
itens por fileira, efeitos da GPU quando disponíveis e vidro. Cada campo
precisa de limite por plataforma e do protocolo de experimento da §4.3.
Sem evidência para recomendar, manter Automático e oferecer diagnóstico.

**Excluir da aplicação automática profundidade, trailer no cartaz e tema.**
`AJ_PROF` e `AJ_FOCO_TRAILER` são sincronizáveis na base; trocar um tema dinâmico
por fixo também pode torná-lo exportável. Não tratar preferências de gosto
como ajuste local de desempenho. Um futuro override local exigiria projeto
explícito de precedência, persistência e indicação do valor efetivo.

`PtvPerfil` já dimensiona recursos como texturas, rede e largura da arte;
não representa esses novos conjuntos de preferências. Usar seu diagnóstico
como entrada, não como prova de que o recurso proposto já existe. RAM, GPU,
quedas e FPS podem orientar uma recomendação depois de validar a relação com
o custo de cada opção por plataforma.

Aplicação manual lista valores anteriores e propostos. Preservar escolhas
existentes no arranque; não reaplicar um conjunto em toda abertura. Após
ajuste manual, indicar Personalizado. Se os dados da TV justificarem uma
mudança, oferecer uma recomendação explicada. “Voltar ao que eu tinha” usa
uma captura do lote anterior e respeita perfil, concorrência e modo seguro.

### 4.7 Assistente opcional de primeira vez

Entrega posterior, condicionada às tarefas que continuarem difíceis. Só
oferecer em instalação nova sem preferências recuperadas; não interceptar
quem quer assistir. Pular mantém padrões explícitos e o assistente pode ser
aberto depois. Conta existente recebe resumo de importação apenas quando
número e origem forem conhecidos.

Perguntas candidatas: idioma da interface; idioma preferido de áudio e de
legenda; seleção automática ou manual da fonte; estilo da home; autoplay de
trailers. Cada resposta mostra as opções afetadas e seu alcance. “Nunca/só
quando necessário/sempre” para legendas exige política além de `LEG_LINGUA`;
não prometer esse comportamento sem especificar suporte no player.

Escolhas ficam em rascunho até um resumo final; Voltar preserva o rascunho e
Sair/Pular não aplica respostas pela metade. Não selecionar silenciosamente
um perfil visual com base em RAM. Manter o dimensionamento automático já
existente e apresentar recomendações adicionais de forma explícita.

### 4.8 Atalhos contextuais

O mesmo ajuste pode ser alcançado de onde seu efeito é percebido, preservando
um destino canônico e o contrato de edição:

- Menu de segurar OK no cartaz: manter “Estilo da fileira” e acrescentar
  entrada de ajustes da superfície atual. Toda opção também existe em Ajustes.
- Player: preferências de reprodução, áudio/legenda e escolha da fonte, sem
  interromper a sessão por navegar. Distinguir faixa desta reprodução de
  preferência para próximas reproduções.
- Página do título: trailers, notas e escurecimento, com retorno ao mesmo
  título e posição. Mensagens de diagnóstico podem oferecer atalho específico.
- Cartões de novidades: usam a mesma abertura por identificador. Se opção
  ou plataforma não estiver disponível, explicar e oferecer destino válido.

Atalhos não contam como segunda definição da opção em `TELA[]`. Busca,
atalhos e tela completa compartilham efeitos, validação, confirmação e retorno.

### 4.9 Medição mínima, sem registrar conteúdo sensível

Instrumentar **mudanças confirmadas**, com uma lista positiva de opções
seguras. Exemplo de formato: `[ajustes] confirmou id=<id> antes=<enum>
depois=<enum> via=<tela|rapido|busca|assistente|recomendacao|conta>`.
Números só entram com domínio/limites conhecidos. Não registrar texto livre,
consultas, nomes de perfil, URLs, servidor, MAC, senha, chave ou token; para
uma conexão, no máximo tipo de ação e resultado sem credenciais.

Separar abertura de ajuste, confirmação, cancelamento e falha. Prévia,
carregamento e tentativa sem mudança não contam como confirmação. `via=busca`
só significa uma alteração confirmada após esse caminho; não mede todas as
buscas, descobertas ou compreensão. Propagar a origem real, inclusive nos
setters fora da tela, sem anunciar cobertura completa antes de instrumentá-los.

Respeitar a preferência existente de envio de registros; não criar upload ou
identificador adicional. Histórico local para a pessoa e telemetria agregada
são recursos distintos. Definir retenção, limite e limpeza dos novos dados.

Uma janela inicial de duas semanas pode revelar padrões de uso, mas baixa
frequência não justifica remover um ajuste: pode haver padrão satisfatório,
falta de descoberta ou uso raro essencial. Combinar agregados com as tarefas
da §7; não atribuir lentidão, queda ou sucesso à mudança sem evidência.

---

## 5. Entregas, dependências e prioridade

Recomendação: validar o caminho completo de uma tarefa antes de transformar
as 183 opções de uma vez. O recorte de protótipo deve incluir Trailers,
Idiomas e uma opção de desempenho; cobre achar, entender, editar e desfazer.
A entrega do produto mantém acesso a todas as opções da versão-alvo.

| Etapa | Entrega | Dependências reais | Condição para avançar |
|---|---|---|---|
| Preparação | Mapa atualizado, textos de exemplo e protótipo das tarefas | Inventário por commit; contrato de foco e estados | Revisão visual e comparação das tarefas da §7 |
| 1A — Navegação e explicação | Árvore por intenção, avançados, ficha curta e folhas de escolha sem gravação por foco | `ajustes.c/.h`, tradução, atalhos existentes e testes de navegação; confirmar onde guardar preferência de avançados | Cobertura de todos os IDs; zero mudança acidental e foco recuperável |
| 1B — Encontrar | Busca local de ajustes, sinônimos, abertura por ID e retorno | `spotlight.c/.h`, despacho em `app.c`, entrada de texto da versão-alvo e índice traduzido | Busca offline; revela avançadas; não dispara consultas de entretenimento |
| 1C — Entender estado e recuperar | Diferente do padrão e restauração por linha, com alcance e status reais | Referência imutável de padrões, aplicação de efeitos, gravação e sync | Reabrir conserva valor; falha/pendência visíveis; nenhuma credencial no reset |
| 2A — Experimentar | Prévia sobre a tela real e atalhos contextuais | Camada temporária, retorno, vida útil de texturas e reversão de efeitos | Cancelar recompõe estado sem gravação/sync; desempenho medido |
| 2B — Recomendar | Experimento com prazo e lotes locais de desempenho | Coordenação com `seguro.c`, limites por plataforma e captura do lote | Reversão integral por timeout/falha/reinício; nunca publicar rascunho |
| 3 — Só com necessidade demonstrada | Histórico de origem/data, restauração por seção, miniaturas e assistente | Persistência nova versionada, isolamento de perfil e evidência das tarefas | Resolve dificuldade observada; não substitui correção das etapas anteriores |

Não chamar a Fase 1 de mudança “só em `TELA[]`”: busca envolve o roteamento do
app; comparação exige padrões; restauração usa efeitos e sync; histórico
exige dados novos. Manter os IDs, chaves e índices existentes é uma restrição
de compatibilidade, não a promessa de que apenas tabelas de tela serão editadas.

Instrumentação segura da §4.9 acompanha cada caminho entregue, com cobertura
explícita. Não bloquear melhorias básicas por uma coleta completa de produção,
nem remover opções por frequência baixa. Nova preferência de avançados e
qualquer estado persistente precisam definir escopo e migrar sem perder os
valores existentes, inclusive no Tizen .wgt.

---

## 6. Riscos (o que não pode quebrar)

- **Vetores paralelos posicionais.** `OPCOES[]`, `CHAVE[]` e `valor[]` são
  indexados pelo enum; opção nova só no fim. Reordenar a apresentação em
  `TELA[]` não exige reordenar enum. Preservar também os índices de `V_*`
  gravados em disco; não inserir valores no meio.
- **Opções compostas.** A junção metadados externos/Cinemeta é apresentação;
  continua gravando as duas chaves. Não normalizar silenciosamente combinações
  legadas ao abrir a tela. Avisar sobre o alcance distinto de cada chave.
- **Sync com conta e app web.** `ajustes_mesclar_blob()` só reescreve chave
  existente e exportável. Uma restauração compartilhável pode ser propagada
  após confirmação do servidor. Informar alcance antes da ação e status
  depois; não substituir padrão por valor da conta silenciosamente. Preservar
  exceções por valor, como tema dinâmico. Lotes de desempenho só usam chaves
  comprovadamente locais.
- **Perfis de pessoa.** `ajustes-p<N>.txt` guarda só o subconjunto de
  `dePerfil()`, não todos os ajustes. Separar origem/histórico do aparelho e
  do perfil; cancelar edição pendente antes de trocar perfil. O logout limpa
  dados vinculados à conta conforme contrato existente, preservando preferências
  do aparelho. `ajustes_perfil_esquecer()` não equivale a reset geral.
- **Cobertura de tela.** Manter uma definição por ID conforme `conferirTela()`
  e `tests/ajustes_secoes.sh`. Atalhos não contam como segunda definição;
  filtros de plataforma e avançados não podem quebrar os destinos da busca.
- **Compatibilidade de navegação.** Atualizar no mesmo conjunto os testes
  `ajustes_secoes`, `ajustes_shot`, `notas_ajustes_shot`, os atalhos
  `ajustes_abrir_no_*` e os cartões de novidades. Preservar o destino esperado.
- **Memória nas TVs com poucos recursos.** Não criar outra cópia da tela para
  a prévia; uma textura RGBA de 1080p ocupa cerca de 8 MB antes de custos
  adicionais. Medir o uso real por plataforma, o tempo de quadro e a vida útil
  das texturas, sem converter estimativa em orçamento de memória comprovado.
- **Persistência no Tizen .wgt.** Estado novo usa a camada `dados_gravar`,
  com formato, escopo e recuperação de falha definidos. Testar fechamento e
  reabertura; escrita em memória não demonstra gravação durável.
- **Textos e acessibilidade.** Traduzir para os idiomas suportados pela
  versão-alvo; verificar rótulos longos, pluralização, fallback de glifos e
  direcionalidade. Manter contraste, foco além de cor e animação reduzida.
- **Modo seguro.** Experimentos e recomendações não podem disputar o diário
  de `seguro.c` (teto de 12 mudanças). Coordenar confirmação humana,
  estabilidade e recuperação; não descartar pendência para caber outro lote.
- **Concorrência.** Se conta, diagnóstico ou modo seguro mudar uma chave
  durante a edição, invalidar o candidato afetado e mostrar o novo estado.
  Cancelar não pode sobrescrever uma atualização legítima posterior. Definir
  essa política antes de oferecer prévia de chaves compartilhadas.

---

## 7. Como validar a proposta

Comparar a tela atual da versão-alvo com um protótipo navegável, mantendo o
mesmo aparelho, perfil de teste e configuração inicial. Alternar a ordem
entre participantes para reduzir aprendizado. Começar com 5–8 pessoas com
experiências distintas; é estudo formativo, não amostra para inferir
percentuais de toda a base. Não foi realizado nesta revisão.

| Tarefa, sem indicar caminho | O que deve ser observado |
|---|---|
| “Não quero trailer quando paro num cartaz; o do topo pode continuar.” | Encontra a opção específica e não desliga as demais |
| “Quero preferir áudio em português, mantendo a interface em inglês.” | Distingue idioma da interface, áudio, legenda e metadados |
| “Quero corrigir uma opção avançada que não aparece na lista.” | Encontra pela busca, entende o estado e retorna à consulta |
| “Quero ligar este recurso, mas ele não está disponível.” | Explica se falta requisito ou suporte e encontra o próximo passo |
| “Quero ver outra aparência e depois ficar como estava.” | Experimenta e cancela sem perder a configuração anterior |
| “Quero restaurar uma opção que também uso na outra TV.” | Identifica valor de destino, alcance por perfil e pendência de sync |
| “Quero deixar esta TV mais leve sem mudar a outra.” | Entende o lote local, confirma conscientemente e consegue desfazer |

Registrar sucesso sem ajuda, tempo, teclas até conclusão, caminhos errados,
mudanças acidentais e explicação do efeito/alcance pela pessoa. Perguntar
“o que você espera que aconteça?” antes de confirmar. Repetir casos sem voz,
sem rede e com avançados ocultos. Uma captura estática não mede essas tarefas.

**Critérios obrigatórios dos cenários testados:**

- Zero mudança causada apenas por navegação; cancelar não grava/sincroniza.
- Zero perda de foco ou controle inacessível; Voltar recupera a origem prevista.
- Confirmação persiste após fechar/reabrir; falha de gravação é visível.
- Estado pendente/offline não é apresentado como sincronizado; outro perfil
  não recebe preferências locais de forma indevida.
- Experimento reverte o lote por cancelamento, prazo, falha e reinício;
  concorrência segue a política da §6 sem sobrescrever estado posterior.
- Busca abre o ID certo, inclusive avançado, indisponível e após tradução.
- Leitura no sofá em 720p/1080p, contraste nos temas, texto longo e animações
  reduzidas; nenhum texto essencial cortado para acomodar as colunas.

Definir metas de tempo e teclas depois de medir a linha de base. A nova árvore
deve reduzir dificuldade de encontrar e explicar efeitos nas tarefas
principais; se só reduzir linhas, a hipótese não passou. Não sacrificar
compreensão/cancelamento para obter menos teclas.

Verificação de implementação futura: testes focados de mapa/IDs, foco e
retorno, persistência por perfil, sync e falhas; depois inspeção visual e
controle físico nas plataformas suportadas. Registrar separadamente revisão
do desenho, build/pacote/instalação e evidência em TV. Para custo de memória e
experimentos, incluir C9, Tizen com recursos limitados e Android; validar .wgt
e .tpk nos respectivos caminhos, sem tratar um como prova do outro.

## 8. Rastreabilidade da revisão

Leitura técnica na base `399c1953` (mesmos arquivos no worktree `9ce4f55e`):

| Evidência | Local para revisão |
|---|---|
| Escopo do aparelho e persistência por perfil | `src/ajustes.c`: `somenteDesteAparelho()`, `dePerfil()`, `ajustes_perfil_guardar()` |
| Exportação condicionada ao blob e ao valor | `src/ajustes.c`: `ajustes_mesclar_blob()`; `src/sync.c`: `empurrarAjustes()` |
| Edição aplica, grava e sinaliza sync | `src/ajustes.c`: `mudarValorDireto()` |
| Lista de ajustes arriscados | `src/ajustes.c`: `RISCOS[]` |
| Confirmação por estabilidade e recuperação após queda | `src/seguro.h`, `src/seguro.c`: `seguro_iniciar()`, `seguro_mudou()` |
| Dimensionamento de recursos por TV | `src/perfiltv.h`: `PtvPerfil`; `src/perfiltv.c` |
| Busca e despacho atuais | `src/spotlight.c/.h`; `src/app.c` |
| Credenciais e restrição de logs | `src/ajustes.c`: armazenamento de provedores de pôsteres |

Comando de leitura para repetir o inventário bruto nas duas revisões. Conta
IDs e macros do fonte; não simula filtros de plataforma, agrupamentos da
proposta nem comportamento em execução:

```bash
python3 - <<'PYCODE'
import collections, re, subprocess
for ref in ('399c1953', 'be9e413e'):
    src = subprocess.check_output(
        ['git', 'show', ref + ':src/ajustes.c'], text=True)
    src = re.sub(r'//[^\n]*|/\*.*?\*/', '', src, flags=re.S)
    enum = src.split('typedef enum {', 1)[1].split('} OpcaoId;', 1)[0]
    ids = re.findall(r'\bAJ_[A-Z0-9_]+\b', enum)
    ids.remove('AJ_N')
    opts = src.split('static const Opcao OPCOES[AJ_N] = {', 1)[1].split('\n};', 1)[0]
    kinds = collections.Counter(re.findall(r'\b(ESC|NUM|LER|ACAO)\(', opts))
    print(ref, len(ids), dict(kinds))
PYCODE
```

A cobertura do mapa-base foi conferida expandindo barras pelo prefixo,
intervalos pela ordem do enum e `NT_*` por prefixo: 176 IDs únicos, 0 ausentes
e 0 duplicados, 116 básicos/60 avançados. O mapa executável da implementação
deve tornar essa verificação automática para os 182 IDs e todas as plataformas.
Logs D1, issues e teste com usuários permanecem evidências a atualizar, não
resultados produzidos por esta revisão.
