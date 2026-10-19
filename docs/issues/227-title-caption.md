# Localized title beside artwork (#227)

The detail page always shows the current catalog's textual title above the logo. The catalog's existing localization updates remain visible after opening; artwork keeps its independent session identity. No OCR, locale guessing, or additional metadata request is introduced.

The complete title wraps without a line limit. Its measured height is reserved above the logo, which shrinks when needed to keep the caption above the agenda, resume rows and action buttons. The logo is skipped if no drawable height remains. Titles without a logo use the same textual block.

Validation: `tests/detail_remonta.sh`, `tests/heroidentidade.sh`, and `NUVIO_SHOT_TITULO=1 bash tests/detail_eps_shot.sh <output-prefix>`. The last command uses production rendering with an offline synthetic catalog, verifies live title replacement, unrestricted wrapping and viewport bounds, and captures loaded-logo and no-logo variants. Different names and logo artwork in those fixtures intentionally exercise the localization mismatch. It is a Mac GL fixture, not physical-TV validation. The loading path uses the same unconditional caption branch but is not independently timed in the fixture; combined agenda/resume layout is anchored to the existing shared `yEstado` calculation.


## Ajuste após teste na Android TV

O usuário confirmou o Discord funcionando e pediu título textual somente como fallback de idioma. O detalhe oculta o nome quando o logo efetivamente mostrado tem idioma TMDB confirmado igual ao idioma configurado; arte sem idioma conhecido, estrangeira, ausente ou ainda carregando mantém o nome legível. A evidência de idioma acompanha a URL exata, preservada no catálogo, para não atribuir o idioma de um logo novo ao snapshot anterior. O cache antigo do catálogo será refeito por mudança no tamanho de CatItem.

Em filmes, direção passa para a linha técnica ao lado do país e o espaço anterior desaparece, aproximando os botões da sinopse. Na Dinâmica (Apple TV), expandir o card para tela cheia agora leva texto e logo para a margem da página já no primeiro passo; a segunda seta continua abrindo o corpo rolável. A volta restaura a posição dentro do card.


### Correção após segunda verificação na TV

Idioma desconhecido não significa estrangeiro: logo carregado sem idioma conhecido não recebe título duplicado. Durante o carregamento não se mostra título provisório que desapareceria depois; ausência ou falha real da imagem permite o nome textual. A linha de direção/roteiro foi movida também nas séries. Em tela cheia do layout Apple TV, as pontas da linha de botões não mudam de título nem fecham a página pela esquerda; Voltar retorna ao carrossel, onde as setas podem trocar o título novamente. Teste dedicado cobre idioma conhecido/desconhecido, URL diferente, imagem carregando/ausente e navegação nos dois estados.
