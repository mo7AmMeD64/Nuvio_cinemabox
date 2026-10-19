-- MIGRACAO 006 — redesenho do Social: pessoa por PERFIL, nome de exibicao,
-- nivel de atividade (alcance), eventos do player, "assistindo agora",
-- agregados por mes e estado das recomendacoes. Contrato: docs/social-contrato.md.
--
-- RODAR UMA VEZ SO e ANTES do `wrangler deploy` (o codigo novo escreve
-- `contato.via` em todo vinculo e le `pessoa.alcance` no /v1/eu: contra o banco
-- velho isso e 500 no registro de toda TV). Da raiz do repositorio:
--
--   npx wrangler@4 d1 execute nuvio-recomendacoes --remote \
--     --config servidor/recomendacoes/wrangler.toml \
--     --file servidor/recomendacoes/migracao-006-social.sql
--
-- Os ALTER TABLE dao "duplicate column name" na segunda execucao (SQLite nao
-- tem ADD COLUMN IF NOT EXISTS): esse erro quer dizer "ja aplicada".
--
-- MIGRACAO DE IDS: NENHUMA LINHA MUDA DE DONO. O perfil PRINCIPAL da conta
-- Nuvio continua sendo `nuvio:<sub>` (o cliente so manda X-Nuvio-Perfil para os
-- outros perfis, que viram `nuvio:<sub>:<indice>`). Contatos, recs, perfil
-- publico e pedidos gravados ate hoje ficam com o principal — e o unico dono
-- possivel, porque o servidor nunca soube qual perfil estava na TV.
--
-- NINGUEM PASSA A COMPARTILHAR NADA POR CAUSA DESTA MIGRACAO: `alcance` nasce
-- -1 ("nao respondeu"), que vale como 0. Nem quem tinha `perfil.ativ` >= 1 e
-- promovido: aquela resposta era sobre "o que assisti", e os eventos novos
-- (abandono, reacao, tempo assistido) sao mais do que ela aceitou.

-- Nome: o da conta (Trakt/Supabase, verificado), o do perfil (mandado pela TV
-- em /v1/eu) e o que a pessoa digitou (/v1/eu/nome). `pessoa.nome` continua
-- sendo o nome PRONTO para exibir, recalculado no registro — por isso todas as
-- consultas antigas (contatos, recs, sugestoes) ja mostram o nome novo.
ALTER TABLE pessoa ADD COLUMN nome_conta TEXT NOT NULL DEFAULT '';
ALTER TABLE pessoa ADD COLUMN nome_perfil TEXT NOT NULL DEFAULT '';
ALTER TABLE pessoa ADD COLUMN exibicao TEXT NOT NULL DEFAULT '';
ALTER TABLE pessoa ADD COLUMN avatar_perfil TEXT NOT NULL DEFAULT '';
-- -1 nao respondeu (= 0), 0 ninguem, 1 so amigos, 2 amigos de amigos.
ALTER TABLE pessoa ADD COLUMN alcance INTEGER NOT NULL DEFAULT -1;
-- Por onde virou contato: codigo | trakt | sugestao | pedido ('' = antigo).
ALTER TABLE contato ADD COLUMN via TEXT NOT NULL DEFAULT '';
-- O que quem RECEBEU fez com a rec (so gravado se ele compartilha atividade).
ALTER TABLE rec ADD COLUMN comecou INTEGER NOT NULL DEFAULT 0;
ALTER TABLE rec ADD COLUMN terminou INTEGER NOT NULL DEFAULT 0;
ALTER TABLE rec ADD COLUMN reacao INTEGER;           -- NULL = sem reacao

-- O nome que existia era o da conta. E quem nunca teve nome (conta Nuvio:
-- user_metadata.name vazio) deixa de aparecer como UUID na TV ja nesta
-- migracao, antes de qualquer cliente novo.
UPDATE pessoa SET nome_conta = nome WHERE nome_conta = '';
UPDATE pessoa SET nome = 'Amigo #' || rowid WHERE nome = '';

-- Eventos que contam no feed. "progresso" NAO entra aqui (so mexe em `agora`
-- e nos agregados). Retencao 90 dias (limpeza diaria).
CREATE TABLE IF NOT EXISTS evento (
  id        INTEGER PRIMARY KEY AUTOINCREMENT,
  pessoa    TEXT NOT NULL,
  ev        TEXT NOT NULL,                 -- inicio|fim|abandono|reacao|salvo
  imdb      TEXT NOT NULL,
  midia     TEXT NOT NULL DEFAULT 'movie', -- movie|series
  titulo    TEXT NOT NULL DEFAULT '',
  poster    TEXT NOT NULL DEFAULT '',      -- so de host conhecido (social.js)
  temporada INTEGER NOT NULL DEFAULT 0,
  episodio  INTEGER NOT NULL DEFAULT 0,
  pct       INTEGER NOT NULL DEFAULT 0,
  reacao    INTEGER NOT NULL DEFAULT 0,    -- 1|0|-1, so ev=reacao
  rec       INTEGER NOT NULL DEFAULT 0,
  criado    INTEGER NOT NULL
);
CREATE INDEX IF NOT EXISTS evento_pessoa ON evento(pessoa, id);
CREATE INDEX IF NOT EXISTS evento_criado ON evento(criado);

-- "Assistindo agora": uma linha por pessoa, vence 15 min sem evento novo.
CREATE TABLE IF NOT EXISTS agora (
  pessoa     TEXT PRIMARY KEY,
  imdb       TEXT NOT NULL,
  midia      TEXT NOT NULL DEFAULT 'movie',
  titulo     TEXT NOT NULL DEFAULT '',
  poster     TEXT NOT NULL DEFAULT '',
  temporada  INTEGER NOT NULL DEFAULT 0,
  episodio   INTEGER NOT NULL DEFAULT 0,
  pct        INTEGER NOT NULL DEFAULT 0,
  atualizado INTEGER NOT NULL
);

-- Agregados do mes ("2026-10"): segundos assistidos; e os titulos distintos
-- (filme concluido / serie tocada) para contar filmes vistos e series em curso.
CREATE TABLE IF NOT EXISTS agregado (
  pessoa TEXT NOT NULL,
  mes    TEXT NOT NULL,
  seg    INTEGER NOT NULL DEFAULT 0,
  PRIMARY KEY (pessoa, mes)
);
CREATE TABLE IF NOT EXISTS agregado_titulo (
  pessoa TEXT NOT NULL,
  mes    TEXT NOT NULL,
  imdb   TEXT NOT NULL,
  tipo   TEXT NOT NULL,                    -- filme | serie
  PRIMARY KEY (pessoa, mes, imdb, tipo)
);
