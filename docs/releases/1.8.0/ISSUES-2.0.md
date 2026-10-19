# Issues abertas x integração 2.0

Auditoria de 05/10/2026. Código: `codex/integration-180-glass` @ `09124343` (local, sem push). Base de comparação: tag `v1.7.4` (última release). 44 issues abertas no GitHub `iqui27/nuvio-native-legacy`. Só leitura: nada editado, nada comentado, nenhum teste de TV.

## Contagem

| Categoria | Qtd | Issues |
|---|---|---|
| RESOLVIDA NA 2.0 | 11 | #223, #231, #233, #235, #237, #238, #239, #243, #244, #245, #249 |
| RESOLVIDA ANTES | 5 | #188, #197, #202, #209, #227 |
| PARCIAL | 12 | #144, #158, #172, #226, #234, #241, #246, #247, #252, #255, #256, #261 |
| SÓ EM OUTRA BRANCH | 0 | — |
| NÃO ATENDIDA | 5 | #250, #253, #254, #258, #260 |
| PRECISA DE PROVA EM TV / INFO DO RELATOR | 6 | #134, #171, #211, #228, #232, #240 |
| NÃO É BUG / FORA DO ESCOPO | 5 | #135, #155, #192, #242, #257 |
| **Total** | **44** | |

## O que bloqueia a release, na minha opinião

Nenhuma issue de crash ou perda de dado está sem conserto no código. O que pesa é falta de prova em TV:

1. **#223 Android TV, login/QR com tela preta.** O conserto (CA embutida, erro do curl, sessão pela metade descartada) está no HEAD, mas o relator só testou até a 1.7.4. Testar o APK 2.0 em Android 11 antes de publicar.
2. **#158 LG C4, Live TV e Continuar.** Conta só TS não decodifica na C4 (relato de 01/10). Legendas por addon e CW offline (aad1c947) só existem na integração. Rodar na C9/C4 antes de dizer que atende.
3. **#233, #254, #255 Samsung .tpk, catálogos e coleções da conta/Home.** #233 tem conserto grande (3b625f58) sem prova na TV. #254 (fantasmas ao remover addon) não tem conserto nenhum.
4. **#211 LG UK6540PSB, fecha ao abrir.** Crash de arranque sem log. Não dá para garantir que a 2.0 abre em webOS 4. Tentar de novo pedir o log.
5. **Árabe na Samsung (#253, #258).** Não é crash, mas são 7 issues de árabe abertas e a Samsung não tem fonte árabe: forma/RTL entrou, a fonte não. Embarcar um subconjunto de fonte árabe é o item de maior retorno.

Não bloqueiam: #134, #171 (recursos novos, sem prova em TV; sair com a ressalva), #228, #232, #241 (Samsung, dependem de teste do relator).

## Fora da integração (branches)

- `agente/icones` (5 commits: e2e54440, e7499919, 4aa68d99, 40f62fcb, e6010365): galeria de ícones do app, atrás de `apoiador_ativo()`. Ajuda #226 e #234 (item do ícone). Merge provavelmente simples nos arquivos de ícone; o Android usa activity-alias e precisa de teste.
- `feat/vidaa` (8 commits): porta VIDAA do #135. Não é parte do pacote; segue separada.
- `perf/trim-fileira` (b4b007f2, 1 commit): onTrimMemory no Android, perf; não cita issue aberta.
- `agente/i195*`, `agente/i203`, `fix/tpk-selo-hdr`: já têm o equivalente em releases ≤1.7.4 (1.6.3 etc.) ou tratam issues já fechadas. Nenhuma issue aberta depende delas.
- `agente/p2p`, `agente/plugins`: linhas antigas; a 2.0 usa f10-p2p e f09-plugins.

## Tabela

Legenda: evidência = commit e arquivo:linha no HEAD `09124343`. "≤1.7.4" = já está na tag v1.7.4.

| # | Título | Plataforma | Categoria | Evidência | O que falta |
|---|---|---|---|---|---|
| #134 | Please support plugin | LG webOS | PRECISA DE PROVA EM TV / INFO DO RELATOR | f09-plugins (merge d5e84282): src/pluginjs.c (QuickJS), src/plugins.c, src/pluginsui.c, src/htmlq.c; docs/releases/1.8.0/F09-PLUGINS.md | Nenhum teste em TV nem com scraper real (repo All-in-One do relator). Caminhos .wgt e Tizen 4/5 não compilados. Sincronização da tabela `plugins` no backend assumida. |
| #135 | Hisense VIDAA: pedido de testers | VIDAA | NÃO É BUG / FORA DO ESCOPO | Porta hospedada (Worker /tv/), não é pacote da release. Código só em feat/vidaa (8 commits fora da integração, ex. c1ddedb5, 70b27fd5). | Nada a fazer na 2.0. Não prometer VIDAA nas notas. Relator parou no modo desenvolvedor da TV. |
| #144 | Alinhar UI com o nuvioTV (Android) | LG webOS | PARCIAL | 1.5.x–1.6.x: b7d10412 (cantos do véu do Continuar), 98160e65 (letra estilizada vira comum). 2.0: Glass UI inteira (merges feat/glass-ilha, ui/w*). | Pedido aberto por natureza. Cantos cortados e cintilação no Continuar (30/09) sem confirmação de que sumiram. Sem prova em TV. |
| #155 | Obrigado por construir isto | — | NÃO É BUG / FORA DO ESCOPO | Agradecimento. | Pode fechar. |
| #158 | Live TV Xtream: canais não tocam, guia vazio (C4) | LG webOS | PARCIAL | Crash ao abrir canal: 1.5.2. EPG do provedor: 1.5.3. Modos A/B/C/D e proxy TS: 31a3da42, 5656c662, 22057178 (todos ≤1.7.4). Legendas por addon e CW local offline: aad1c947 (só na integração; src/addons.c, src/descoberta.c, tests/cwlocal.c). | Conta nova só TS: dado chega mas a C4 não decodifica (relato de 01/10). Flix-Streams sem resultado e CW que não aparece: não confirmados (docs/issues/158 não prova o caso dele). Precisa de log novo na 2.0. |
| #171 | Addons P2P/Torrentio sumidos das fontes | Samsung .tpk 6+ | PRECISA DE PROVA EM TV / INFO DO RELATOR | Resposta do dono: sem debrid/P2P ficam de fora de propósito. 2.0: f10-p2p (775f9db9, src/p2pmotor.c, src/p2p.c); debrid local src/debrid.c. | P2P embutido compila no .tpk 6+ mas nunca rodou em TV Samsung (F10-P2P.md). Pergunta respondida; só vale prova. |
| #172 | UI: ideias do fork Corby7 | LG webOS | PARCIAL | Texto abre inteiro, sem palavra a palavra: a65080ae, 414cd61e (1.6.x). 2.0: Glass UI. | Resto é direção de design, sem critério de pronto. Relator testou só na C3. |
| #188 | Vídeo não aparece, só áudio (S90D) | Samsung .tpk 6+ | RESOLVIDA ANTES | 1.6.3: fee4e632, 359937e6, 5b0e4957 (tpk host, janela de vídeo). Presente no HEAD por herança do v1.7.4. | Relator não confirmou depois da 1.6.3. Aguardar ou fechar por falta de retorno. |
| #192 | Relatório de logs | — | NÃO É BUG / FORA DO ESCOPO | Issue de triagem automática; não é bug. | Manter ou fixar fora da lista de bugs. |
| #197 | Catálogos não aparecem (3 fileiras) | Samsung .tpk 5.5 | RESOLVIDA ANTES | 1.6.4: b98ae904 e f5813122 (limpeza única das fileiras). Dono confirmou pelos logs do whoareux3. | praveencudz: Bharat Binge desligado no perfil (configuração, não bug). Rolagem que seleciona sozinha: sem resposta. |
| #202 | Cor de destaque igual esconde o texto | LG webOS | RESOLVIDA ANTES | 1.7.0: c522e1f2 (src/ajustes.c:95-103, tinta por contraste). 2.0 reforça: b6e14d69. | Relator não respondeu. Pode fechar. |
| #209 | Título/sinopse de série em inglês | LG webOS | RESOLVIDA ANTES | 1.7.0: 0da72c2a (TMDB em outro idioma vence o texto do addon). Em v1.7.0 e v1.7.1. | Em 2.0 o catálogo catalog.nuvio.tv virou a 1ª fonte (ecf852ea): rever com TMDB em pt-BR. Sem teste novo que cubra. |
| #211 | LG UK6540PSB fecha ao abrir | LG webOS (2018, webOS 4) | PRECISA DE PROVA EM TV / INFO DO RELATOR | Nada no código. Dono pediu versão do webOS e /tmp/nuvio.log; sem resposta. | Crash de arranque sem dados. Não dá para saber se a 2.0 roda nessa TV. Pedir log de novo. |
| #223 | Android TV: tela preta após 'Preparando QR' (TCL, Android 11) | Android TV | RESOLVIDA NA 2.0 | 665646f6 / merge 5ca3ec85: erro do curl na tela de login, CA embutida em toda requisição (src/rede.c:990,1711,2011; main.c:862 + deploy/app/art/discord-ca.pem), sessão pela metade descartada (src/sessao.c:588). Android 1.7.2: 105e759b. | Relator confirmou 60 FPS na 1.7.4 e falha de TLS. Falta ele testar a 2.0. A espera de 2 s em pedirSuperficie (NuvioActivity.kt:213) segue igual (sem causa provada). |
| #226 | Ícone antigo / alternativo | Todas | PARCIAL | 54316088 (merge de97342e): 'Logo do app' Novo\|Clássico e 'Abertura do app' em Ajustes (src/logoapp.c). Splash neutro: 1b832a97. | O ícone do launcher não troca. Galeria de ícones só em agente/icones (5 commits fora; Android com activity-alias; atrás de apoiador_ativo()). Merge parece direto, mas é Android e tem portão de apoiador. |
| #227 | Título localizado em algum lugar | Todas | RESOLVIDA ANTES | 1.7.4: 6d74a0ac, f1903d6c. Ainda no HEAD: src/detail.c:3713 e 3827 (nomeAbaixo/hCaption). | Pode fechar após o relator confirmar. |
| #228 | Trailer automático não toca (hero e pôster) | Samsung .tpk 6+ (Q80A) | PRECISA DE PROVA EM TV / INFO DO RELATOR | 1e03fc30 (ordem de fallback e estado nativo do trailer; hero com índice de fileira retido); src/home.c, src/trailer.c, src/trailerfonte.c; docs/issues/228-home-trailers.md | Relator ainda falha na 1.7.4. O próprio doc diz que não prova a causa dele. Precisa teste na Q80A com a 2.0 e log. |
| #231 | Opção para desligar o Cinemeta | Samsung/LG | RESOLVIDA NA 2.0 | d6495798: opção 'Buscar no Cinemeta' (src/ajustes.c:1636, src/descoberta.c:739). | Cobre a busca (o que o relator pediu). Não desliga o Cinemeta na ficha do título. |
| #232 | Miniatura do próximo ep. não desfocada | Não dita | PRECISA DE PROVA EM TV / INFO DO RELATOR | Popup pós-reprodução: 2495e39e (1.7.4, src/posplay.c:389,459). Home hero/cards: d6495798 (src/home.c:785,820,859). | Relator ainda vê o erro na 1.7.4 e mandou log (04/10). Falta saber plataforma e se 'Miniatura do episódio' está ligada. |
| #233 | Conta Nuvio não sincroniza catálogos/ordem | Samsung .tpk 6+ | RESOLVIDA NA 2.0 | 1.7.4 (limite de 64 entradas); 3b625f58: ids estáveis, ordem/oculto, editor igual à Home (src/colecoes.c, src/fileiras.c, src/colfileiras.c; tests/colfileiras_sync.c). | Sem prova na Samsung do relator. Relato de 03/10 (itens velhos em 'Fora da Home') é o que o 3b625f58 ataca. |
| #234 | Ícone/splash, título do trailer (OLED), botão de trailer | LG webOS | PARCIAL | Splash neutro 1b832a97; logo Clássico 54316088; título some durante trailer com OLED 3fc5a37b (src/home.c:3211); esmaecer 973a3780; botão 'Assistir trailer' 652b759d (src/detail.c:2042). | Ícone do launcher não troca (ver #226; branch agente/icones). |
| #235 | Seek aplicado na hora | Todas | RESOLVIDA NA 2.0 | 652b759d: janela de 1 s com Seekr (src/salto.h:85-88, src/player.c:2682). Sem Seekr segue 420 ms. | Só vale com miniaturas Seekr ligadas, que era o contexto do relator. |
| #237 | Falta '/' na entrada de canais (Stalker/Xtream) | LG/Samsung/Android | RESOLVIDA NA 2.0 | e5a848c3, 8f8be06a (merge b1088476): src/ajustes.c:829 (alfabeto com / e _), src/stalker.c (prefixo de caminho). | Sem prova em TV; testes de host (tests/stalker_portal.c). |
| #238 | 'Esperar add-ons' travado pelo efeito de profundidade | Todas | RESOLVIDA NA 2.0 | 73f93686 e d6495798: src/ajustes.c:4419 (AJ_FONTE_PRAZO sem dependência). | — |
| #239 | Árabe: letras desconectadas | LG webOS (C1) | RESOLVIDA NA 2.0 | 6bb186ac (merge d00870f0): src/bidi.c, src/text.c:751 (txt_bidi_legenda), src/player.c:3135, src/legendasui.c:708. Fonte do sistema LG: src/text.c:1147. | Sem prova em TV. Caminhos libass/ASS não passam pelo bidi (legenda embutida ASS segue sem forma). |
| #240 | Demora para carregar título (Torbox+addons) | LG webOS | PRECISA DE PROVA EM TV / INFO DO RELATOR | Perf 2.0: perf/b2-arranque, ed27533e (abre título na hora), b3-texturas. Nada específico do relato. | Sem log, sem métrica. 'Addons permitidos' (filtro por addon) não existe. Pedir log. |
| #241 | Barras pretas no trailer voltaram | Samsung .tpk 6+ (AU7000) | PARCIAL | 50ba9279, 9a0cce0f (merge 50df3f42): Ajustes > Trailers > Zoom (experimental); src/ajustes.c:363,4825; src/video_tpk.c:491. | Padrão é Desligado: sem ligar, as barras continuam. A ajuda diz que pode dar tela preta em algumas Samsung. Sem prova na AU7000. |
| #242 | Pergunta: chave Torbox/Debrid da TV vale nos addons? | Todas | NÃO É BUG / FORA DO ESCOPO | Sim para streams só com infoHash: src/debrid.c (chave da conta ou digitada na TV). | Responder e fechar. |
| #243 | Nota IMDb ausente e '14' no Continuar | Samsung .tpk 5.0 | RESOLVIDA NA 2.0 | 8e7f4098 (tira o '14' inventado), ccfe01a8 (Cinemeta uma vez por obra): src/trakt.c, src/simkl.c, tests/trakt_cw_dup.c. | Sem prova em TV. Relator tem forks próprios (fix/cw-*) para comparar. |
| #244 | Continuar: mesmo ep. 5x e 'visto' não remove | Samsung .tpk 5.0 | RESOLVIDA NA 2.0 | 8e7f4098 + d6495798 (merge 67065f88, 8d1acd4b): src/trakt.c:1256,1398 (dedupObras), src/ctxmenu.c:723, tests/cwdup244.c, tests/trakt_cw_dup.c. | Troca de fonte do CW refazendo a fileira é 'hipótese não provada em dispositivo' (mensagem do commit). |
| #245 | Árabe: letras soltas e ordem errada | LG webOS | RESOLVIDA NA 2.0 | Mesmo conserto do #239 (6bb186ac). | Sem prova em TV. |
| #246 | HEVC anime: seek não pula | webOS (provável) | PARCIAL | Só diagnóstico: 023497dd (merge 394f9b7a), src/video.c:136,1805. | Nenhum conserto. O log novo só vale depois que o relator rodar a 2.0. Plataforma não informada. |
| #247 | Legenda árabe (codificação) | LG webOS | PARCIAL | Forma/RTL: 6bb186ac. Decodificação: src/legenda.c:495-560 só trata UTF-8/16, 1251 e 1252. | Windows-1256 (árabe) não é detectado: legenda nesse codepage sai errada mesmo com bidi. |
| #249 | Manter vídeo cheio com o card 'Próximo' | Todas | RESOLVIDA NA 2.0 | dee639f7 (merge a202c686): src/posplay.c:89 (posplay_sobre_video), src/player.c:2662. Só séries; filme (relacionados) ainda recua o vídeo. | Comentário em posplay.c:411 diz '52%' e está velho. Sem opção para escolher; o padrão já é vídeo cheio. |
| #250 | Idioma árabe na interface | Todas | NÃO ATENDIDA | 29 tabelas idioma_*.h, nenhuma 'ar' (ls src/idioma_*.h). | Feature nova: tabela de textos + fonte árabe + RTL. Fora do prazo da 2.0, salvo decisão do dono. |
| #252 | Idioma de áudio padrão para anime | LG webOS | PARCIAL | 'Idioma do áudio' global já escolhe a faixa a cada título: src/ajustes.c:850, src/video.c:876. | Não existe opção só para anime. Faixa sem tag de idioma é ignorada (video.c:884), comum em anime. Sem prova. |
| #253 | Árabe vira quadrados (UI e legenda) | Samsung .tpk 5 | NÃO ATENDIDA | src/text.c:1147-1152: fontes árabes só LG (/usr/share/fonts/DroidNaskh...), macOS e Android. deploy/app/fonts não tem fonte árabe. | Embarcar subconjunto de fonte árabe (como DroidSansFallback-Subset). UI árabe e RTL não existem (#250, #260). Sem isso Samsung segue em quadrados. |
| #254 | Home não atualiza ao instalar/remover addon (fantasmas) | Samsung .tpk 6 | NÃO ATENDIDA | Nenhum commit trata remoção de addon. Relacionado e não provado: 3b625f58 (coleções), f5813122 (limpeza, 1.6.4). | Reproduzir: instalar/remover addon com catálogos e coleções. Sem log do relator. |
| #255 | Home mostra só 5 coleções | Samsung .tpk 6 | PARCIAL | 3b625f58: coleções não consomem a cota de catálogos; src/home.c, src/fileiras.c. Existe 'Limite de fileiras' (src/ajustes.c:876) e o corte conta coleções (home.c:2336). | Não ficou provado que o limite era o da TV. A UI não avisa quando corta. |
| #256 | Próximo ep. aparece cedo e não dá autoplay | Não dita | PARCIAL | Aparecer cedo: cadeia de créditos (capítulo MKV, IntroDB, ep. vizinho, offset aprendido): dee639f7 (src/credfonte.c, src/player.c). Contagem de 8 s que avança: src/posplay.c:326-334. | Autoplay que não dispara não foi reproduzido. Sem plataforma e sem log. |
| #257 | FYI: incluído no Tizen Community Packages | Samsung | NÃO É BUG / FORA DO ESCOPO | Aviso de terceiro (Apps2Samsung). | Atenção: eles puxam os 4 .tpk e o .wgt do release 'sem alterar'. Manter nomes dos assets. |
| #258 | Legenda árabe não funciona | Samsung .tpk (S90F) | NÃO ATENDIDA | Mesma causa do #253 (sem fonte árabe fora da LG): src/text.c:1147-1152. | Sem log nem detalhes além das fotos. |
| #260 | Layout RTL opcional | Todas | NÃO ATENDIDA | grep RTL/rtl em src/*.c: só dlopen. Nada de layout RTL. | Feature grande; não entra sem decisão do dono. |
| #261 | Legenda árabe não funciona | LG webOS (impex) | PARCIAL | Mesmo conserto do #239 (bidi/forma). | Sem detalhes (arquivo, codepage). Pode ser Windows-1256 (#247). |

## Notas de método

- Cada linha RESOLVIDA NA 2.0 foi conferida lendo o trecho no HEAD, não só a mensagem do commit.
- A maioria dessas correções tem teste de host (Mac), não de TV. Onde o dono ainda não tem prova física, está dito na última coluna.
- Comentários do dono nas issues ("fixed in 1.x") foram tratados como pista, não como prova.
- Dados de relatores (links, logs) não foram copiados. Nenhum segredo lido.
