// Enquete na ilha (N3, migracao 009) contra o WORKER INTEIRO
// (src/index.js -> fetch), com SQLite real em memoria e sem rede: as sessoes sao
// semeadas como preparar-local.sh faz, e as verificacoes de token no Trakt, no
// Supabase e no Simkl sao respondidas por um fetch falso. Node 22+:
//   node --test servidor/recomendacoes/teste-enquete.mjs
import assert from "node:assert/strict";
import { createHash } from "node:crypto";
import { readFileSync } from "node:fs";
import { DatabaseSync } from "node:sqlite";
import test from "node:test";
import worker from "./src/index.js";

const MIGRACOES = ["schema.sql", "migracao-001-avatar-nota.sql", "migracao-002-descobrivel.sql",
  "migracao-003-registro.sql", "migracao-004-triagem.sql", "migracao-005-amigos.sql",
  "migracao-006-social.sql", "migracao-007-resposta.sql", "migracao-008-identidade.sql", "migracao-009-enquete.sql"];

function banco(t) {
  const sqlite = new DatabaseSync(":memory:");
  t.after(() => sqlite.close());
  for (const m of MIGRACOES) {
    // schema.sql ja cria colunas que 001/002 acrescentam: "duplicate column" e
    // esperado, exatamente como em preparar-local.sh. Comando a comando.
    for (const cmd of readFileSync(new URL(m, import.meta.url), "utf8").split(/;\s*\n/)) {
      const c = cmd.replace(/^\s*--.*$/gm, "").trim();
      if (!c) continue;
      try { sqlite.exec(c); } catch (e) { if (!/duplicate column/.test(String(e.message))) throw e; }
    }
  }
  const stmt = (sql, valores) => {
    const st = sqlite.prepare(sql);
    return {
      first: async () => st.get(...valores) ?? null,
      all: async () => ({ results: st.all(...valores) }),
      run: async () => { const r = st.run(...valores);
        return { meta: { changes: Number(r.changes), last_row_id: Number(r.lastInsertRowid) } }; },
      _exec: () => st.run(...valores),
    };
  };
  const DB = {
    prepare(sql) {
      const base = stmt(sql, []);
      return { ...base, bind: (...v) => stmt(sql, v) };
    },
    async batch(lista) {
      sqlite.exec("BEGIN");
      try { for (const s of lista) s._exec(); sqlite.exec("COMMIT"); }
      catch (e) { sqlite.exec("ROLLBACK"); throw e; }
      return [];
    },
  };
  const sessao = (via, token, id, nome) => sqlite.prepare(
    "INSERT INTO sessao (hash, id, nome, expira) VALUES (?, ?, ?, 9999999999)"
  ).run(createHash("sha256").update(`${via}:${token}`).digest("hex"), id, nome);
  return { sqlite, DB, sessao };
}

// Os emissores de mentira. Trakt: token -> slug. Supabase: token -> sub e a
// lista de perfis. Simkl: token -> id da conta.
function emissores(t, { trakt = {}, nuvio = {}, perfis = {}, simkl = {}, perfisFalha = false } = {}) {
  const original = globalThis.fetch;
  globalThis.fetch = async (url, op = {}) => {
    const u = String(url);
    const tok = String(op.headers?.authorization || "").replace("Bearer ", "");
    if (u.startsWith("https://api.trakt.tv/users/settings")) {
      const slug = trakt[tok];
      return slug ? Response.json({ user: { ids: { slug }, name: slug.toUpperCase() } }) : new Response("", { status: 401 });
    }
    if (u === "https://sb.test/auth/v1/user") {
      const sub = nuvio[tok];
      return sub ? Response.json({ id: sub, user_metadata: {} }) : new Response("", { status: 401 });
    }
    if (u === "https://sb.test/rest/v1/rpc/sync_pull_profiles") {
      if (perfisFalha) return new Response("", { status: 500 });
      return Response.json((perfis[tok] || [1]).map((i) => ({ profile_index: i })));
    }
    if (u === "https://api.simkl.com/users/settings") {
      const id = simkl[tok];
      return id ? Response.json({ user: { name: "Simkl " + id }, account: { id } }) : new Response("", { status: 401 });
    }
    throw new Error("rede inesperada no teste: " + u);
  };
  t.after(() => { globalThis.fetch = original; });
}

