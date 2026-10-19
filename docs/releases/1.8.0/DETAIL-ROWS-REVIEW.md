# Review independente — ratings e cast/recommendations

Checkout integrado, somente leitura do renderer/root layout. Edicao autorizada
apenas em `tests/detail_layout.c` para regressao de rota TMDB, sem I/O.
Nao repetida suite GL do root.

Rereview FINAL: root corrigiu os tres itens abaixo; sem blocker residual
observado na leitura. Rota movie/tv PASS em
`/tmp/nuvio-180-validation/detail-layout-tmdb-route.log`. Recomendacoes agora
desenham cada c com x-scrollSec e range[c,c+1], bound EX_REL_MAX e
revRel[EX_REL_MAX]; culling de c0 nao impede demais posters. Altura de pointer
inclui poster+legenda e handler seleciona o mesmo c. Fixture root agora usa10
itens, coluna8 e scroll>0 nos dois tipos; GL final e do root, nao duplicado.

## Achados

1. P2 `src/detail.c:2195`: ao liberar SEC_RELACIONADOS na serie, OK num id
   `tmdb:N` passava `movie` para desc_pedir_titulo_tmdb. A mesma chave numerica
   pertence a namespaces distintos de filme/TV. Root ja corrigiu para
   `ehSerie()?"tv":"movie"`. Regressao adicionada ao fixture layout: stubs
   capturam id42+tipo movie/tv pelo KEYDOWN/KEYUP real do detalhe, sem rede.

2. P2 `src/detail.c:1722`, `:5103`, `:6070`: contagem de SEC_RELACIONADOS aceita
   ate12 itens (EX_REL_MAX), mas desenho itera somente os sete primeiros e
   ignora scrollSec. Quando foco vai ao item7+, nao existe poster/foco visivel;
   o culling do loop externo pode ainda pular c0 e impedir desenho de toda a
   fileira. Filme ja tinha essa fragilidade; a mudanca a introduz na serie,
   cujo antigo ramo controlava relFoco ate7. Solucao: renderer percorrer todos
   os itens com scrollSec e culling coerentes, ou limitar contagem (perde dados).
   Importante: revRel atualmente [8]; ampliar a EX_REL_MAX se desenhar todos.
   Fixture atual usa somente3 cards e nao prova este caso excedente.

3. Pointer (divida anterior do filme, nao regressao comprovada):
   `alturaAlvo(SEC_RELACIONADOS)==0` e desenhaRelacionados nao registra targets,
   portanto Magic Remote nao seleciona/clica estes posters. Ao adequar a nova
   fileira dedicada, registrar os mesmos rects reais do renderer e usar c
   correspondente; nao targets de avatar de elenco.

## Demais conclusoes

- `git diff v1.7.4 -- src/notasui.c` contem apenas notasui_media. Renderer,
  medir/altura e heatmap/restauracao sao precisamente os publicados1.7.4.
  API media respeita fontes normalizadas e exclui agregado via nf_resumo.
- Enum/foco e empilhamento colocam cast seguido das recomendacoes nos dois
  tipos; grupos posteriores usam conteudoSec e o documento inclui altura da
  nova fileira. RelNaLista nao acende poster enquanto cast recebe foco.
- Chegada de extras reconta secoes e recalcula stack por quadro; sem novo
  bloqueador observado nesse fluxo. Nenhuma promessa de estabilidade de
  scroll na chegada de rede real alem da leitura e fixtures disponiveis.
- Pointer do cast preserva seu handler de foco+OK/filmografia.
- Fixture NV_ROWS_ONLY original provava ida/volta D-pad para tres posters,
  movie+series. Root ampliou para10 e coluna8 no rereview; rota opaqueTMDB
  tem regressao separada PASS. Click fisico permanece limite de dispositivo.

Fonte renderer do veu permanece estavel. Evidencia e host/code; sem instalacao
ou prova fisica LG/Samsung.
