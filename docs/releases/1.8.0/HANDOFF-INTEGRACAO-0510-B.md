# Handoff — integração Nuvio 2.0, 05/10/2026 11:46 (-03)

Continua o [HANDOFF-INTEGRACAO-0510.md](HANDOFF-INTEGRACAO-0510.md): as regras de trabalho, a receita de build/deploy e
os caminhos de lá continuam valendo e não estão repetidos aqui. Leia aquele primeiro, depois este.
Desempenho: [HANDOFF-PERFORMANCE.md](HANDOFF-PERFORMANCE.md).

## Estado

| Item | Valor |
|---|---|
| Integração | `codex/integration-180-glass` em `4eec55a7`, sem push |
| Android TV (TCL, 192.168.1.128) | APK de `0c8a0f9d` (worktree `/Volumes/ExternalSSD/nuvio-180-apk-0c8a0f9d`), sha256 conferido |
| LG C9 (192.168.1.32) | `dcef5c14` alto-cache (worktree `/Volumes/ExternalSSD/nuvio-180-apk-dcef5c14`) — **sem** as imagens por submenu |

Outra sessão escreve direto no checkout de integração: `8364262f` e `4eec55a7` (só stubs de teste) não são deste
coordenador. Confira `git log` antes de cada merge.

## Juntado nesta sessão

| Merge | Branch | O que é | Prova |
|---|---|---|---|
| `83a8e7ad` | `fix/frost-ambiente-lg` (`f1f5034d`) | Frost e Arte borrada pelo caminho da luz ambiente; despejo único `/tmp/nuvio-fundo-<tipo>[-assado].bmp` | **C9: Frost certo na página do título** (`[cor] fundo frost conferido`, tela = esperado 94,40,35; 59–60 fps parado). Capturas em `docs/reviews/frost-c9-0510/` do repo do dono |
| `dcef5c14` | `feat/social-simkl-letterboxd` (`e0686df6`) | Linhas Simkl e Letterboxd na aba Amigos | Só Mac. Nunca visto em tela |
| `0c8a0f9d` | `feat/ajustes-imagens-submenus` (`3c2386ee`) | Imagem por categoria/submenu de Ajustes + crossfade 0,22 s | Só Mac; dono viu no Android e pediu mais (frente abaixo) |

Pontos abertos desses merges:
- `3c2386ee` pôs em `gfx_rect` (`src/gfx.c`) um retorno cedo quando `gfx_opacidade_grupo <= 0`. Código compartilhado; ninguém
  revisou quem dependia de desenhar a 0.
- O despejo do Frost roda em toda plataforma com `/tmp` gravável (um `glReadPixels` de tela cheia por tipo, por execução).
  O agente do Frost foi instruído a pôr atrás de um interruptor de diagnóstico.
- Causa do escuro do assado antigo na C9: sem prova. Só deixamos de usar aquele caminho.
- Arte borrada nunca foi vista na C9 (a TV do dono está em Frost, `fundoLocal 2`).

## Agentes em voo (o novo coordenador NÃO recebe as notificações — conferir as worktrees)

Todos partem da integração, trabalham só no Mac, sem push, com commit na própria worktree. Ao terminar: `git merge --no-ff`
na integração, testes focados, compilação inteira. Se um sumiu sem commit, retomar com agente novo a partir de `git -C <wt> diff`.

