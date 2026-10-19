# Issue158 — Flix-Streams e add-ons de legenda

03/10/2026. Revisão somente leitura dos500 registros privados21252–21751 já exportados do D1. Sem chamadas a URLs privadas de add-on, sem autenticação, comentário público ou alteração runtime. Identidade do reporter não foi estabelecida pelo dump, que não possui coluna de pessoa; combinação de add-ons é indício, não identificação.

## Evidência útil, com duplicatas removidas

| Registro/linha | Evidência sanitizada | O que permite concluir |
|---|---|---|
|21749:135–137|Flix-Streams Free, manifesto `org.flixstreams.free`, catalogo=1/stream=1/legenda=0|Manifesto foi lido e resource stream reconhecido |
|21749:169–172|Subs.ro Subtitles, `org.stremio.subsro`, catalogo=0/stream=0/legenda=1|Resource de legenda foi reconhecido; zero catálogos é esperado para esse tipo |
|21749:191 e224|HTTP404 no host associado a Flix-Streams Free|Há duas respostas404 reais no caminho usado pelo app, mas rota/configuração redigidas impedem diagnosticar endpoint |
|21738:905–907|Flix-Streams, `org.flickystream.addon`, catalogo=1/stream=1/legenda=0/meta=1|É outra implementação/manifesto; não pode ser tratado como o mesmo Flix-Streams Free |
|21738:541|Busca de legendas terminou com12 resultados agregados|Não é prova de resposta Subs.ro nem Community; evento não informa add-on responsável |

Os23 uploads com Subs.ro têm corpo idêntico byte a byte (checado por hash local). Representante21749: webOS, uploader1.7.2, corpo56316 caracteres. Portanto não são23 sessões independentes nem23 ocorrências novas. Esse corpo NÃO possui evento `[legendas]`; não comprova consulta HTTP ou parsing de resultado Subs.ro. Presença do manifesto não comprova uso posterior.

PashaMetadata/PashaStreams e StremioCommunitySubtitles não aparecem por esses nomes no recorte. Ausência de nome não prova que o usuário não instalou: lista limitada, logs redigidos/truncados e manifesto pode usar outro nome. Nenhum registro foi atribuído ao reporter somente porque contém Flix+Subs.ro.

## Por que o404 ainda não fecha a causa

`rede.c` passa URLs por `rede_url_publica` ao registrar status. O path/configuração deixa de ser visível, então o log não distingue `/catalog`, `/stream`, `/meta` ou `/subtitles` naquele host. Contexto é atividade de catálogo/descoberta, não confirmação de playback. O404 pode ser rota incorreta, catálogo declarado e indisponível, ID incompatível ou servidor; não dá para escolher entre eles com essa evidência.

Comprimento29/57 dos textos de base redigida no log NÃO é comprimento da URL configurada original e não prova truncamento. `addons.c` guarda base600 e usa `url[900]` na busca de legendas, mas a configuração privada integral não está no dump para medir limite ou perda de segmento/query. Não afirmar correção de truncamento para este caso sem fixture que reproduza.

## Pontos concretos do cliente e próximo teste

1. `addons.c:701–780`, `buscarLegendas`: consulta add-ons ativos com flaglegenda, sequencialmente, prazo25s por request. Falha ou falta do array `subtitles` não gera diagnóstico por add-on; apenas resumo agregado ao final. Acrescentar evento sanitizado com identificador público do manifesto, resource, tipo do item, HTTP, bytes, JSON válido/array existente e resultados aceitos/filtrados. Nunca logar ID do filme, configuração ou URL/token.
2. O mesmo loop forma `/subtitles/{tipo}/{id}.json` sem aplicar declaração types/idPrefixes específica do resource de legenda. Isso é suspeito para providers com IDs próprios, mas NÃO foi comprovado como causa Pasha/Subs.ro/Community neste dump. Fixtures devem incluir manifesto com resourceobjeto/type/prefix, série com season/episode e resposta vazia versus inválida, além de idioma principal/secundário e campos obrigatórios.
3. A normalização de base em `addons.c:200` remove query e sufixo manifest conforme regra anterior. Avaliar em fixture query funcional versus query de versão e base longa; não alterar política global só porHTTP404. Mapear manifesto de cada serviço para endpoints esperados usando fonte pública ou configuração explicitamente fornecida para teste, sem executar os privados desta revisão.
4. Distinguir Flix-Streams Free (`org.flixstreams.free`) do Flix-Streams (`org.flickystream.addon`) nas fixtures e relatório. O nome de exibição não é chave suficiente para deduplicar provider ou aplicar correção específica.

Conclusão: reconhecimento do resource Subs.ro está demonstrado; falha de consulta/parser de legendas não. Há404 associado a FlixFree com endpoint indeterminado. A investigação precisa de diagnóstico por resource e fixture reproduzível antes de declarar correção da issue158.


## Correções reproduzidas após a publicação da 1.7.3

Na branch `codex/fix-addons-158`, commits `a8f8d031` e `354215a3`:

- Subs.ro mapeia `ro` para `ron` em sua implementação pública ([addon.js](https://github.com/allecsc/stremio-subs-ro/blob/master/addon.js)). O cliente só reconhecia `ro`/`rum`; preferência por romeno rejeitava `ron`. A família de idioma, o nome na tela e a seleção automática agora aceitam os três códigos. Cinco verificações falhavam antes e passaram com a correção.
- Preferência local “Todas” e ausência de preferência sem idioma secundário criavam um grupo exclusivo de inglês na busca dos add-ons. Agora usam um grupo sem filtro; idiomas escolhidos continuam na ordem principal, secundário, inglês, com o mesmo teto compartilhado de resultados. O teste do worker real retornava só uma legenda inglesa e passou a retornar os quatro idiomas da fixture.
- `tests/addons_legendas.sh`, regressão `addonslista.sh`, testes de línguas e execução com ASan/UBSan passaram. Os testes usam respostas artificiais locais; não acessam a configuração do reporter nem comprovam resposta de seu servidor.

Estas correções estão no código local e **não fazem parte dos pacotes publicados da 1.7.3**. Não houve alteração dos anexos/tag, instalação ou nova publicação. Elas não demonstram que todos os add-ons do relato foram corrigidos nem estabelecem a causa do HTTP404 no Flix-Streams.

O protocolo Stremio permite extras `filename`, `videoHash` e `videoSize` ([SDK](https://github.com/Stremio/stremio-addon-sdk/blob/master/docs/api/requests/defineSubtitlesHandler.md)). Sua ausência não foi aceita como explicação geral: Community admite rota sem extras e Subs.ro admite filename opcional. Preservar configuração de usuário e comparar resposta real antes de alterar essa política.


## Sonda pública sem configuração privada

Em 03/10/2026, o manifesto público sem configuração [free.flixnest.app/manifest.json](https://free.flixnest.app/manifest.json) respondeu HTTP200 e confirmou `org.flixstreams.free`. Resources: stream/catalog/meta; types: movie/series/tv; idPrefixes: tt/tmdb/essential/dlstreams. Os dois catálogos declarados são tv/essential-live-events e tv/dlstreams-live. O manifesto público principal [flixnest.app/flix-streams/manifest.json](https://flixnest.app/flix-streams/manifest.json) respondeu403 nesta sonda. Não foram consultadas URLs configuradas, catálogos, fontes nem arquivos de mídia. Essas sondas não identificam o add-on `org.flickystream.addon` nem estabelecem a causa das respostas404 históricas. Para concluir o caso do Flix é necessário log novo da tentativa com diagnóstico de resource/HTTP ou o manifesto público exato do add-on instalado.
