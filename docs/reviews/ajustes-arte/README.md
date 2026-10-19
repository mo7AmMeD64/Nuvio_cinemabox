# Ajustes: arte sem repetição e controles horizontais

Capturas nativas 1920×1080 no Mac, com fixtures locais sem conta. Categorias capturadas: todas as 11. Amostras preservadas: Idiomas, TV ao vivo e editor numérico com Glass. Não representam validação em TV física.

- [Idiomas](idiomas.png)
- [TV ao vivo](tv-ao-vivo.png)
- [Barra horizontal](barra-horizontal.png)
- [Mapa completo de 196 opções](../../ajustes-mapa-visual.md)

## Checks executados

- Build nativo da fixture `tests/ajustes_shot.c`, com todos os módulos do app exceto `main.c`.
- `tests/ajustes_ux_interacao.c`: passou. Inclui edição horizontal, limites, cancelamento, restauração, persistência, inversão da animação, movimento reduzido e cobertura do mapa de ícones.
- `tests/ajustes_ux_dados.c`: passou (busca, padrões, escopo, dependências e segredos).
- Todos os ícones do novo mapa têm SVG Lucide local e PNG embarcado.
- Inspeção de capturas em superfícies sólidas e Glass; `git diff --check` sem erros.

Binários de teste foram compilados diretamente; nenhum script de build/publicação ou medição longa foi executado. O teste de dados usa seu diretório temporário padrão; os demais binários, fixtures e capturas foram produzidos no worktree.

Pendentes: TV/controle físico, comportamento e fluidez nos alvos LG/Samsung/Android, todas as escalas e traduções. DESIGN.md ausente na base; foram usadas as regras presentes em `.impeccable/design.json` e os componentes Glass existentes.

## Arquivos de implementação

- `src/ajustes.c`
- `src/ajustes_ux_desenho.inc`
- `src/ajustes_ux_ilha.inc`
- `src/ajustes_ux_interacao.inc`
- `src/ajustes_ux_previa.inc`
- `src/ajustes_ux_v2.inc`
- `src/ajustes_ux_visual.inc`
- `tests/ajustes_ux_interacao.c`

Documentação: mapa visual, este registro e as três capturas.
