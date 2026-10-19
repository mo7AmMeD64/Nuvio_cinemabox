# Handoff: fundos da escolha de perfil + tela de descanso (2.0)

Worktree: `/Volumes/ExternalSSD/nuvio-180-fundos-descanso`, branch `feat/perfil-fundos-descanso`, base `6e4bdfb3` (integração `codex/integration-180-glass`). Commit WIP no fim desta sessão. Ainda NÃO juntado na integração.

Mockup aprovado pelo dono: https://claude.ai/artifact/5XkkA6NoVho9DDw1E5zKy5

## Decisões do dono (05/10)
1. Padrão: **Filmes** nas TVs boas, **Luz** onde a GPU cai para efeitos mínimos (já feito em `perfilsel.c:modoFundo`).
2. Descanso: padrão **2 min**, opções Desligado / 30 s / 1 / 2 / 5 / 10 min.
3. Vitrine: as duas fontes como opção (Catálogo / Minha lista e Continuar).
4. Descanso também na tela de perfil: sim (o esmaecer roda em todas as telas, já vale).

## Feito (compila no Mac; testes de esmaecer passam; capturas olhadas)
- `gfx.c/h`: `gfx_girar(rad, cx, cy)` / `gfx_sem_girar()` — uniform `uGiro` no vertex shader, inicializado (1,0,0,0) em todo programa, desligado nas passadas internas (ESC_REAL) e no atalho de clear de tela cheia.
- `text.c/h`: estilo `TXT_DESC_HORA` (230, peso `PESO_FINO`), fonte `deploy/app/fonts/Montserrat-ExtraLight-Relogio.ttf` (subset só dígitos e `:`, 8 KB). Carregada pelo mesmo caminho da mono (`caminhoMono[2]`).
- `esmaecer.c/h`: 6 escolhas (`esmaecer_ms`, padrão `ESM_PADRAO`=3 → 2 min); estilo (`ESM_ESTILO_VITRINE/RELOGIO/ESCURECER`); com vitrine/relógio: 5 s de véu 60% e depois preto 100% + `descanso_desenhar`. `esmaecer_segura_protetor_tv()`. Teste `tests/esmaecer.sh` atualizado e passando.
- `descanso.c/h` (novo): vitrine (Ken Burns, logo ou título, meta+gêneros, 1ª frase da sinopse, cascata, troca a cada 20 s, pré-carrega o próximo, "OK para abrir" só com a Home na frente) e relógio (numeral fino, linha do minuto no destaque, data, próxima estreia da Agenda, rota anti burn-in ±140×80 px, brilho ~62%).
- `main.c`: OK com descanso no ar → `descanso_pedir_abrir()`; `esmaecer_estilo(player_aberto() ? ESCURECER : ajuste)`; `descanso_quadro(...)` por quadro.
- `app.c`: `descanso_pedido_abrir()` → `abrirPorIndice(k)` se `app_na_home()`.
- `video.c` (webOS): resposta ao screensaver da LG segura também quando o descanso do Nuvio está ligado (`segurar = 2`); `protegerScreensaver()` agora também no fim de `iniciar()`. LIMITAÇÃO: só registra depois que o módulo de vídeo subiu (primeiro trailer/filme). Conferir na C9 se o screensaver da LG não entra por cima.
- `ajustes.c` + `.inc`: `AJ_DESCANSO_ESTILO`, `AJ_DESCANSO_FONTE` no fim do enum (chaves `descansoEstiloLocal`, `descansoFonteLocal`); `V_ESMAECER` com 6 valores, rótulo "Tela de descanso"; `V_PS_FUNDO` = Filmes/Listras/Arte do perfil/Luz/Projetor; inativas; ícones; palavras de busca; nova seção "Tela de descanso" em Interface (`ajustes_ux_tela.inc`). Prévia de Ajustes (`ajcHomePerfil`) corrigida: 1 e 2 estavam trocados (Listras mostrava arte borrada); Luz/Projetor ganharam prévia simples.
- `psparede.c/h` (novo): parede por perfil em `perfilparede.txt` (12 cartazes: progresso → lista → começo da Home), gravada em `perfilsel_iniciar` para o perfil ATIVO; PIN não guarda; apagada no logout (`sync.c`).
- `psestilos.c/h` (novo): desenho de Filmes (parede girada −0,16 rad, 9 colunas alternadas, cross-fade 0,9 s ao trocar de perfil, reserva = mural do catálogo), Luz (luz segue o foco, vizinho, horizonte em y=896, grão 640×360) e Projetor (feixe = GFX_SOMBRA + duas lâminas giradas, poeira, película, risco, grão 0,07).
- `perfilsel.c`: `modoFundo()`, `montarCena()`, chamadas em iniciar/atualizar/desenhar.
- Testes de captura: `tests/descanso_shot.sh [prefixo]` (novo) e `NUVIO_PERFILSEL_FUNDO=0|3|4 NUVIO_PERFILSEL_SHOT_PREFIX=... bash tests/perfilsel.sh --capturas`.

## Falta (nesta ordem)
1. **Traduções** (28 línguas + inglês). Chaves novas que faltam em `src/idioma_tab.h`: "Luz", "Projetor", "Tela de descanso", "Estilo do descanso", "Títulos da vitrine", "Vitrine", "Relógio", "Só escurecer", "Minha lista e Continuar", "%s · T%d E%d · %s" (en "%s · S%d E%d · %s"; usar a letra de `T%dE%d` de cada língua; nas não latinas deixar igual ao inglês), "%s, %d de %s" (en "%s, %d %s"), "domingo"…"sábado" (7), "Próxima estreia", "No seu catálogo", e os 4 textos de ajuda novos (`AJ_PS_FUNDO`, `AJ_ESMAECER`, `AJ_DESCANSO_ESTILO`, `AJ_DESCANSO_FONTE` em `ajustes.c`, copiar o pt exato). Inserir ordenado por bytes, `python3 tools/idiomas.py --sincronizar`, preencher os vazios, `python3 tools/idiomas.py` tem de passar. Rodar `tools/varredura-i18n.py`.
2. `tools/arm.sh` / build ARM (gcc do webOS é mais estrito; `video.c` webOS não compila no Mac). Conferir se a fonte nova vai no pacote (`deploy/app/fonts/`) em todos os alvos (Samsung WASM tem lista de preload?).
3. Rodar só os testes do que mudou: `tests/esmaecer.sh`, `tests/perfilsel.sh`, `tests/psfundo.sh`, `tests/ajustes_padroes.sh` (padrão do esmaecer mudou de 5 para 2 min, pode precisar ajustar), teste de ordem/quantidade de opções de ajustes se existir.
4. Ajuste visual pequeno: na vitrine o logo pode subir para 820×220 (`DESC_LOGO_W/H`); vãos entre origem/logo/meta estão grandes com logo de padding transparente.
5. Na TV (C9, `arm.sh --alto-cache`; captura com `/tmp/nuvio-key` e .bmp): Filmes/Luz/Projetor na escolha de perfil (fps), descanso 30 s com vitrine e relógio, OK abrindo título, screensaver da LG não entrando por cima.
6. Juntar na integração (a outra sessão tinha conflito aberto em `ajustes.c`, `ajustes_ux_dados.inc`, `ajustes_ux_padrao.inc` — as minhas mudanças nesses arquivos estão no fim das listas).

## Cuidado
- Nada de mancha escura no centro da tela de perfil (pedido do dono de 21/09).
- `ajustes_ux_padrao.inc`: valores padrão são posicionais; acrescentei 2 no fim (estilo 0, fonte 0) e mudei o do esmaecer para 3.
