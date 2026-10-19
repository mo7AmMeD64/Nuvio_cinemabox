// Friend profile states (src/amigoperfil.c), no GL. M sweep, 04/10/2026.
//
// The owner's TV showed four "—" cards and "Profile unavailable" for a friend
// "via Trakt". That is not a load failure: GET /v1/amigo answers 404 for a
// person the Nuvio account does not know (servidor/recomendacoes/src/social.js
// rotaAmigo: no `pessoa` row), which the client models as REC_SOC_NAO_ACHOU,
// and the feed still carries what Trakt shared. This test pins the three
// states the screen has to tell apart:
//   - source-only friend (Trakt, 404): soFonte() and semNumeros() both true;
//   - Nuvio friend whose profile the server refuses: semNumeros() only;
//   - Nuvio friend with month numbers: neither (the 2x2 grid stays).
#define NV_REC_URL "http://127.0.0.1:8799"
// recomenda.c and amigoperfil.c both have a static `fila`/`pessoa`/`perf`...:
// rename the first one's while it is included.
#define fila recFila
#define pessoa recPessoa
#define perf recPerf
#define entrada recEntrada
#define sair recSair
#define col recCol
#define pedido recPedido
#define temPedido recTemPedido
#include "../src/recomenda.c"
#undef fila
#undef pessoa
#undef perf
#undef entrada
#undef sair
#undef col
#undef pedido
#undef temPedido
#include "../src/amigoperfil.c"
#include "dados.h"
#include <assert.h>

static void evento(SvEvento *e, const char *id, const char *nome, int fonte, int acao) {
  memset(e, 0, sizeof *e);
  snprintf(e->pessoaId, sizeof e->pessoaId, "%s", id);
  snprintf(e->pessoaNome, sizeof e->pessoaNome, "%s", nome);
  e->fonte = fonte; e->acao = acao; e->reacao = SV_REAC_NADA;
  snprintf(e->imdb, sizeof e->imdb, "tt0000104");
  snprintf(e->tipo, sizeof e->tipo, "series");
  snprintf(e->titulo, sizeof e->titulo, "The Gentlemen");
  e->temporada = 2; e->episodio = 5; e->pct = 96;
  e->quando = (long long)time(NULL) - 600;
}

static void estado(int st) {
  SDL_LockMutex(mtx);
  temAmigo = 0; amigoEstado = st;
  SDL_UnlockMutex(mtx);
}

int main(void) {
  SvEvento v[2];
  dados_iniciar("deploy/app/art");
  recomenda_iniciar();
  aparecer = REC_APARECER_SIM;
  evento(&v[0], "trakt:kevin", "Kevin", SV_FONTE_TRAKT, SV_AGORA);
  evento(&v[1], "nuvio:lia", "Lia", SV_FONTE_NUVIO, SV_FIM);
  nContatos = 0;
  memset(&contatos[0], 0, sizeof contatos[0]);
  snprintf(contatos[0].id, sizeof contatos[0].id, "trakt:kevin");
  snprintf(contatos[0].nome, sizeof contatos[0].nome, "Kevin");
  snprintf(contatos[0].origem, sizeof contatos[0].origem, "trakt");
  nContatos = 1;
  socialvis_definir_feed(v, 2);

  // 1. Trakt-only friend, server 404.
  amigoperfil_abrir("trakt:kevin");
  estado(REC_SOC_NAO_ACHOU);
  amigoperfil_atualizar(1.0f / 60.0f, SDL_GetTicks() + 2000u);
  assert(temPerfil);
  assert(perf.estado == SV_PERFIL_NAO_ACHOU);
  assert(perf.porOnde == SV_FONTE_TRAKT);
  assert(perf.minutosMes < 0 && perf.filmesMes < 0 && perf.seriesCurso < 0 && perf.recsVistas < 0);
  assert(soFonte());
  assert(semNumeros());
  // What Trakt shared still shows: he is watching right now.
  assert(perf.nAssistindo == 1 && perf.assistindo[0].acao == SV_AGORA);

  // 2. While the answer is still on its way the grid keeps its place (no
  // layout jump when the numbers arrive).
  estado(REC_SOC_INDO);
  amigoperfil_atualizar(1.0f / 60.0f, SDL_GetTicks() + 4000u);
  assert(perf.estado == SV_PERFIL_INDO);
  assert(!soFonte());
  assert(!semNumeros());

  // 3. A transient failure is not "only the source": still the grid, and the
  // existing "could not refresh" line explains it.
  estado(REC_SOC_FALHA);
  amigoperfil_atualizar(1.0f / 60.0f, SDL_GetTicks() + 6000u);
  assert(perf.estado == SV_PERFIL_FALHA);
  assert(!soFonte());
  assert(!semNumeros());

  // 4. Nuvio friend the server refuses to show: no numbers, but NOT a source.
  contatos[1] = contatos[0];
  snprintf(contatos[1].id, sizeof contatos[1].id, "nuvio:lia");
  snprintf(contatos[1].nome, sizeof contatos[1].nome, "Lia");
  snprintf(contatos[1].origem, sizeof contatos[1].origem, "nuvio");
  nContatos = 2;
  socialvis_definir_feed(v, 2);
  amigoperfil_abrir("nuvio:lia");
  estado(REC_SOC_NAO_ACHOU);
  amigoperfil_atualizar(1.0f / 60.0f, SDL_GetTicks() + 8000u);
  assert(temPerfil);
  assert(perf.porOnde == SV_FONTE_NUVIO);
  assert(!soFonte());
  assert(semNumeros());

  // 5. Nuvio friend with month numbers: the grid stays.
  perf.estado = SV_PERFIL_OK; perf.compartilha = 1;
  perf.minutosMes = 600; perf.filmesMes = 3; perf.seriesCurso = 2;
  assert(!semNumeros());
  perf.compartilha = 0;                  // private: numbers exist but are not shared
  perf.minutosMes = perf.filmesMes = perf.seriesCurso = -1;
  assert(semNumeros());

  puts("PASS: amigoperfil estados");
  return 0;
}
