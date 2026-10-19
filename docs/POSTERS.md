# Pôsteres personalizados

Troca os cartazes retrato do app (home, biblioteca, busca, "Ver tudo" e "Mais
como este") por cartazes **já prontos** de um serviço externo: arte sem texto,
logo, notas, selos 4K/HDR/Dolby Vision e faixa Top 10, tudo desenhado no
próprio cartaz. **Desligado por padrão.**

Ajustes > Layout > **Pôsteres personalizados**.

Só cartaz retrato (2:3) de card. O destaque da home, o fundo da página do
título e os cards deitados **não mudam**: nenhum dos serviços abaixo entrega
16:9.

## Provedores

| Opção | O que é | Precisa de |
|---|---|---|
| **SpatialPosters** | Servidor de código aberto ([TheAceOfficials/SpatialPosters](https://github.com/TheAceOfficials/SpatialPosters), AGPL-3.0) que monta o cartaz na hora. Instância pública ou a sua. | Nada (instância pública) ou um servidor seu |
| **RPDB** | [RatingPosterDB](https://ratingposterdb.com), serviço pago com chave gratuita de teste limitada. | Chave da sua conta |
| **Modelo próprio** | Qualquer endereço com `{imdb}`, `{tmdb}`, `{type}`, `{tipo_tmdb}`. | Um servidor que responda imagem |

Se o serviço não responde (erro HTTP, prazo, imagem quebrada), o card volta ao
cartaz normal e **não tenta de novo naquela sessão**. Depois de 6 falhas
seguidas o serviço fica em pausa por 5 minutos, para uma instância fora do ar
não custar 20 segundos de espera por card.

## SpatialPosters

### Instância pública

1. Ajustes > Layout > Pôsteres personalizados > escolha **SpatialPosters**.
2. Deixe o endereço vazio: usa `https://spatial-posters.vercel.app`.
3. **Testar pôsteres** baixa o cartaz de *Um Sonho de Liberdade* e diz se deu
   certo.

A pública é gratuita e **compartilhada** (ver "Limites"). Para uso diário com
muitos cartazes, prefira a sua.

### Instância própria (Docker)

```bash
docker run -d --name spatialposters -p 3000:3000 \
  -e SPATIALPOSTERS_TMDB_KEY="sua_chave_v3_do_tmdb" \
  -e SPATIALPOSTERS_PUBLIC_INSTANCE=1 \
  -v spatialposters_data:/data --restart unless-stopped \
  theaceofficials/spatialposters:latest
```

No app: **Endereço do SpatialPosters** = `192.168.1.5:3000` (IP e porta do
servidor; sem `https://` vira `http://` para IP e nome `.local`). O visual
padrão (estilo dos selos, gradiente, faixa) é o que você escolher no estúdio da
própria instância — não precisa de token.

### Escolher o estilo: parâmetros curtos (recomendado)

**Parâmetros do SpatialPosters** aceita a parte curta da URL, por exemplo:

```
bs=vetro&side=right
```

Os mais úteis (nomes do próprio SpatialPosters): `bs` estilo do selo
(`shadow`, `pill`, `bar`, `colored`, `bordo`, `vetro`), `rs` estilo da faixa
Top 10 (`default`, `bar`, `colored`, `pill`, `netflix`), `side` (`left` ou
`right`), `badges=0` (sem selos), `ranking=0` (sem faixa). O idioma da
interface vai sozinho (`lang=`). `fmt`, `format`, `config` e `c` são recusados:
o formato é sempre JPEG e o token tem campo próprio.

### Token de configuração e o manifest do addon

O SpatialPosters também gera um **token** com todo o seu estilo (`/c/<token>/…`).
Dá para colar no campo **Token do SpatialPosters** o token ou o endereço do
manifest inteiro (`https://…/c/<token>/manifest.json`): o app extrai o token e,
se vier com host, também a instância.

**Limite honesto:** um token real tem mais de 500 caracteres, e o app aceita no
máximo 400 (a URL do cartaz precisa caber em 512 bytes no cache de imagens; o
teclado da TV também não ajuda). Na prática o token só serve para
configurações mínimas. Use os **parâmetros curtos** ou os **padrões da sua
instância**.

### Duas maneiras de usar o SpatialPosters

**A) Endpoint de imagem (o que esta tela faz).** O app monta a URL
`<instância>/api/poster/{movie|series}/<tt…>?fmt=jpeg&lang=<idioma>[&extras][&config=<token>]`
para o IMDb de cada título. O título continua sendo o do seu catálogo normal:
streams, Trakt, legendas e progresso seguem iguais. É o modo recomendado.

**B) Addon Stremio.** Instalar `https://<domínio>/c/<token>/manifest.json` na
conta Nuvio já funciona **sem esta tela**: os catálogos dele trazem o campo
`poster` com o cartaz pronto e o app o desenha como vem (o destaque e os fundos
continuam vindo da "Fonte da arte"). Mas os itens desse addon têm id
`tmdb:<número>`, não IMDb, e por isso addons de fontes/legendas que só falam
IMDb não acham o título. Serve para descobrir títulos, não para substituir o
seu catálogo. Os dois modos combinam: com o addon instalado e esta tela
ligada, os cartazes dos seus outros catálogos também ganham o visual.

## RPDB

1. Pegue a chave em ratingposterdb.com.
2. Escolha **RPDB** e preencha **Chave do RPDB**.

URL usada: `https://api.ratingposterdb.com/<chave>/imdb/poster-default/<tt…>.jpg?fallback=true`
(com `fallback=true` o RPDB devolve o cartaz do TMDB quando ele não tem o
seu). Título sem IMDb usa `…/tmdb/poster-default/{movie|series}-<id>.jpg`.
O arquivo é um JPEG progressivo de ~145 KB (580x859), bem maior que o do
SpatialPosters (~48 KB, 500x750).

## Modelo próprio

**Modelo de URL dos pôsteres** = um endereço `http(s)://…` com pelo menos um
destes marcadores:

| Marcador | Vale |
|---|---|
| `{imdb}` | `tt0111161` (título sem IMDb: fica com o cartaz normal) |
| `{tmdb}` | id numérico do TMDB (se ainda não se sabe: cartaz normal) |
| `{type}` | `movie` ou `series` |
| `{tipo_tmdb}` | `movie` ou `tv` |

Exemplo: `https://meu.servidor/{type}/{imdb}.jpg`. Máximo 300 caracteres; a URL
final tem de caber em 512 bytes.

## O que é pedido, e quando

- Só filme e série com id de IMDb (`tt…`) ou de TMDB (`tmdb:…`). Canais,
  TV ao vivo e itens de outros addons ficam com o cartaz normal.
- A URL é **estável** por título (não muda quando o app descobre o id do TMDB
  depois): o cartaz vai para o mesmo cache de imagens de todo o resto, em
  disco, e não é baixado de novo na próxima abertura.
- No máximo **3 downloads ao mesmo tempo** do provedor, mesmo com 4 fios de rede
  livres, e prazo de 20 s por cartaz (o primeiro cartaz de um título é montado
  no servidor: ~3 s medidos na instância pública). Uma home de 200 cards vai
  enchendo aos poucos; o card mostra o esqueleto cinza até o cartaz chegar (ou
  o cartaz normal, se o serviço falhar).
- Formato: **sempre JPEG** (`fmt=jpeg`). É o que decodifica direto em todos os
  alvos (LG com libjpeg; Samsung no navegador) e, nos testes, o WebP do
  SpatialPosters tinha o mesmo tamanho.
- O token e a chave **nunca vão para o log**: a URL só aparece como
  `https://host/...`. Ficam em `posteres.txt` na pasta de dados desta TV, não
  sincronizam com a conta.

## Limites e custo

- **Instância pública do SpatialPosters (Vercel):** compartilhada, sem
  garantia. O servidor limita cada IP a um balde de 200 pedidos com
  reposição de 20 por segundo (rota de cartaz) e responde `429`/`503` quando
  passa disso; o app trata como falha do item. Cartazes frios são renderizados
  na função da Vercel e podem demorar. O cartaz pronto fica em cache do CDN por
  6 h (`max-age=21600`).
- **Custo para você:** o app não cobra nada. Instância pública: grátis. Sua
  própria: o servidor/NAS e uma chave gratuita do TMDB. RPDB: o plano dele.
- **Dados enviados:** o servidor escolhido vê o IP da TV, o id do título e, se
  houver, o token ou a chave (por isso "instância própria" é o caminho mais
  privado).
- **Licença:** o SpatialPosters é AGPL-3.0. Este app **só chama os endpoints
  HTTP dele**; não contém nem copia código dele. Quem hospeda a própria
  instância segue as condições da AGPL para o código do servidor.
- **Memória e disco:** cartaz de 500x750 decodificado no tamanho do card, no
  mesmo orçamento de texturas de sempre.

## Problemas comuns

| A linha "Testar pôsteres" diz | Significa |
|---|---|
| `funcionou · 47 KB em 900 ms` | Tudo certo. |
| `sem resposta do serviço` | Endereço errado, servidor fora do ar, sem internet ou o prazo de 20 s estourou. |
| `respondeu, mas não é uma imagem` | O endereço responde, mas não é o endpoint de cartaz (proxy, página de erro). |
| `configuração incompleta ou grande demais` | Falta a chave do RPDB / o modelo, ou a URL passou de 512 bytes (token longo). |
| `token inválido …` | O texto colado tem caracteres fora de letras, números e `_ . ~ = -` ou passa de 400. |
