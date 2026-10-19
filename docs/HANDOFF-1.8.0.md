# Handoff — release 1.8.0 (Glass UI)

Para quem for montar a 1.8.0 a partir de `feat/glass-ilha`. Escrito em 03/10/2026.

## Estado das branches

| Ref | Onde | O que é |
|---|---|---|
| `feat/glass-ilha` | ver `git log -1` (worktree `/private/tmp/nv-glass`) | Glass UI inteiro. ~212 commits que o master não tem. Nada empurrado. |
| `origin/master` | `1345620b` | v1.7.3 e v1.7.4 publicadas. 56 commits que `feat/glass-ilha` não tem. |
| Base comum | — | O `feat/glass-ilha` saiu de `release/1.7.2` (merge `c7278fc7`). |

A 1.8.0 precisa das duas linhas: primeiro trazer o master para dentro do Glass UI, depois fechar a versão.

## Ainda rodando (não estão em `feat/glass-ilha`)

Juntar antes da release se ficarem prontos, ou deixar de fora e citar nas notas:

- `agente/codex-contexto` (`/private/tmp/nv-codex-ctx`): só documentação (`docs/glass-ui-contexto.md`).

## Guia de TV: parcial (já em `feat/glass-ilha`)

Cabeçalho (kicker, título, chips, Cartões/Lista segmentado) e a grade como ilha estão feitos. Faltam no Glass UI: o herói da esquerda, os painéis de categorias e de addons, a busca do guia, a barra de ajuda em teclas-pílula e o véu do fundo. Não há quadro de mockup para a página principal; foi extrapolado de `aovivo-guia` (player-mock) e do DESIGN.md.

## Passos

