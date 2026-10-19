# Worker/D1 — revisão dos logs de clientes 1.7.3

03/10/2026. Consulta de diagnóstico previamente autorizada, somente leitura: dump privado de 500 registros, IDs 21252–21751; consulta confirmou rows_written=0/changes=0. Texto privado permanece fora do repositório. Este relatório contém somente IDs, contagens e eventos técnicos sanitizados. Não foi alterado runtime, servidor ou deploy.

## Identidade de versão antes do veredito

| Registro | Versão do uploader | Versão embutida/escopo | Conclusão |
|---|---|---|---|
|21737|Android 1.7.3|`[tv]` identifica app 1.7.3; inclui evidência da sessão anterior 1.7.2|É evidência de execução 1.7.3, sem atribuir saída anterior ao binário atual |
|21732|Android 1.7.3|Cabeçalho de execução 1.7.2; quatro tokens 1.7.2 no corpo|Histórico enviado após atualização; não conta como playback/fluidez 1.7.3 |
|21745|Android 1.7.3|Relatório de diagnóstico com `versao=1.7.3`, sem janela de FPS/playback|Comprova emissão desse relatório; não valida reprodução |

A saída anterior em 21737 menciona instalação de pacote (`installPackageLI`, status 0), não sinal fatal nem OOM. Isso não prova crash da 1.7.3. A amostra tem somente três uploads etiquetados 1.7.3 e nenhum deles fornece validação LG/Samsung 1.7.3.

## Achados mais fortes

### 1. Jank de quase 1 s durante atualização do app — prioridade de reprodução

21737 registra pior quadro 995,6 ms: `upd=910,1 ms`, `des=77,5 ms`, `swap=2,1 ms`, `bomb=0,0 ms`, RSS 419 MB. A demora dominante é atualização, não upload de textura nem espera de swap. O contexto próximo contém eventos de vídeo, Onde ver e fonte. É observação real de um stall, mas não stack trace suficiente para apontar a função responsável.

Suspeitos concretos para instrumentação: `main.c:1183` envolve `app_atualizar`; `streams.c:1258` atualiza apps ao abrir fontes; `ondever_apps_atualizar` chama o adaptador Android; `NuvioActivity.listarApps` faz `PackageManager.queryIntentActivities` e montagem da lista. Essa enumeração síncrona merece duração própria/cache, mas NÃO foi provado que consumiu os 910 ms. Também medir aplicação de sync/remontagem/fileiras no mesmo update, antes de escolher correção.

Reprodução: mesma Android TV/build, abrir fontes frias e quentes, alternar detalhe/voltar/home e captura dos tempos `apps-enumeration`, `streams-open`, `sync-apply`, `home-rebuild`. Critério: confirmar qual chamada domina o update e remover bloqueio de UI sem publicar cache errado após instalar/remover apps. Não aumentar orçamento de memória como tratamento desse evento.

### 2. Fluidez depende do modo de desenho — não há ganho global demonstrado

21737 apresenta sequência estável próxima de 60 fps depois de carregar Home e depois queda em configuração de vidro/layout; registro de modos mostra `fps=43,3`, `layout=2`, vidro ligado. O conjunto de 27 amostras tem mediana 59,3 fps e faixa 43,4–60,3, mas mistura telas e modos: não é benchmark comparativo. Há ainda quadro 132,6 ms dominado por desenho 119,8 ms. Medir tela/modo fixo e custo de preenchimento, blur e texto; não interpretar mediana da sessão como aprovação universal.

21732 contém 306 amostras, mediana 46,75 fps, RSS 211–593 MB, mas executava 1.7.2 e alternava vidro/modos. Esse histórico serve para planejar comparação controlada, não para dizer que 1.7.3 regrediu ou melhorou. RSS crescente sozinho não prova vazamento; decoder, imagens e cache têm alocações legítimas e precisam de ciclo repetido com plateau/retorno.

Suspeitos: `gfx`/modos de vidro e trabalho de desenho em `main.c` instrumentado por `[gpu-modos]` e `[quadro]`. Preservar budgets até A/B medido na mesma tela. O fix Android de layout repetido foi validado em JVM, mas esta amostra não identifica o commit/hash instalado e não demonstra seu ganho físico.

### 3. Falhas de trailer/fontes existem na 1.7.2, sem causa suficiente para bloquear 1.7.3

21732 inclui evento Android erro 2000 e cancelamento/erro de trailer. O código `NvPlayer.onPlayerError` preserva errorCodeName/code/message e envia evento 5; erro numérico sozinho não identifica URL expirada, timeout, codec ou decoder. Investigar com recurso legal controlado, status de resolução, HTTP e código nominal, mantendo URLs privadas fora dos logs compartilháveis.

Em registros atuais 1.7.2,14 uploads TPK incluem erros numéricos `0xfe6c002e`, `0xfe6c0026` ou `0xffffffff`. Os mesmos dumps podem repetir sessões anteriores:59 ocorrências textuais não significam 59 falhas independentes. Exemplos recentes:21739,21722,21704. Não atribuir esses códigos a codec/rede sem contrato do host; novo `video_tpk.c` dá evidência numérica à tela de erro, mas ainda requer fonte/aparelho para comprovar solução.

102 registros etiquetados 1.7.2/1.7.3 contêm timeout curl 28 (545 ocorrências no texto agregado). Isso mistura add-ons, imagens e outras consultas; não prova falha geral do player nem do Worker. Verificar subsistema e cancelar consultas após troca de geração antes de mexer em deadlines. `trailer.c`/resolução e `NvPlayer.kt` são pontos de reprodução; rede lenta de fornecedor não autoriza alterar cache global.

## Limites e gates

- Busca do dump não encontrou `SIGSEGV`, `SIGABRT`, `Fatal signal`, `ENOSPC` ou `EDQUOT` nos registros etiquetados 1.7.2/1.7.3. Ausência no texto armazenado não comprova ausência de crash ou armazenamento cheio; dumps podem ser truncados (muitos têm 204800 caracteres) e conter somente cauda.
- Sem validação física LG/Samsung 1.7.3 nesta amostra, sem hash confiável por upload e sem pares baseline/fix controlados. Publicação não deve transformar upload de diagnóstico em prova de todas as features.
- Live tail de exceções do Worker não foi validado: tentativa de 35 s sem saída nem marcador de conexão. Não afirmar que Worker não tem exceções. D1 é log do cliente armazenado, não trace de execução Worker.
- Questão #223/attachment de 16 linhas foi analisada separadamente: retorno de suporte 4 K não deve ser convertido em prova de abort em 2 s. Ver relatório de issue correspondente.

Próximos passos prioritários: reproduzir update 910 ms com spans; A/B vidro/tela fixa por aparelho; reproduzir trailer 2000/TPK códigos com fonte controlada e detalhes redigidos. Cada resultado deve distinguir build instalada, sessão anterior, UI/teste host e prova física.
