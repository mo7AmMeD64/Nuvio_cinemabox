// Maquina de reconexao quando a rede cai no meio do video (video_reconexao.h).
#include <assert.h>
#include <stdio.h>
#include "../src/video_reconexao.h"

int main(void) {
  NvReconexao r;
  unsigned t = 1000;

  // Esperas 3 s, 10 s, 25 s.
  assert(nv_recon_espera_ms(1) == 3000 && nv_recon_espera_ms(2) == 10000 &&
         nv_recon_espera_ms(3) == 25000);

  // Erro que nao e de rede, sem queda em curso: desiste na hora.
  nv_recon_zerar(&r);
  assert(!nv_recon_erro(&r, 0, t, 600.0));
  assert(!nv_recon_ativa(&r));

  // Queda de rede aos 600 s: tentativa 1 agendada para +3 s, alvo = 600.
  assert(nv_recon_erro(&r, 1, t, 600.0));
  assert(r.tentativa == 1 && r.pendente && r.alvo == 600.0 && nv_recon_ativa(&r));
  // Erro repetido da sessao morta enquanto espera: nao gasta tentativa.
  assert(nv_recon_erro(&r, 1, t + 10, 0.0) && r.tentativa == 1);
  assert(!nv_recon_vencida(&r, t + 2999));
  assert(nv_recon_vencida(&r, t + 3000));
  assert(!nv_recon_vencida(&r, t + 3001));   // uma vez so

  // O recarregar nao abriu (qualquer erro conta): tentativa 2, +10 s, alvo mantido.
  t += 5000;
  assert(nv_recon_erro(&r, 0, t, 0.0));
  assert(r.tentativa == 2 && r.alvo == 600.0);
  assert(!nv_recon_vencida(&r, t + 9999) && nv_recon_vencida(&r, t + 10000));
  // Posicoes antes do seek de retomada nao contam como "voltou".
  nv_recon_progresso(&r, 3.0);
  assert(r.tentativa == 2);

  // Terceira: +25 s.
  t += 12000;
  assert(nv_recon_erro(&r, 1, t, 0.0) && r.tentativa == 3);
  assert(!nv_recon_vencida(&r, t + 24999) && nv_recon_vencida(&r, t + 25000));
  // Quarta falha: esgotou, falhou.
  t += 30000;
  assert(!nv_recon_erro(&r, 1, t, 0.0));
  assert(r.esgotou && !nv_recon_ativa(&r));
  assert(!nv_recon_erro(&r, 1, t + 1, 0.0));

  // Queda superada: andou 10 s alem do ponto da queda, a proxima comeca do zero.
  nv_recon_zerar(&r);
  assert(nv_recon_erro(&r, 1, t, 100.0));
  assert(nv_recon_vencida(&r, t + 3000));
  nv_recon_progresso(&r, 100.0);
  assert(r.tentativa == 1);
  nv_recon_progresso(&r, 109.9);
  assert(r.tentativa == 1);
  nv_recon_progresso(&r, 110.0);
  assert(r.tentativa == 0 && !nv_recon_ativa(&r));
  assert(nv_recon_erro(&r, 1, t + 50000, 400.0));
  assert(r.tentativa == 1 && r.alvo == 400.0 && r.quando == t + 50000 + 3000);

  // Relogio que da a volta (SDL_GetTicks em 32 bits).
  nv_recon_zerar(&r);
  assert(nv_recon_erro(&r, 1, 0xFFFFFFF0u, 5.0));
  assert(!nv_recon_vencida(&r, 0xFFFFFFFFu));
  assert(nv_recon_vencida(&r, 0xFFFFFFF0u + 3000u));

  // Classe do erro.
  assert(!nv_recon_rede_ums(200) && !nv_recon_rede_ums(203) && !nv_recon_rede_ums(210));
  // Medidos nos logs da LG: 300 Network Error e 4xxxx server error sao rede.
  assert(nv_recon_rede_ums(300) && nv_recon_rede_ums(40400) && nv_recon_rede_ums(40403));
  assert(!nv_recon_rede_ums(100) && !nv_recon_rede_ums(204) && !nv_recon_rede_ums(700) &&
         !nv_recon_rede_ums(600) && !nv_recon_rede_ums(-1) && !nv_recon_rede_ums(0));
  assert(nv_recon_rede_tpk((int)0xFE6C0026u, NULL));
  assert(nv_recon_rede_tpk(0, "erro ConnectionFailed"));
  assert(!nv_recon_rede_tpk(-1, "erro NotSupportedFile"));
  assert(!nv_recon_rede_tpk(-1, NULL));
  assert(nv_recon_rede_avplay("PLAYER_ERROR_CONNECTION_FAILED"));
  assert(nv_recon_rede_avplay("player_error_network_disconnected"));
  assert(!nv_recon_rede_avplay("PLAYER_ERROR_NOT_SUPPORTED_FILE"));
  assert(!nv_recon_rede_avplay(""));

  printf("reconexao: ok\n");
  return 0;
}