| Frente | Worktree / branch | Modelo | Estado 11:46 | Pedido do dono (palavras dele resumidas) |
|---|---|---|---|---|
| Frost: 2 defeitos no Android | `nv-frostamb`, `fix/frost-ambiente-lg` | Opus | 1 arquivo mexido, sem commit novo | (1) Home **Moderna** + Frost: o fundo fica **por cima do hero**. (2) Arte borrada certa nos Ajustes, **escura no detalhe do filme**. Reproduzir em captura no Mac antes/depois em `/Volumes/ExternalSSD/tmp/frost-android/`. Hipóteses não provadas: passo opaco de tela cheia fora de ordem; véu de 28% duas vezes ou assado feito antes da arte chegar. No Android o despejo não grava (`/tmp` não existe para o app) |
| Teclado 2.0 | `nv-teclado`, `feat/teclado-20` | Opus | 37 arquivos mexidos, sem commit | Navegação: campo e modos (TV keyboard / Speak / Type on your phone) estão à ESQUERDA mas só se chega com CIMA (`fileira == -1` em `src/teclado.c`) → Esquerda entra, Direita volta. Caps lock (um toque = próxima letra, dois = trava) e camada de símbolos, sem deixar digitar fora do alfabeto do campo. QR do celular dentro da modal (hoje `celb_abrir` desenha em outro canto), sem quebrar Spotlight e Busca. Capturas em `/Volumes/ExternalSSD/tmp/teclado-shots/`. Volta com lista de campos de alfabeto estreito para o dono decidir |
| Ajustes: itens e pílulas | `nv-ajitens`, `fix/ajustes-itens-toggles` | Opus | 4 arquivos mexidos, sem commit | (1) "Reorder and enable rows": **sumiu a linha do que está fora da Home** — achar o commit e restaurar. (2) Toggle de avançadas no índice mostra **só o olho, sem texto**. (3) As 3 pílulas do topo **um pouco maiores e coloridas**. (4) **Cada opção entendível**: se a cena não exemplifica a opção em foco, cena própria que reage ao valor; inventário em `docs/ajustes-mapa-visual.md`. Capturas em `/Volumes/ExternalSSD/tmp/ajitens-shots/`. Observado na TV: miniaturas de Background "Blurred art" e "Frost" quase iguais e escuras — repassar a esta frente |
| Explorar 2.0 | `nv-explorar`, `feat/explorar-20` | Opus | 1 arquivo mexido, sem commit | Dono: "pode já implementar o explorar com a sua recomendação" = **B (Toca do coelho) principal + grade da C como entrada pela barra lateral + botão "Explorar a partir daqui" no Detalhe**. A não é construída. Contrato = mockup `/Volumes/ExternalSSD/nv-analise/mockups-20/06-explorar.html` (publicado: https://claude.ai/artifact/31siKmoEB3qk46SGjsuXMH). Prioridade: `mapa_vizinhos` + B + botão do Detalhe → C com catálogo local → `/discover` → Salvar trilha. A tecla "I" do mockup não existe no controle: o agente mapeia e o coordenador **diz ao dono qual ficou**. Capturas em `/Volumes/ExternalSSD/tmp/explorar-shots/` |
| Player: opening source + spinner | `nv-abrindo`, `ui/abrindo-minimal` | Sonnet | commit `63df0d20` (spinner sem placa) feito; 6 arquivos mexidos em andamento | (1) Opening source **minimalista**: só selos do que a fonte tem; **Baixo** abre mais (nome completo, addon, latência, velocidade de download…); nunca URL. (2) Ao terminar de abrir, **encolhe e some como a ilha do relógio**. (3) Spinner do player **sem o fundo cinza opaco**, só as bolinhas. Se Baixo já fizer outra coisa no carregamento, o agente reporta e o coordenador **pergunta ao dono**. Capturas em `/Volumes/ExternalSSD/tmp/abrindo-shots/` |

Conflitos prováveis ao juntar: `nv-ajitens` e `nv-teclado` mexem em i18n (`src/idioma_*.h`, `idioma_tab.h` é ordenada por
bytes — reordenar por script, não à mão); `nv-explorar` e `nv-abrindo` também acrescentam chaves. Juntar um de cada vez e
rodar `python3 tools/idiomas.py` depois de cada merge.

## Esperando o dono

- Ver as capturas de cada frente antes de mandar para TV (ele pede o deploy; não instalar sozinho).
- Arte borrada na C9: trocar o ajuste dele para testar, só com ok.
- Aprovar What's New 2.0 (https://claude.ai/artifact/4BTSfCUDQ1CJ7MrxWm49S7).
- Deploy do worker da enquete + semear a primeira enquete.
- Secret `SIMKL_CLIENT_ID` no worker (sem ele a linha do Simkl responde 501 e some). O relatório do agente diz que a
  migração 008 + deploy da F08 também estão pendentes — **não conferido no D1**.
- Do handoff anterior, intocados: perfil/estatísticas, página de filme renovada, logo "Clássico renovado", respostas das
  issues, tamanho do D1.

## Aprendido hoje na bancada

- LG: OK no hero da Home durante o arranque abre o trailer em tela cheia; a captura sai **preta com o logo** porque o plano
  de vídeo não entra no BMP. Não é tela quebrada.
- LG: `sleep` do busybox não aceita fração; nunca rodar `find /` na TV (segura o ssh mais de 60 s).
- LG captura: `echo 1 > /tmp/nuvio-shot-req; chown 5152:5000 /tmp/nuvio-shot-req` → `/tmp/nuvio-shot.bmp`; teclas em
  `/tmp/nuvio-key` com o mesmo `chown`, 2 s entre elas.
- Android: `adb exec-out screencap -p` é passivo e pode ser usado com o dono navegando; `run-as` não funciona (pacote não
  depurável), então não dá para ler o `ajustes.txt` de lá.
- A saída de `tools/arm.sh` imprime a linha de compilação com as chaves do `local.properties`. Redirecionar para arquivo
  no SSD e ler só o fim (`==> conferindo`), nunca colar no chat.
- `wrangler d1 execute --file` falha (code 10000); usar `--command`.