function cliente(env) {
  // api(via, token, metodo, rota, corpo?, perfil?)
  return async (via, token, metodo, rota, corpo, perfil) => {
    const headers = { authorization: `Bearer ${token}`, "x-nuvio-auth": via };
    if (perfil) headers["x-nuvio-perfil"] = String(perfil);
    const req = new Request("https://w.test" + rota, {
      method: metodo, headers, body: metodo === "POST" ? JSON.stringify(corpo ?? {}) : undefined,
    });
    const res = await worker.fetch(req, env);
    const txt = await res.text();
    let body = null;
    try { body = txt ? JSON.parse(txt) : null; } catch { body = txt; }
    return { status: res.status, body };
  };
}

function cenario(t, opcoes) {
  const b = banco(t);
  emissores(t, opcoes);
  const env = { DB: b.DB, SUPABASE_URL: "https://sb.test", SUPABASE_ANON_KEY: "anon",
                TRAKT_CLIENT_ID: "tc", ...(opcoes?.env || {}) };
  b.sessao("nuvio", "tok-a", "nuvio:aaa", "");
  b.sessao("nuvio", "tok-b", "nuvio:bbb", "Gustavo");
  b.sessao("nuvio", "tok-c", "nuvio:ccc", "Carolina");
  b.sessao("nuvio", "tok-e", "nuvio:eee", "Elisa");
  b.sessao("trakt", "tt-p", "trakt:pedrinho", "Pedrinho");
  return { ...b, env, api: cliente(env) };
}


const q = (sqlite, sql, ...v) => sqlite.prepare(sql).all(...v);
const T = Math.floor(Date.now() / 1000);

function semear(sqlite, { id = "logo", inicio = T - 100, fim = T + 3600, ativa = 1 } = {}) {
  sqlite.prepare("INSERT INTO enquete (id, pergunta, arte, inicio, fim, ativa) VALUES (?,?,?,?,?,?)")
    .run(id, "Qual logo voce prefere?", "https://x.test/logo.jpg", inicio, fim, ativa);
  ["Novo", "Classico renovado", "Classico"].forEach((tx, i) =>
    sqlite.prepare("INSERT INTO enquete_opcao (enquete, idx, texto) VALUES (?,?,?)").run(id, i + 1, tx));
}

test("sem enquete cadastrada: nada, e sem autenticacao: 401", async (t) => {
  const { api, env } = cenario(t);
  const r = await api("nuvio", "tok-a", "GET", "/v1/enquete");
  assert.equal(r.status, 200);
  assert.deepEqual(r.body, { optout: 0, enquete: null });
  const sem = await (await import("./src/index.js")).default.fetch(new Request("https://w.test/v1/enquete"), env);
  assert.equal(sem.status, 401);
});

test("enquete ativa: opcoes sem resultado antes de votar; voto unico; contagem so agregada", async (t) => {
  const { sqlite, api } = cenario(t);
  semear(sqlite);
  const a = await api("nuvio", "tok-a", "GET", "/v1/enquete");
  assert.equal(a.body.enquete.id, "logo");
  assert.equal(a.body.enquete.opcoes.length, 3);
  assert.equal(a.body.enquete.voto, 0);
  assert.equal(a.body.enquete.resultado, undefined);

  const v = await api("nuvio", "tok-a", "POST", "/v1/enquete/voto", { id: "logo", opcao: 2 });
  assert.equal(v.status, 200);
  assert.equal(v.body.enquete.voto, 2);
  assert.deepEqual(v.body.enquete.resultado.map((r) => r.votos), [0, 1, 0]);
  // repetir a mesma opcao e idempotente; outra opcao e 409
  assert.equal((await api("nuvio", "tok-a", "POST", "/v1/enquete/voto", { id: "logo", opcao: 2 })).status, 200);
  assert.equal((await api("nuvio", "tok-a", "POST", "/v1/enquete/voto", { id: "logo", opcao: 1 })).status, 409);
  await api("nuvio", "tok-b", "POST", "/v1/enquete/voto", { id: "logo", opcao: 2 });
  await api("nuvio", "tok-c", "POST", "/v1/enquete/voto", { id: "logo", opcao: 1 });
  assert.equal(q(sqlite, "SELECT COUNT(*) n FROM enquete_voto")[0].n, 3);
  const r = await api("nuvio", "tok-a", "GET", "/v1/enquete");
  assert.deepEqual(r.body.enquete.resultado.map((x) => x.votos), [1, 2, 0]);
  assert.equal(r.body.enquete.total, 3);
  // nenhuma identidade vaza
  assert.ok(!JSON.stringify(r.body).includes("nuvio:"));
  // quem nao votou nao ve resultado
  const e = await api("nuvio", "tok-e", "GET", "/v1/enquete");
  assert.equal(e.body.enquete.resultado, undefined);
});

