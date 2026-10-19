-- MIGRACAO 008 — identidade social unificada (F08, 1.8.0).
-- Contrato: docs/releases/1.8.0/F08-SOCIAL-IDENTIDADE.md. Rotas: src/identidade.js.
--
-- NAO APLICADA EM PRODUCAO. Escrita em 04/10/2026 junto com o cliente; o dono
-- decide o deploy. Ordem obrigatoria: ESTA MIGRACAO PRIMEIRO, depois o
-- `wrangler deploy` do worker que a usa (o worker novo le `identidade` em toda
-- requisicao autenticada pelo Trakt e em /v1/contatos; contra o banco velho isso
-- e 500). Da raiz do repositorio:
--
--   npx wrangler@4 d1 execute nuvio-recomendacoes --remote \
--     --config servidor/recomendacoes/wrangler.toml \
--     --file servidor/recomendacoes/migracao-008-identidade.sql
--
-- SO ADITIVA E IDEMPOTENTE: duas tabelas e tres indices, todos IF NOT EXISTS.
-- Nenhuma linha existente muda de dono por causa desta migracao. A fusao de
-- dados (trakt:<slug> -> nuvio:<sub>[:<perfil>]) so acontece depois, pessoa a
-- pessoa, quando ELA pede o vinculo com as duas provas (POST
-- /v1/identidades/vincular). Worker velho com este banco continua funcionando:
-- ele nao conhece as tabelas novas.

-- Uma identidade externa ligada a uma PESSOA canonica (`pessoa.id`).
--   provedor  trakt | simkl | letterboxd
--   sujeito   trakt: slug | simkl: id numerico da conta | letterboxd: usuario
--             (minusculo). Nunca e-mail, nunca token.
--   metodo    'token'     = provado agora contra a API do provedor (o token
--                           veio no pedido e NAO foi guardado);
--             'declarado' = a pessoa digitou (Letterboxd nao tem API aberta).
--   verificado 1 so com metodo 'token'. Identidade NAO verificada nunca funde
--             pessoas, nunca resolve autenticacao e nunca deduplica amigos.
--   visivel   0 = so eu (o servidor usa para fundir/resolver, ninguem ve);
--             1 = amigos diretos (grau 1) recebem o id em /v1/contatos, para a
--                 TV deles juntar o feed do Trakt com o nosso.
CREATE TABLE IF NOT EXISTS identidade (
  provedor      TEXT NOT NULL,
  sujeito       TEXT NOT NULL,
  pessoa        TEXT NOT NULL,
  metodo        TEXT NOT NULL DEFAULT 'token',
  verificado    INTEGER NOT NULL DEFAULT 0,
  visivel       INTEGER NOT NULL DEFAULT 1,
  nome          TEXT NOT NULL DEFAULT '',
  criado        INTEGER NOT NULL,
  verificado_em INTEGER NOT NULL DEFAULT 0,
  PRIMARY KEY (provedor, sujeito, pessoa)
);
-- Uma conta de cada provedor por pessoa.
CREATE UNIQUE INDEX IF NOT EXISTS identidade_pessoa ON identidade(pessoa, provedor);
-- Uma identidade VERIFICADA pertence a uma pessoa so. Declaradas podem repetir
-- (ninguem "toma" o Letterboxd de outro so por digitar o nome primeiro).
CREATE UNIQUE INDEX IF NOT EXISTS identidade_dona ON identidade(provedor, sujeito) WHERE verificado = 1;

-- Ids que deixaram de existir por fusao: `de` (ex.: trakt:fulano) virou `para`
-- (ex.: nuvio:<sub>). TVs antigas ainda tem `de` guardado como contato, como
-- destino de recomendacao ou no perfil aberto; o worker traduz na entrada.
CREATE TABLE IF NOT EXISTS fusao (
  de     TEXT PRIMARY KEY,
  para   TEXT NOT NULL,
  criado INTEGER NOT NULL
);
CREATE INDEX IF NOT EXISTS fusao_para ON fusao(para);
