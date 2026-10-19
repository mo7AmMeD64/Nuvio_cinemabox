# Passagem 1.7.1 (02/10/2026)

Base: `origin/master` = v1.7.0 publicada (b07bb926 + docs do dono). Branch de trabalho: **`release/1.7.1`** (worktree `/private/tmp/nv-171`; o `/private/tmp` ja foi apagado 3x — so o commitado sobrevive). Versao nos arquivos ainda e 1.7.0 (bump = commit `v1.7.1` em `deploy/app/appinfo.json` + `tools/tizen-config.xml`).

## Ja mesclado em `release/1.7.1` (testes no Mac passam; NADA provado em TV salvo onde dito)

| Issue / origem | Causa (medida) | Conserto | Commits |
|---|---|---|---|
| #212 selo de visto / olho / "marcar visto nao faz nada" | Selo e olho so olhavam `progresso >= 90`; marcar zerava o progresso; historico do Trakt (`limit=100`) so tinha episodios | `cat_visto()` unica regra; `/sync/watched/movies` (so rebaixa se `/sync/last_activities` mudou); serie via `/progress/watched`; ajuste novo `AJ_SELO_VISTO` (ligado) | 5f5174ad 24a14ed0 964bb741 |
| #213 (1) CW do Trakt com series ja vistas | Serie marcada inteira = mesmo `watched_at` em todos os ep; pegava ep do meio como "ultimo" | Empate pega maior T/E; cada candidata conferida em `/shows/<id>/progress/watched` (3 em paralelo) | 28064cae |
| #213 (2) "Ends at 1" | Frase traduzida (ru 35 bytes) num buffer de 32 | `relogio_fim()` com 128 bytes; teste cobre 30 idiomas | ff1c8bbc |
| #213 (3)(4) hero/CW em ingles 1-2 s | CW do Trakt vinha com arte/texto do Cinemeta (en); traducao so em memoria | arte do addon de metadados; texto localizado gravado em `loc-texto.txt` por conta/perfil | 5658d0b0 |
| #213 (5) abertura cinza | splash padrao do webOS | `splashBackground` no appinfo + `src/abertura.c` (marca, mola, 350 ms..1,5 s) + Android windowBackground | 880a4f76 8557e459 |
| Logs 1.7.0: queda por "catalogo remontou X para X+1" | id de titulo guardado com 23 bytes (canal ao vivo, colecoes AIOM, anime) -> reinsere o titulo a cada quadro, memoria sobe ate SIGSEGV (tpk 1.7.0; tizen 1.6.5 27->107 MB) | id com 64 bytes | 2ed8cc02 |
| Seekr (pedido do dono) | seekr.tv = so miniaturas da barra de tempo (nao addon) | Ajustes > Integracoes > Seekr: ligar (off), chave pessoal mascarada (seekr.txt, por TV), testar, fita, sincronia -60..+60 s. Testado no Mac com chave real (Matrix 817 quadros, folha 3200x1800 ~22 MB decodificada, solta apos a busca). Chave NUNCA em arquivo | b3512ba9 ee198248 fc32733c fb66d609 097b41ed |
| i18n | 13 chaves novas (Seekr) + selo de visto nos 28 idiomas | | bf736a5a |
| Mescla #212 x #213 em `src/trakt.c` | chave `}` perdida na uniao | corrigido | 7a5896fa |

Teste na TCL (Android 14, APK de release do HEAD, 02/10 11:45): conta e Trakt ok, `[trakt] filmes vistos: 100`, `a seguir conferido no Trakt: 20 serie(s), 4 sem proximo`, `[abertura] fim em 934 ms`, sem crash.

## Achados do teste na TCL para quem continuar
1. `[abertura] sem a marca (.../files/res/art/marcas/nuvio_wordmark.png)`: o PNG ESTA no APK; o Android so reextrai os assets quando muda a versao (continuou 1.7.0). Some com o bump para 1.7.1, mas a extracao deveria olhar o `lastUpdateTime`/hash do APK, nao so a versao.
2. `filmes vistos: 100` exatamente — conferir se `/sync/watched/movies` esta paginando/limitando (deveria ser o mapa completo).

## Logs 1.7.0 (triagem, so informativo)
Erros de player vem das fontes (Pluto 401, IPTV 429, codec nao suportado, decoder retirado ao ir para segundo plano). Long tasks do .wgt = arranque do WASM (taxa menor que 1.6.5). "Catalog quota" e "ASS fallback" sao informativos. Recursos novos (ilha, Spotlight, teclado do sistema, Salvos) sem erro nos logs. Quedas sem causa: 1 .wgt (8x, TV fechando?) e 3 TCLs (5x, log sem o fim).