test("validacao: id/opcao invalidos, enquete desconhecida, encerrada e futura", async (t) => {
  const { sqlite, api } = cenario(t);
  semear(sqlite);
  semear(sqlite, { id: "velha", inicio: T - 900, fim: T - 10 });
  semear(sqlite, { id: "futura", inicio: T + 500, fim: T + 900 });
  const v = (corpo) => api("nuvio", "tok-a", "POST", "/v1/enquete/voto", corpo);
  assert.equal((await v({})).status, 400);
  assert.equal((await v({ id: "logo", opcao: "x" })).status, 400);
  assert.equal((await v({ id: "logo", opcao: 9 })).status, 400);
  assert.equal((await v({ id: "nada", opcao: 1 })).status, 404);
  assert.equal((await v({ id: "velha", opcao: 1 })).status, 410);
  assert.equal((await v({ id: "futura", opcao: 1 })).status, 410);
  assert.equal(q(sqlite, "SELECT COUNT(*) n FROM enquete_voto")[0].n, 0);
  assert.equal((await api("nuvio", "tok-a", "GET", "/v1/enquete")).body.enquete.id, "logo");
});

test("encerrada ou desligada some do GET", async (t) => {
  const { sqlite, api } = cenario(t);
  semear(sqlite, { id: "velha", fim: T - 1 });
  semear(sqlite, { id: "off", ativa: 0 });
  assert.equal((await api("nuvio", "tok-a", "GET", "/v1/enquete")).body.enquete, null);
});

test("opt-out: guardado na conta, enquete some, voltar a receber apaga a linha", async (t) => {
  const { sqlite, api } = cenario(t);
  semear(sqlite);
  assert.deepEqual((await api("nuvio", "tok-a", "POST", "/v1/enquete/optout", { optout: 1 })).body, { optout: 1 });
  assert.deepEqual((await api("nuvio", "tok-a", "POST", "/v1/enquete/optout", { optout: 1 })).body, { optout: 1 });
  assert.equal(q(sqlite, "SELECT COUNT(*) n FROM enquete_optout")[0].n, 1);
  const g = await api("nuvio", "tok-a", "GET", "/v1/enquete");
  assert.deepEqual(g.body, { optout: 1, enquete: null });
  // outra pessoa nao e afetada
  assert.equal((await api("nuvio", "tok-b", "GET", "/v1/enquete")).body.enquete.id, "logo");
  assert.deepEqual((await api("nuvio", "tok-a", "POST", "/v1/enquete/optout", { optout: 0 })).body, { optout: 0 });
  assert.equal((await api("nuvio", "tok-a", "GET", "/v1/enquete")).body.enquete.id, "logo");
});

test("perfis da mesma conta votam separado (cada perfil e uma pessoa)", async (t) => {
  const { sqlite, api } = cenario(t, { nuvio: { "sb-a": "aaa" }, perfis: { "sb-a": [1, 2] } });
  semear(sqlite);
  assert.equal((await api("nuvio", "sb-a", "POST", "/v1/enquete/voto", { id: "logo", opcao: 1 })).status, 200);
  const p2 = await api("nuvio", "sb-a", "GET", "/v1/enquete", null, 2);
  assert.equal(p2.status, 200);
  assert.equal(p2.body.enquete.voto, 0);
});
