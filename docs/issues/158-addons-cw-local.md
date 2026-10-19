# Issue #158 — agregação de legendas e retomada local na integração 1.8

Data: 03/10/2026. Worktree `codex/integration-180-glass`. Correções locais; nenhuma instalação, publicação, alteração de tag/anexos ou comentário/encerramento de issue foi feito nesta etapa.

## Defeitos reproduzidos e comportamento atual

| Caminho | Antes | Depois | Prova |
|---|---|---|---|
| Legendas de vários addons | `buscarLegendas` parava o loop externo assim que o primeiro addon preenchesse `LEG_MAX=12`; as cotas de idioma eram consumidas antes de consultar os seguintes | Consulta todos os addons ativos que declaram legendas. Guarda um lote limitado por origem e distribui os 12 lugares por rodízio de addon dentro de cada grupo de idioma | Dois providers, ambos com 56 candidatas. As duas consultas ocorrem e cada um contribui com seis linhas; principal/secundário/inglês recebem quatro linhas cada |
| Origem na folha de faixas | Toda legenda externa mostrava o selo OpenSubtitles, inclusive resultado de outro addon | `Legenda.provedor` acompanha o resultado até `rotuloLegenda`; origem ausente usa “Legenda externa” | Função de rótulo da folha real testada com OpenSubtitles, Subs.ro e Community Subtitles |
| Continuar assistindo local | Montava `CatItem` vazio a partir de progresso e dependia do enfeite Cinemeta/Trakt. IDs sem `tt` e sem poster eram compactados para fora | Copia metadados do catálogo/cache por identidade base + tipo sob `pubTrava`. O progresso atual repõe ID, episódio, percentual, instante e tempo restante. Enriquece uma cópia; descarte remoto não descarta registro local | IMDb, TMDB, Kitsu e provider próprio preservam título/poster/origem conhecidos com rede fora. Registro sem metadados continua com “Filme”/“Programa de TV”, sem imagem inventada |
| Retorno da rede | Um título de reserva persistido poderia impedir um parser que preenche só campos vazios de trazer o título real | Reserva é removida apenas na cópia enviada ao enfeite; resultado real substitui-a quando chega | Fixture offline → catálogo com reserva → reconexão traz o título real |

A distribuição mantém a ordem principal → secundário → inglês. “Todas” e ausência de preferência usam um grupo sem filtro. O limite visual continua sendo 12; não promete espaço para todos quando há mais origens com resultados que lugares disponíveis. O lote temporário está no heap e tem teto `ADD_MAX * LEG_MAX`, sem vetor grande adicional na pilha da TV.

`cat_copiar_por_id` compara a base inteira (`kitsu:41370`, `tmdb:t101`, etc.), sem cortar no primeiro `:`; filtra pelo tipo e prefere uma cópia com poster. Retorna uma cópia independente sob a trava de publicação, para que a descoberta não retenha ponteiro de um bloco liberável na virada de quadro. Se o episódio mudou, o nome do episódio antigo é limpo.

As regras de entrada no Continuar local continuam sendo duração mínima de 60 segundos, pelo menos 1% e percentual abaixo de `ajustes_cw_concluido()` (90% de fábrica). Não houve mudança no gate de gravação `video_pronto >= 120`. A remoção/ocultação de uma coleção não remove o registro de progresso; o teste local conserva os cinco registros mesmo com catálogo vazio. Ordenação e remoção explícita de CW também passaram nas regressões existentes.

## Diagnóstico por resource

A consulta de legendas usa `rede_baixar_medido_controle` e emite:

```text
[addon-recurso] addon=2 resource=subtitles tipo=movie http=200 bytes=4007 ms=7 array=1 recebidas=56 candidatas=12
```

`addon` é o ordinal na lista, `tipo` é movie/series/outro e os demais campos são medidas/contagens. `array=1 recebidas=0` distingue um array vazio de `array=0`. `candidatas` conta o lote aceito antes da divisão dos lugares finais. O resumo de conclusão não inclui mais o ID do título. URLs, IDs de conteúdo, nomes de arquivo, corpos, configuração e cabeçalhos não são emitidos nesses novos eventos; a fixture verifica ausência do host, token artificial e ID artificial no stdout.

Esse evento não identifica sozinho a causa de status HTTP0, não valida todo o JSON e não constitui prova de resposta de servidores reais.

## Validação local

Executados com sucesso:

```sh
bash tests/addons_legendas.sh
SANITIZE=1 bash tests/addons_legendas.sh
bash tests/cwlocal.sh
SANITIZE=1 bash tests/cwlocal.sh
bash tests/cwordem.sh
bash tests/cwremover.sh
```

`addons_legendas.sh` cobre o worker real e o rótulo real da folha: idioma romeno ISO, preferências ordenadas, duas origens densas, origem correta, resposta vazia, resource ausente, HTTP404, campos obrigatórios e identificação de episódio. `cwlocal.sh` usa `continuarLocal` real com enfeite artificial que compacta linhas sem arte ou simula indisponibilidade total; cobre base + tipo, metadados salvos, progresso sem cache, mudança de episódio, reconexão e limites originais. Cópia de metadados continua válida após troca do catálogo e aposentadoria de blocos.

A prova antes/depois exportou somente `HEAD:src/addons.c` e `HEAD:src/descoberta.c` para diretórios temporários, sem substituir arquivos do worktree. O worker anterior falhou na asserção de duas consultas/contribuições; a montagem local anterior falhou na asserção de cinco registros preservados. Os mesmos cenários passam no código corrigido, com ASan/UBSan.

As suites existentes de CW passam integralmente (regra, descoberta e Home). Foram acrescentados apenas doubles mínimos para símbolos novos da integração: geração de recomendações, estado da modal e pacote de selos da conta. Esses doubles não substituem a regra de CW testada. Avisos já existentes: inicializador de `VideoLegendaEstilo` sem `negrito` e variável `fim` de `colecoes.c` usada apenas por atribuição.

## Flix e condições para concluir a issue

A evidência histórica permanece em [ISSUE-158-ADDON-LOGS.md](../releases/1.7.3/ISSUE-158-ADDON-LOGS.md). Os HTTP404 do recorte antigo não distinguem resource nem demonstram a causa. Nesta integração, a nova consulta D1 falhou com Error7403/account não autorizado; não há log novo atribuível ao reporter. URLs privadas não foram executadas.

As reproduções fecham os defeitos de agregação, selo de origem e descarte local por metadados; não demonstram que Flix, Pasha, Subs.ro ou Community instalados pelo reporter respondem corretamente agora. Falta obter uma tentativa real na versão candidata, com manifesto exato/resource/status, e separar Flix-Streams Free de outra implementação com nome semelhante. Sem isso, Flix continua sem causa confirmada.

Para a 1.8: validar a folha de legendas e a retomada em TVs com versão/build verificados; observar comportamento com os addons efetivamente instalados; anexar log sanitizado de tentativa Flix antes de declarar a parte Flix corrigida. Host/fixtures, pacotes e funcionamento físico são provas distintas.
