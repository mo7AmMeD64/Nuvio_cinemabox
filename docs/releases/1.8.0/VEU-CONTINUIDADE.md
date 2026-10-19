# Continuidade do veu Apple — host GL

Fontes renderer estaveis no checkout de integracao. Ownership restrito a
`src/detail.c:carFundo`, `src/gfx.c:GFX_JANELA` e seu guard de blend; fixtures
`tests/carrossel_shot.c/sh` e `tests/carrossel_continuidade.py`. Nenhum stage,
commit, push ou instalacao. Layout/texto/ratings/fileiras do root preservados.

O salto era a troca em `cartao=.002`: GFX_JANELA misturava o veu no fundo
solido, mas GFX_DETALHE revelava ambiente/Frost/arte borrada por alfa. Agora
a folha converge para o chao da pagina, a base selecionada entra durante a
expansao e a janela converge para a mesma composicao do detalhe. O shader
tambem alinha apagar por rolagem e o veu sozinho do trailer (incluindo cinema).
Placeholder sem textura e poster contido convergem para os mesmos endpoints.

Nao ha crossfade permanente de duas paginas nem captura fullscreen nova.
Ambiente reutiliza o assado ja existente. A pagina assentada usa o renderer
anterior: fundo-only Imersiva 1 rect, Frost 6, arte borrada 3; GFX_JANELA sai
ao terminar a expansao. Poster pode usar duas artes apenas durante sua
transicao de cover para encaixe contido. Expansao de Frost/blur desenha sua
base real sob a arte; custo transitorio deve ser validado na TV.

## Provas

Fixture captura somente fundo em `cartao=1,.5,.1,.02,.0021,.0019,0`, com
GL real SDL/OpenGL no Mac e artes locais, sem provider externo. Compara o
interior (8 px de margem) em RGB de 8 bits, tolerando grao/geometria subpixel.
`tests/carrossel_continuidade.py` usa somente Python stdlib e exige mean<=1,
p95<=3. Todos os casos finais abaixo PASS:

| Caso | Mean antes /255 | p95 antes | Mean depois /255 | p95 depois | Max depois |
|---|---:|---:|---:|---:|---:|
| Imersiva | 7.682 | 29 | .217 | 1 | 2 |
| Frost | 9.187 | 45 | .191 | 1 | 2 |
| Arte borrada | 4.117 | 16 | .219 | 1 | 3 |
| Arte | .092 | 1 | .092 | 1 | 1 |
| Poster contido | — | — | .274 | 1 | 22 (arestas) |
| Frost rolado pg=.5 | — | — | .130 | 1 | 2 |
| Frost sem arte | — | — | .033 | 0 | 1 |

Capturas/pares BMP+PNG: `/tmp/nuvio-car-bg-before-{immersive,frost,blur}` e
`/tmp/nuvio-car-bg-final-{immersive,frost,blur,plain,poster,scroll}`.
`c-limiar-4` e `c-limiar-5` sao os dois lados do limiar. Sequencia inteira
`c-limiar-0` a `c-limiar-6`. O fixture imprime branch/fill/rects por quadro.

Animacoes reduzidas: caminho normal completo (abrir, trocar titulo, abrir
pagina, voltar, fechar) PASS e termina `detalhe aberto no fim: 0` em
`/tmp/nuvio-180-validation/car-bg-final-reduced.log`.
`git diff --check` PASS. Logs finais dos modos em
`/tmp/nuvio-180-validation/car-bg-final-*.log`.

Ausencia de arte: replay corrigido PASS em
`/tmp/nuvio-car-bg-final-missing-corrected`. Primeiro ensaio era invalido:
o fixture apagava IMDb mas mantinha idxImdb e o reconciliador reencontrava
uma copia com arte no catalogo. Corrigido somente fixture; renderer nao mudou.

Limites: prova host, sem GPU-ms de TV, sem instalacao, sem video nativo de
trailer. A composicao trailer foi alinhada por codigo; playback/device ainda
depende da prova na plataforma. Root revisou endpoint e guard de blend sem
achado bloqueador e roda sua captura integrada independente.
