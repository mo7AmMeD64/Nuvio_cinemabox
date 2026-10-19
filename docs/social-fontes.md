# Social: de onde vem cada dado de amigo

Pesquisa de 02/10/2026 para o redesenho do Social (decisao 3: "ver todos os
dados e ver o que precisa"). **Medido** = testado com curl ou lido no nosso
codigo; **doc** = so a documentacao publica diz; nada aqui foi provado em TV.

## O que cada fonte expoe sobre AMIGOS

| Dado do amigo | Trakt | Simkl | Letterboxd | Nuvio (Supabase) | Nosso servidor |
|---|---|---|---|---|---|
| Lista de amigos/seguidos | sim: `users/me/following`, `/friends`, `/followers` (medido: `corpoSlugsTrakt` em recomenda.c, `socialPorSeguidos` em trakt.c) | **nao** — nao ha endpoint de amigos/seguidos na API (doc: llms.txt da api.simkl.org, 127 paginas, nenhuma de friends/following) | nao no RSS; so na API fechada | **nao** — 13 tabelas, nenhuma social (medido 15/09, ver index.js) | sim: `contato` (codigo, Trakt, sugestao, pedido) |
| Historico / "assistiu" | sim: `users/:id/history`, feed `users/me/friends/activities` (medido: o feed agregado da **401** na LG, comentario de `trakt_social`; o fallback por seguido funciona) | parcial: `GET /users/recently-watched-background/{user_id}` = **1** titulo (doc), sem lista | sim: RSS `letterboxd.com/<user>/rss/` traz as ultimas **50** entradas do diario (medido 02/10: 50 itens) | nao | sim (`evento`, 90 dias) |
| "Assistindo agora" | sim: `users/:id/watching` (medido no codigo, 8 seguidos por ciclo) | so do proprio usuario (scrobble); nada de terceiro (doc) | nao | nao | sim (`agora`, expira 15 min) |
| Nota / like | sim: `users/:id/ratings` (doc); o feed traz acao `rating` (medido no parse) | nao para terceiros | sim no RSS: `memberRating` (0,5-5), `memberLike` Yes/No, `rewatch`, `watchedDate` (medido) | nao | sim: `reacao` 1/0/-1 |
| Watchlist | sim: `users/:id/watchlist` (doc; perfil privado so para quem a pessoa aprova) | so propria; custom lists so PRO/VIP (doc) | nao no RSS | nao | `salvo` (evento) |
| Agregados (tempo, contagem) | `users/:id/stats` (doc) | `GET /users/{user_id}/stats` (doc: "a chamada mais cara da API", so por acao explicita; precisa do **id numerico**) | nao | nao | sim: `agregado` por mes |
| Episodio/temporada | sim | — | so filmes (Letterboxd nao tem series) | — | sim |
| Id do titulo | IMDb | IMDb/TMDB | **so TMDB** (`tmdb:movieId`), sem IMDb (medido) | — | IMDb |
| Recs e o que o amigo fez com elas | nao existe | nao existe | nao existe | nao existe | **so nos** |

## Autenticacao e limites

- **Trakt**: token OAuth do proprio usuario + `trakt-api-key` + User-Agent
  (sem UA a resposta e 403, medido — comentario de `idTrakt` em index.js).
  Limite de GET autenticado: 500 a 1000 / 5 min por app+usuario (doc diverge
  entre versoes; docs.trakt.tv diz 500). Perfil privado so aparece para quem
  e aprovado.
- **Simkl**: `client_id` basta para dado PUBLICO de outro usuario; perfil
  privado da `403 private_profile` (doc). 10 GET/s por client_id; abuso
  suspende o client_id "sem aviso" (doc, api-rules). Nao ha como descobrir o
  id numerico de um amigo pela API: a pessoa teria de digita-lo.
- **Letterboxd**: API oficial e **fechada** (pedido por e-mail; doc diz que nao
  libera para projeto pessoal nem para "recomendacao"). O RSS e publico, sem
  chave, 200 OK sem User-Agent especial (medido). **ToS**: proibe "robot,
  spider, scraper ... para acessar, copiar ou **monitorar**" o servico e
  exportar conteudo que nao e seu. Ler o RSS de um AMIGO a cada ciclo para
  montar feed e, na leitura mais literal, "monitorar". Recomendacao: **nao**
  ligar por padrao; se o dono quiser, so com o proprio amigo informando o
  usuario dele (consentimento do dono do diario) e cache longo (>= 1 h).
- **Nuvio/Supabase**: o token da conta so prova identidade (`/auth/v1/user`).
  O perfil (nome, avatar) mora em `profiles`, lido pelo app; o servidor nao
  le. Por isso o cliente passa a mandar nome+avatar do perfil em `/v1/eu`.

## Conclusao

1. **So o nosso servidor** tem: quem usa so conta Nuvio (sem Trakt), reacao
   (gostei/nao gostei) por titulo, estado das recs (entregue/aberta/comecou/
   terminou/reacao), abandono, "gosto parecido" entre os dois, agregados do
   mes de quem nao tem tracker, e amigo-de-amigo com nivel de privacidade.
2. **Trakt** e a unica fonte externa com grafo de amigos + atividade +
   "assistindo agora" usavel na TV, e ja e lida (`trakt_social`). Entra no
   merge como `fonte=trakt`.
3. **Simkl** nao serve para feed de amigos (sem lista de amigos, 1 titulo por
   pessoa, stats caros). No maximo um "ultimo visto" de um amigo cujo id
   numerico a pessoa digitar. Nao implementado.
4. **Letterboxd** tecnicamente e facil (RSS com nota, like, rewatch, data), so
   filmes e so TMDB id (precisa TMDB->IMDb para o dedupe). Bloqueio e de
   ToS, nao tecnico. Nao implementado; a estrutura `RecEvento` ja aceita
   `REC_FONTE_LETTERBOXD` se o dono decidir. A UI teria de pedir o usuario
   Letterboxd do amigo (texto) — e idealmente o proprio amigo o informa.

Fontes: api.simkl.org/llms.txt, api.simkl.org/api-rules,
api.simkl.org/api-reference/users.md, letterboxd.com/legal/terms-of-use,
letterboxd.com/api-beta, docs.trakt.tv/docs/rate-limiting, e curl em
letterboxd.com/dave/rss/ (02/10/2026).
