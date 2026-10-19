-- MIGRACAO 007 — resposta a uma recomendacao recebida ("Ja assisti" + gostei /
-- nao gostei + mensagem curta opcional). Rota: POST /v1/rec/resposta
-- (src/social.js, rotaRecResposta). Contrato: docs/social-contrato.md.
--
-- NAO APLICADA EM PRODUCAO. Escrita junto com o cliente em 03/10/2026 e deixada
-- para o dono decidir. RODAR UMA VEZ SO e ANTES do `wrangler deploy` do codigo
-- que a usa (rotaAmigo passa a ler `resposta`/`respondido`: contra o banco
-- velho isso e 500 no perfil do amigo). Da raiz do repositorio:
--
--   npx wrangler@4 d1 execute nuvio-recomendacoes --remote \
--     --config servidor/recomendacoes/wrangler.toml \
--     --file servidor/recomendacoes/migracao-007-resposta.sql
--
-- "duplicate column name" na segunda execucao quer dizer "ja aplicada".
--
-- A RESPOSTA NAO DEPENDE DO ALCANCE. `alcance` governa o que sai SOZINHO do
-- player (atividade automatica). A resposta e um gesto explicito da pessoa,
-- dirigido a quem mandou a recomendacao, numa tela que diz "Ana vai ver sua
-- resposta" — e so a rec mandada PARA quem responde muda.

-- Texto curto (a-z0-9 e espaco, como rec.texto; limparTexto no servidor).
ALTER TABLE rec ADD COLUMN resposta TEXT NOT NULL DEFAULT '';
-- Epoch da resposta; 0 = nunca respondeu.
ALTER TABLE rec ADD COLUMN respondido INTEGER NOT NULL DEFAULT 0;