## Respostas pendentes (postar so com a release no ar)
- #212, #213: rascunhos nos relatorios dos agentes (ver mensagens de commit).
- #211 LG UK6540 nao abre: pedimos firmware e `tail /tmp/nuvio.log`; aguardando.

## Em branch, FORA da 1.7.1 (decisao do dono)
- `agente/plugins` (QuickJS + htmlq, 29 scrapers reais testados no Mac; falta TV e escolha por scraper)
- `agente/p2p` (nuvio-engine/libtorrent, `-DNV_P2P_MOTOR`; falta C9/tpk/Android)
- `agente/icones` (icones de apoiador, gate `apoiador_ativo()`=0; RISCO: activity-alias muda a entrada do launcher para todos no Android)
- `agente/ajustesux` (proposta `docs/ajustes-ux.md`, fase 1 aguardando OK) e `agente/ajustesvisual` (peles A/B)
- `agente/sidebarorg` (parado a pedido)
- `agente/guiaunicode` (outra sessao: `\u` nos generos da Live TV)
- .tpk teclado/STT = canario `NUVIO_TPK_TEXTO=1` (em master desde 1.7.0, desligado no build)

## Receita (ver memoria receita-de-release)
`export NUVIO_PROPERTIES="/Users/hrocha/Projetos/Pessoal/LG WEB/NuvioWeb-0.3.38-beta/local.properties"`; `/private/tmp/NuvioWeb-0.3.38-beta` com `@webos-tools/cli` (ares-package) e symlink do local.properties; libass WASM: `source ~/emsdk/emsdk_env.sh; sh tools/build-ass-wasm.sh` (copia em `~/.cache/nuvio-ass-wasm`). adb para a TCL pode precisar rodar fora do sandbox (`No route to host` com a porta aberta). Pacotes da 1.7.0 em `~/.cache/nuvio-rel-170`.

## Adendo: "Digitar pelo celular" (agente/celular, ja mesclado)
Todo campo que usa src/teclado.c mostra um QR para colar o texto pelo celular na mesma rede (src/celular.c: servidor HTTP so com o teclado aberto, token de uso unico, 5 min, 4 KB, sem CORS, log sem o texto). Provado no Mac e na TCL. FALTA: LG (jail/firewall deixa porta alta? log "[celular] servidor no ar"/"pagina aberta"), .tpk, iPhone/Safari; .wgt nao tem (navegador nao escuta). FALTA traduzir as frases novas (i18n.sh falha por elas). Spotlight e Busca principal nao usam teclado.c (fora).

## Adendo 2 (02/10 ~13h BRT)
Mesclados depois da primeira passagem (Mac ok; TV parcial):
- **agente/celular2**: botao de celular (src/celbotao.c) ao lado de TODO campo de texto (Spotlight, Busca, teclado.c: guia, Salvos, apelido, codigos, chaves). OK abre cartao com QR por cima; texto recebido busca direto no Spotlight/Busca. Servidor so sobe com o cartao aberto. NuvioActivity.kt reextrai os assets a cada instalacao (resolve o "sem a marca" da abertura). Provado na TCL (curl ponta a ponta). Testes de tela (celbotao_shot, teclado_shot, ilha2_shot, spotlight_shot) FALHARAM na ultima rodada so por DISCO CHEIO no Mac — rodar de novo.
- **agente/ilhasaida**: saindo do player no meio com o relogio ligado volta para a HOME e a arte encolhe ate a mini capa da ilha; ajuste novo AJ_SAIDA_PLAYER ("Ao sair do player": home / pagina do titulo). Caminho dentro de app.c sem teste automatizado; nao visto na TV.
- Efeito colateral dos testes na TCL: "Blue Lights" foi parar na watchlist/Salvos do dono por OK perdido — conferir.

**QUEDA DO SERVIDOR DA CONTA (medido 02/10 ~13h):** api.nuvio.tv /rest/v1 responde 504 (auth/v1/health 200) — do lado do Nuvio. Por isso #214 (QR HTTP 400) e #215 (sem addons + login HTTP 504) e a TCL sem addons. Rascunho de resposta curta pronto (nao postado; dono precisa liberar): "The Nuvio account server (api.nuvio.tv) is currently returning errors (504)... don't sign out... next update keeps your last add-ons".
- Em andamento: **agente/offline** (base release/1.7.1): cache em disco da ultima lista boa de addons/colecoes/biblioteca/vistos/ordem por conta+perfil, aviso "servidor fora do ar", login com mensagem clara e sem apagar sessao em 5xx; investigar o 400 do #214. Mesclar quando terminar.

Disco do Mac chegou a 136 MB livres (testes falharam por isso); liberados 8,5 GB apagando so build/ das worktrees de agentes e pacotes velhos. Continua ~99% cheio.
