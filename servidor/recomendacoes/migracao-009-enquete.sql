-- MIGRACAO 009 — enquete na ilha do relogio (N3, 2.0).
-- Contrato: docs/releases/1.8.0/N3-ENQUETE.md. Rotas: src/enquete.js.
--
-- NAO APLICADA EM PRODUCAO, e NAO SEMEIA NADA: nenhuma enquete nasce desta
-- migracao. Ordem obrigatoria: ESTA MIGRACAO PRIMEIRO, depois o `wrangler
-- deploy` do worker (o worker novo consulta `enquete` em GET /v1/enquete; contra
-- o banco velho isso e 500, mas so essa rota). Da raiz do repositorio:
--
--   npx wrangler@4 d1 execute nuvio-recomendacoes --remote \
--     --config servidor/recomendacoes/wrangler.toml \
--     --file servidor/recomendacoes/migracao-009-enquete.sql
--
-- SO ADITIVA E IDEMPOTENTE (IF NOT EXISTS). Worker velho com este banco segue
-- funcionando: ele nao conhece as tabelas novas.

-- A pergunta. `inicio`/`fim` em segundos unix; ativa = 0 desliga na hora.
-- `arte` e uma URL https 480x270 (vazia = sem arte).
CREATE TABLE IF NOT EXISTS enquete (
  id        TEXT PRIMARY KEY,
  pergunta  TEXT NOT NULL,
  arte      TEXT NOT NULL DEFAULT '',
  inicio    INTEGER NOT NULL,
  fim       INTEGER NOT NULL,
  ativa     INTEGER NOT NULL DEFAULT 1
);

-- As opcoes, de 2 a 3 (o modal da ilha tem ate 3 botoes). `idx` comeca em 1.
CREATE TABLE IF NOT EXISTS enquete_opcao (
  enquete TEXT NOT NULL,
  idx     INTEGER NOT NULL,
  texto   TEXT NOT NULL,
  PRIMARY KEY (enquete, idx)
);

-- UM VOTO POR PESSOA POR ENQUETE (chave primaria). `pessoa` e a identidade
-- canonica da conta/perfil, a mesma de todo o servico; nunca e devolvida a
-- ninguem — so a contagem sai.
CREATE TABLE IF NOT EXISTS enquete_voto (
  enquete TEXT NOT NULL,
  pessoa  TEXT NOT NULL,
  opcao   INTEGER NOT NULL,
  criado  INTEGER NOT NULL,
  PRIMARY KEY (enquete, pessoa)
);
CREATE INDEX IF NOT EXISTS enquete_voto_opcao ON enquete_voto(enquete, opcao);

-- "Nao receber mais enquetes": uma linha por pessoa que saiu. Apagar a linha
-- (POST /v1/enquete/optout {"optout":0}) volta a receber.
CREATE TABLE IF NOT EXISTS enquete_optout (
  pessoa TEXT PRIMARY KEY,
  criado INTEGER NOT NULL
);