1. **Worktree limpa** a partir de `feat/glass-ilha`, ex.: `git worktree add -b release/1.8.0 /private/tmp/nv-180 feat/glass-ilha`. Nunca na pasta principal do repo: ela tem trabalho não commitado do dono (`fix/tpk-selo-hdr`).
2. **`git merge origin/master`**. Conflitos esperados:
   - `src/detail.c`: o master mexeu muito em detalhes (ações que expandem, badge de episódio, Apple TV, título acima do logo). O Glass UI reescreveu a página (`agente/detalhe-glass`). Manter o layout Glass UI e portar as correções de comportamento do master: runtime do episódio do addon/TMDB (`0f7254d5`), fallback de título traduzido (`6d74a0ac`, `2bf250fe`), elenco centralizado e filmografia visível (`e1a3f83d`), ações secundárias quando "Mais" recebe foco (`84a31d60`, `6bac65c0`) — essas em estilo Glass UI.
   - `src/home.c`: master tem trailers do hero depois de fileiras sem conteúdo (`09a70f44`), contadores do manifesto (`64ca4854`), espera de manifesto desativado (`ce13d98c`), destaque sem a Home animada por baixo (`1eee19b6`). O Glass UI mexeu em fade da Dinâmica, cartão do menu contextual e corte das fileiras na fila. Manter os dois.
   - `src/ajustes.c` e `ajustes_ux_*.inc`: master adicionou opções (Discord #222, Seekr, Onde assistir). **Enum `OpcaoId` só cresce no FIM**, e `valor[]`, `CHAVE[]`, `ESC(...)` e `ajustes_ux_padrao.inc` são posicionais: unir as entradas na mesma ordem nas quatro listas. Conferir com `tests/ajustes_secoes.sh` e `tests/ajustes_padroes.sh`.
   - `src/idioma_*.h`: unir as linhas. Depois reordenar `src/idioma_tab.h` pelos **bytes decodificados** (`\xNN` decodificado antes de comparar; a tabela é busca binária), rodar `python3 tools/idiomas.py --sincronizar` e preencher as 28 línguas. `tests/i18n.sh` tem de dar "tudo ok".
   - `src/text.h` / `src/text.c`: estilos `TXT_*` novos só no FIM do enum, com `ESTILOS[]` na mesma ordem.
   - Discord (`1ea1510b`, `f515a5cc`, `78a5f222`): telas de Ajustes do Discord devem usar o material da ilha.
   - `src/novidades*.c`: o master tem o cartão da 1.7.4 ("animated highlights card", `872ca223`). Na 1.8.0 a regra é: **instalação nova vê só o cartão da 1.8.0** (`novidadesfila.c` grava os marcadores antigos). Conferir que o cartão da 1.7.3/1.7.4 entra na lista de marcadores antigos.
3. **Compilar e rodar só o que importa**: `tests/i18n.sh`, `tests/ajustes_secoes.sh`, `tests/ajustes_padroes.sh`, `tests/novidades_fila.sh`, `tests/acentos.sh`, e uma compilação completa por um harness (`bash tests/fontepref_shot.sh /tmp/x`). O dono pediu para não rodar a suíte inteira.
4. **Versão 1.8.0** nos dois arquivos: `deploy/app/appinfo.json` (`"version"`) e `tools/tizen-config.xml` (`version=`). Commit só com isso, mensagem `v1.8.0`. `tools/env.sh` aborta se discordarem.
5. **Pacotes** (receita completa em `docs/HANDOFF-1.7.1.md` e na memória do projeto):
   - Sempre `export NUVIO_PROPERTIES="/Users/hrocha/Projetos/Pessoal/LG WEB/NuvioWeb-0.3.38-beta/local.properties"` antes de qualquer build em worktree, e `bash tools/env.sh --require-core >/dev/null` (nunca imprimir a saída). Sem isso o pacote sai sem login, addons e log.
   - LG: `bash tools/arm.sh --ipk` (normal) e depois o alto-cache. O `arm.sh` apaga `./*.ipk` no início: copiar o normal antes. Conferir o modo do binário: `tar tvzf data.tar.gz | grep nuvio-proto` tem de ser `-rwxr-xr-x` (a 1.7.1 saiu 705 e não abria).
   - Samsung: `tools/release-samsung.sh` (.wgt, .wgt Tizen 4, .tpk 6+, .tpk 4/5). Worktree nova precisa de `build/ass-wasm` (symlink para `~/.cache/nuvio-ass-wasm`).
   - Android: `bash tools/release-android.sh` (chave `~/.nuvio-android/release.jks`; nunca gerar outra).
   - **Credenciais fora dos pacotes**: `ar p <ipk> data.tar.gz | tar tz | grep 'art/<arquivo>$'` para trakt.txt, addons.txt, tmdb.txt, mdblist.txt, sessao.txt e collections.json, e `unzip -l <wgt> | grep -E '\.txt|collections'`. Tudo 0.
6. **Notas** em inglês, `## Added` / `## Fixed`, balas curtas (o cartão do app mostra 3 linhas por bala e corta `## Notes`). Destaques da 1.8.0 abaixo.
7. **Publicar só com ok do dono**: `git push`, tag `v1.8.0`, `gh release create` com todos os anexos (2 .ipk, 4 .tpk, .wgt, .apk, libnuvio .so, `repo.json` + `webosbrew.manifest.json` gerados por `tools/hb-repo.sh`, SHA256SUMS de LG+Samsung+Android). Baixar de volta e conferir os sha256. Release sem `repo.json` quebra o Homebrew de todo mundo.

## O que entra na 1.8.0 (para as notas)

- Glass UI: todo painel vira uma ilha no material da ilha do relógio, em vidro ou sólido (Aparência). Opacidade do vidro e Vidro fosco (avançados).
- Ajustes v2: miniaturas com arte real em cada categoria, toggles animados, barras que crescem na horizontal; menu de categorias grande, a categoria em foco abre em duas linhas, lista desliza por cima do menu; Fundo (Arte / Arte borrada / Frost); prévias e gráficos reais (memória, diagnóstico, teste de velocidade); Guia de uso com 87 recursos.
- Tamanho da interface (100/120/130/150%); Spotlight mín. 130%, Agenda e topo da Biblioteca mín. 120%, menu lateral fixo em 90%.
- Menu lateral: Moderna como trilho flutuante que abre em painel; Padrão como painel na borda, centralizado.
- 18 cores de destaque (6 novas), texto escuro nas cores claras, Da arte / Gradiente / Imersiva, Textura (recorte do logo do título).
- Folha de Fontes nova: Melhor para esta TV primeiro, grupos por resolução e HDR/SDR, filtros Só MP4 / Em cache / Dublado, Logo do título, Selos coloridos, pacotes de selos personalizados (da conta ou por URL), resolução reconhecida nos canais ao vivo.
- Player: componentes nascem da ilha (áudio, legendas, episódios, erro, carregando), abertura toda preta só com a ilha, Seekr como ilha, "Ao vivo" traduzido.
- Ilha do relógio: confirmações com Desfazer, capa voando ao salvar, pergunta de onde salvar na primeira vez, cartão de Atualização em 12 estados.
- Detalhes do filme e da série, Agenda, Busca (com Pessoas), Biblioteca (setas laterais, menu contextual em pôsteres e listas com resumo), sidebar Social maior, registro do app na tela com código de 6 letras (servidor já publicado), teclado medido pelo texto.
- Home: canais ficam no guia; fileira na fila não aparece; Dinâmica mostra um pedaço da fileira de cima.

## Sem prova em TV

Quase tudo acima foi verificado só por captura no Mac. A TCL (Android) recebeu builds locais até `04cbd3f3` (1.7.6 local). Nada foi instalado na LG nem na Samsung. Conferir pelo menos: abertura do player toda preta, Textura e Vidro fosco no desempenho da C9, menu lateral novo, folha de Fontes na LG (6 botões).

## Pegadinhas

- Agentes paralelos usam worktrees em `/private/tmp/nv-*`. Adicionar arquivos **pelo nome**, nunca `git add -A`.
- Disco do Mac anda com 2–4 GB livres: apagar binários e capturas temporárias.
- O servidor de recomendações/registro já está com o código de 6 letras (`7cbb25e6`); não precisa novo deploy para a 1.8.0.
