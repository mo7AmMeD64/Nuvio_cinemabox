-- MIGRACAO 005 — perfil publico opcional, busca, pedidos de amizade, bloqueio
-- e atividade dos amigos ("Entre amigos" alem do Trakt).
-- Modelo de privacidade: docs/SOCIAL-PRIVACIDADE.md.
--
-- SO ADITIVA, e SEM ALTER TABLE: cinco tabelas novas (CREATE ... IF NOT
-- EXISTS), nenhuma coluna tocada. Por isso e SEGURA DE RODAR MAIS DE UMA VEZ —
-- ao contrario das migracoes 001 e 002, que dao "duplicate column name" na
-- segunda execucao. `pessoa.descobrivel` (migracao 002) continua sendo O
-- sinalizador de "aparecer para outras pessoas": nada foi renomeado.
--
-- NINGUEM APARECE POR CAUSA DESTA MIGRACAO. `perfil` nasce vazia; uma pessoa so
-- entra nas buscas depois de publicar apelido (POST /v1/perfil) com
-- `descobrivel = 1`. Quem ja tinha respondido SIM na aba Social continua
-- descobrivel = 1, mas sem apelido nao e achado por busca — so aparece nas
-- sugestoes de amigo-de-amigo, como antes.
--
-- APLICAR NO BANCO DE VERDADE, da raiz do repositorio, ANTES do `wrangler
-- deploy` (o codigo novo le estas tabelas em toda rota de amigos e, contra o
-- banco velho, responde 500):
--
--   npx wrangler@4 d1 execute nuvio-recomendacoes --remote \
--     --config servidor/recomendacoes/wrangler.toml \
--     --file servidor/recomendacoes/migracao-005-amigos.sql

-- O que a pessoa escolheu mostrar e o handle publico. `pub` e o UNICO
-- identificador que estranhos veem (o id da conta, `nuvio:<uuid>`, nunca sai).
-- Sorteado de novo ao despublicar, para links/telas antigas nao acharem mais.
CREATE TABLE IF NOT EXISTS perfil (
  pessoa      TEXT PRIMARY KEY,
  pub         TEXT NOT NULL UNIQUE,
  apelido     TEXT NOT NULL DEFAULT '',
  apelido_norm TEXT NOT NULL DEFAULT '',   -- a-z0-9, para a busca por prefixo
  bio         TEXT NOT NULL DEFAULT '',
  generos     TEXT NOT NULL DEFAULT '',    -- ids de uma lista fechada, com virgula
  com_avatar  INTEGER NOT NULL DEFAULT 0,  -- 1 = mostrar a foto da conta
  recentes    INTEGER NOT NULL DEFAULT 0,  -- 1 = "vistos recentemente" no cartao publico
  -- Atividade para AMIGOS (nao e publica): 0 nada, 1 o que assisti, 2 tambem
  -- "assistindo agora".
  ativ        INTEGER NOT NULL DEFAULT 0,
  atualizado  INTEGER NOT NULL
);
CREATE INDEX IF NOT EXISTS perfil_norm ON perfil(apelido_norm);

-- Pedido de amizade. estado 0 = pendente; 1 = recusado (em silencio: continua
-- ali para o mesmo remetente nao reenviar todo dia).
CREATE TABLE IF NOT EXISTS pedido (
  de     TEXT NOT NULL,
  para   TEXT NOT NULL,
  criado INTEGER NOT NULL,
  estado INTEGER NOT NULL DEFAULT 0,
  PRIMARY KEY (de, para)
);
CREATE INDEX IF NOT EXISTS pedido_para ON pedido(para, estado, criado);

CREATE TABLE IF NOT EXISTS bloqueio (
  quem   TEXT NOT NULL,
  alvo   TEXT NOT NULL,
  criado INTEGER NOT NULL,
  PRIMARY KEY (quem, alvo)
);
CREATE INDEX IF NOT EXISTS bloqueio_alvo ON bloqueio(alvo);

-- Uma linha por (pessoa, titulo), a mais recente; no maximo 30 por pessoa e 30
-- dias. SEM poster: a capa e montada na TV de quem ve, a partir do imdb.
CREATE TABLE IF NOT EXISTS atividade (
  pessoa TEXT NOT NULL,
  imdb   TEXT NOT NULL,
  tipo   TEXT NOT NULL DEFAULT 'movie',
  titulo TEXT NOT NULL DEFAULT '',
  ano    TEXT NOT NULL DEFAULT '',
  nota   INTEGER NOT NULL DEFAULT 0,
  acao   INTEGER NOT NULL DEFAULT 0,      -- 0 assistiu, 1 assistindo agora
  criado INTEGER NOT NULL,
  PRIMARY KEY (pessoa, imdb)
);
CREATE INDEX IF NOT EXISTS atividade_criado ON atividade(criado);
-- taste: "quem viu este titulo", so entre quem publicou recentes
CREATE INDEX IF NOT EXISTS atividade_imdb ON atividade(imdb);

-- Contador de uso por (acao, pessoa) numa janela fixa; limpo quando expira.
CREATE TABLE IF NOT EXISTS limite (
  chave  TEXT PRIMARY KEY,
  janela INTEGER NOT NULL,
  n      INTEGER NOT NULL,
  expira INTEGER NOT NULL
);
