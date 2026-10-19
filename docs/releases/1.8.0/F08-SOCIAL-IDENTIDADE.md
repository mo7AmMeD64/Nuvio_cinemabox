# F08 — Social com identidade unificada (contrato)

Estado em 04/10/2026: implementado no repositório (branch `f08-social-identidade`), **sem deploy**. Migração `servidor/recomendacoes/migracao-008-identidade.sql`, rotas em `servidor/recomendacoes/src/identidade.js`, cliente em `src/recomenda.c`, `src/socialvis.c`, `src/amigoperfil.c` e `src/salvospainel.c`. Contratos anteriores continuam valendo: [social-contrato](../../social-contrato.md), [social-fontes](../../social-fontes.md). Decisões do dono (02/10): cada perfil da casa é uma pessoa; juntar Trakt/Simkl/Letterboxd; privacidade em níveis.

## 1. Pessoa canônica

- A **pessoa canônica** é uma linha de `pessoa`. Com conta Nuvio, ela é o **perfil**: `nuvio:<sub>` no principal e `nuvio:<sub>:<índice>` nos demais (cabeçalho `X-Nuvio-Perfil`, já em produção desde a migração 006). Nenhum id novo foi criado: tudo o que já existe continua com o mesmo dono.
- Quem só tem Trakt continua sendo `trakt:<slug>` até ligar um perfil Nuvio. O Social não depende de Trakt nem de Simkl: com só a conta Nuvio, amigos (código, sugestão, pedido), atividade (`/v1/feed`), recomendações e respostas já funcionam com o id do perfil.
- Trakt, Simkl e Letterboxd são **identidades ligadas** à pessoa, nunca pessoas próprias depois do vínculo.

## 2. Identidades ligadas e verificação

Tabela `identidade (provedor, sujeito, pessoa, metodo, verificado, visivel, nome, criado, verificado_em)`.

| Provedor | Sujeito | Como é provado | Efeito |
|---|---|---|---|
| `trakt` | slug | Token OAuth conferido agora em `api.trakt.tv/users/settings` (mesma verificação de `quemE`) | Verificado; resolve a autenticação e funde a pessoa `trakt:<slug>` |
| `simkl` | id numérico da conta | Token conferido em `api.simkl.com/users/settings` com `SIMKL_CLIENT_ID`. **Não medido contra a API real**; sem o segredo a rota responde 501 | Verificado; não funde nada (Simkl nunca foi pessoa no servidor) |
| `letterboxd` | usuário (minúsculo) | Nenhum: a API é fechada. Fica `metodo='declarado'`, `verificado=0` | Só aparece para a própria pessoa; nunca funde, resolve ou deduplica |

Regras: vínculo só com **as duas provas no mesmo pedido** — o token que autentica o pedido e o token da outra conta no corpo, ambos conferidos na hora no emissor e **nunca guardados**. Nada é fundido por nome, e‑mail ou slug digitado. Uma identidade verificada pertence a uma pessoa só (índice único parcial); uma pessoa tem no máximo uma conta por provedor. Perfil não principal: o índice é conferido no Supabase com o mesmo token (`rpc/sync_pull_profiles`, que já resolve conta compartilhada); se a consulta falhar, o vínculo falha fechado (503).

### Rotas

- `POST /v1/eu` ganha `recursos: ["identidade1"]`, `autenticado: "trakt"|"nuvio"` e `identidades: [...]`. É a detecção de recurso do cliente.
- `GET /v1/identidades` → `{pessoa, recursos, identidades}`.
- `POST /v1/identidades/vincular`
  - Pedido autenticado pelo Trakt: `{"provedor":"nuvio","token":"<supabase>","perfil":<índice, opcional>}` → canônica = perfil Nuvio; `trakt:<slug>` é fundido nele.
  - Pedido autenticado pela conta Nuvio: `{"provedor":"trakt","token":"<trakt>"}`, `{"provedor":"simkl","token":"..."}` ou `{"provedor":"letterboxd","usuario":"..."}`.
  - Respostas: `200 {ok, pessoa, ja, fundiu}`; `409` identidade de outro perfil ou segunda conta do mesmo serviço; `401` token inválido; `403` perfil não pertence à conta; `501` Simkl sem segredo; `503` não deu para conferir o perfil.
- `POST /v1/identidades/desvincular {"provedor"}` — apaga o vínculo. **Não separa o passado**: o que foi fundido continua na canônica; o próximo pedido daquele Trakt vira uma pessoa nova e vazia.
- `POST /v1/identidades/visivel {"provedor","visivel":0|1}` — só para verificadas.

### Resolução

Todo pedido autenticado pelo Trakt consulta `identidade(trakt, slug, verificado=1)`; se houver, é atendido **como a pessoa canônica** (nome/foto do Trakt não sobrescrevem os do perfil). Ids antigos guardados por TVs (`para` de uma rec, `id` de remover/bloquear, `?id=` do perfil do amigo) são traduzidos pela tabela `fusao (de → para)` antes de qualquer rota.

## 3. Deduplicação

- **Amigos**: depois da fusão, contatos do `trakt:<slug>` são reescritos para a canônica (`INSERT OR IGNORE` nos dois sentidos), então a lista do servidor já sai sem duplicata. `GET /v1/contatos` ganha `ids: ["trakt:<slug>", ...]` por contato — só verificadas com `visivel=1`, só para contatos diretos. O cliente usa esses ids para trocar o `trakt:<slug>` dos itens do Trakt pelo id do contato (`recomenda_feed_unido`), e o rosto/linha fica um só.
- **Atividade**: no cliente, `rec_eventos_unir` já junta mesma pessoa + título + ação (+ episódio, + valor da reação) com |Δt| ≤ 1 h; com o id trocado, o fato visto pelo Trakt e pelo nosso servidor vira uma linha (vence o do nosso servidor). No servidor, `/v1/feed` agora colapsa o mesmo fato da mesma pessoa em 1 h (duas TVs da pessoa, uma antiga pelo Trakt e outra pela conta Nuvio). Episódios diferentes e reações diferentes nunca se fundem.
- **Canais**: Simkl e Letterboxd não têm feed de amigos (ver social-fontes); não há canal a deduplicar ainda. A estrutura (`ids` e `RecEvento.fonte`) já aceita quando houver adaptador.

