// ENQUETE NA ILHA DO RELOGIO (N3, 2.0). Contrato: docs/releases/1.8.0/N3-ENQUETE.md.
// Tabelas: migracao-009-enquete.sql. Mesma autenticacao das demais rotas (index.js
// resolve `quem` antes de chamar).
//
//   GET  /v1/enquete                 a enquete ativa para esta conta/perfil
//   POST /v1/enquete/voto            {"id","opcao"}  um voto por pessoa
//   POST /v1/enquete/optout          {"optout":1|0}  sair / voltar a receber
//
// PRIVACIDADE: so a CONTAGEM por opcao sai, e so para quem ja votou. Quem votou
// em que nunca e devolvido.

// Enquete ativa mais recente que a pessoa pode ver; null se nenhuma.
async function ativa(env, t) {
  return env.DB.prepare(
    "SELECT id, pergunta, arte, fim FROM enquete WHERE ativa = 1 AND inicio <= ? AND fim > ? " +
    "ORDER BY inicio DESC, id LIMIT 1").bind(t, t).first();
}

async function montar(env, quem, e) {
  const ops = await env.DB.prepare(
    "SELECT idx, texto FROM enquete_opcao WHERE enquete = ? ORDER BY idx").bind(e.id).all();
  const meu = await env.DB.prepare(
    "SELECT opcao FROM enquete_voto WHERE enquete = ? AND pessoa = ?").bind(e.id, quem.id).first();
  const out = { id: e.id, pergunta: e.pergunta, arte: e.arte || "", fim: e.fim,
    opcoes: (ops.results || []).map((o) => ({ idx: o.idx, texto: o.texto })),
    voto: meu ? meu.opcao : 0 };
  if (meu) {
    const c = await env.DB.prepare(
      "SELECT opcao, COUNT(*) AS n FROM enquete_voto WHERE enquete = ? GROUP BY opcao").bind(e.id).all();
    const por = new Map((c.results || []).map((r) => [r.opcao, r.n]));
    out.resultado = out.opcoes.map((o) => ({ idx: o.idx, votos: por.get(o.idx) || 0 }));
    out.total = out.resultado.reduce((s, r) => s + r.votos, 0);
  }
  return out;
}

async function saiu(env, quem) {
  return !!(await env.DB.prepare("SELECT 1 FROM enquete_optout WHERE pessoa = ?").bind(quem.id).first());
}

export async function rotaEnquete(rota, metodo, env, quem, corpo, h) {
  if (!rota.startsWith("/v1/enquete")) return null;
  const { json, erro, agora } = h;
  const t = agora();
  if (rota === "/v1/enquete" && metodo === "GET") {
    const optout = await saiu(env, quem);
    if (optout) return json({ optout: 1, enquete: null });
    const e = await ativa(env, t);
    return json({ optout: 0, enquete: e ? await montar(env, quem, e) : null });
  }
  if (rota === "/v1/enquete/voto" && metodo === "POST") {
    const id = typeof corpo?.id === "string" ? corpo.id : "";
    const opcao = Number(corpo?.opcao);
    if (!id || !Number.isInteger(opcao) || opcao < 1) return erro("enquete/opcao invalida", 400);
    const e = await env.DB.prepare("SELECT id, pergunta, arte, fim, inicio, ativa FROM enquete WHERE id = ?").bind(id).first();
    if (!e) return erro("enquete desconhecida", 404);
    if (!e.ativa || e.inicio > t || e.fim <= t) return erro("enquete encerrada", 410);
    const ok = await env.DB.prepare("SELECT 1 FROM enquete_opcao WHERE enquete = ? AND idx = ?").bind(id, opcao).first();
    if (!ok) return erro("opcao desconhecida", 400);
    const antes = await env.DB.prepare("SELECT opcao FROM enquete_voto WHERE enquete = ? AND pessoa = ?").bind(id, quem.id).first();
    if (antes && antes.opcao !== opcao) return erro("ja votou", 409);
    if (!antes)
      await env.DB.prepare("INSERT OR IGNORE INTO enquete_voto (enquete, pessoa, opcao, criado) VALUES (?, ?, ?, ?)")
        .bind(id, quem.id, opcao, t).run();
    return json({ optout: 0, enquete: await montar(env, quem, e) });
  }
  if (rota === "/v1/enquete/optout" && metodo === "POST") {
    const sair = corpo?.optout === 1 || corpo?.optout === true;
    if (sair) await env.DB.prepare("INSERT OR IGNORE INTO enquete_optout (pessoa, criado) VALUES (?, ?)").bind(quem.id, t).run();
    else await env.DB.prepare("DELETE FROM enquete_optout WHERE pessoa = ?").bind(quem.id).run();
    return json({ optout: sair ? 1 : 0 });
  }
  return erro("rota desconhecida", 404);
}