## 4. Privacidade

| Estado | Onde | Significado |
|---|---|---|
| `alcance` −1/0/1/2 | `pessoa` (006) | Inalterado: quem vê a atividade automática |
| `descobrivel` | `pessoa` (002) | Inalterado: aparecer em sugestões |
| `identidade.visivel` 0/1 | 008 | 0 = só eu (o servidor usa para fundir/resolver); 1 = amigos diretos recebem o id em `/v1/contatos`. Padrão 1 para verificadas, 0 para declaradas. Hoje só pela API (sem tela) |
| Comparação | `/v1/amigo` | Só entre dois que compartilham; senão o cliente mostra "Privado" ou "Ative sua atividade para comparar" |

Conflitos na fusão, todos no sentido de **não abrir o que estava fechado**: quem nunca respondeu (−1) adota a resposta do outro lado; se os dois responderam, fica o `alcance` menor; `descobrivel` fica o menor (o aparelho reconcilia a escolha local no `/v1/eu` seguinte); bloqueio vence contato; o handle público da canônica vence; o código de pareamento do Trakt deixa de existir.

## 5. Comparações com cobertura

`GET /v1/amigo` → `gosto` passa a trazer `filmes {total, iguais}`, `series {total, iguais}`, `comum {filmes, series}` (títulos concluídos pelos dois em 90 dias), `cobertura {eu, ele}` e `generos: null`.

- **Match** = mesma reação (gostei / mais ou menos / não gostei) entre os títulos que os dois reagiram nos últimos 90 dias. Só aparece com ≥ 5 pares (`SV_CMP_MIN`); abaixo disso, "Poucos dados · n de 5". O tamanho da amostra vai junto ("75% · 9 de 12 iguais").
- **Filmes / Séries**: o mesmo, separado pela mídia.
- **Vistos pelos dois**: contagem real, sem mínimo.
- **Gêneros**: nenhuma fonte tem gênero nos eventos → sempre "Sem dados de gênero".
- Estados explícitos no cliente: Carregando, Desconhecido (servidor antigo ou sem perfil do servidor, ex.: amigo só do Trakt), Privado, Ative sua atividade, Poucos dados. Nenhum vira zero.

## 6. Migração dos dados atuais

- A migração 008 é **só aditiva e idempotente** (2 tabelas, 3 índices). Ninguém muda de dono ao aplicá‑la; worker antigo continua funcionando com ela.
- A fusão acontece pessoa a pessoa, quando ela pede o vínculo. Um batch (transação D1) move contatos, pedidos, bloqueios, recs, eventos, "agora", agregados (somando segundos do mês), atividade antiga, registros e identidades; apaga a pessoa antiga e grava `fusao`.
- Teste: `servidor/recomendacoes/teste-identidade.mjs` (worker inteiro, SQLite real, emissores falsos).

## 7. Compatibilidade

- **Cliente antigo + servidor novo**: continua preferindo o Trakt; depois do vínculo, seus pedidos caem na canônica sem ele saber. Campos novos (`recursos`, `ids`, detalhes de `gosto`) são ignorados. O teste de shell `"gosto":{"total":2,...` foi ajustado para o prefixo (objeto ganhou campos).
- **Cliente novo + servidor antigo**: sem `identidade1` no `/v1/eu`, a linha "Trakt neste perfil" não existe, nenhum `ids` é esperado, o merge de feed segue igual a antes, e a comparação mostra "Desconhecido" por mídia (o Match aparece se o servidor velho mandar o total).
- **Unir é explícito**: o cliente nunca vincula sozinho. Na aba Amigos, com Trakt e conta Nuvio no aparelho e o servidor novo, aparece "Trakt neste perfil · OK para unir…". Separar pede dois OKs e avisa que o que já foi unido fica.

## 8. Deploy (pendente, pedir ao dono)

Ordem obrigatória:

1. `npx wrangler@4 d1 execute nuvio-recomendacoes --remote --config servidor/recomendacoes/wrangler.toml --file servidor/recomendacoes/migracao-008-identidade.sql`
2. Só então `npx wrangler@4 deploy --config servidor/recomendacoes/wrangler.toml` **a partir de uma árvore que contenha F08 e as rotas de 006/007** (deploy do master antigo apaga `/v1/feed`, `/v1/amigo`, `/v1/rec/resposta`). A 007 precisa já estar aplicada (o pedido diz que está).
3. Opcional: `wrangler secret put SIMKL_CLIENT_ID` para o vínculo Simkl (sem ele: 501, e o cliente não oferece Simkl).

## 9. O que falta

- Tela para Simkl/Letterboxd e para `visivel` (API pronta, cliente não oferece).
- Pessoa canônica por **dono** de conta compartilhada (`get_sync_owner`): hoje o id usa o `sub` do token; dois membros de uma conta compartilhada no mesmo perfil seguem pessoas diferentes. Mudar isso exige outra fusão (mesmo mecanismo de `fusao`).
- Nada foi provado em TV nem contra Trakt/Supabase/Simkl reais: só SQLite local, `wrangler dev --local` e capturas GL no Mac.
