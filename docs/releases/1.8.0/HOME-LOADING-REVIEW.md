# Home progressiva — review independente

Revisao read-only de `src/catalogo.c/h:cat_home_apenas_fixas`, decisao em
`src/descoberta.c:montar` e novo fixture `tests/home_progressiva.c/sh`.
Sem edicoes ou duplicacao da suite executada pelo implementador.

Veredito: sem blocker novo observado nos hunks. A decisao antiga cat_n()==0
nao reconhecia a Home inicial quando CW/social chegavam antes do primeiro
catalogo; getter novo reconhece esse estado com leitura coerente de itens e
fileiras. O log de reproducao antes tinha ready=1ms, visible=354ms esperando
o catalogo lento. Resultado depois deve vir dos testes do implementador.

- Mutex pubTrava cobre snapshot conjunto n/itens/fils/nFils, evitando observar
  n==0 ou janelas intermediarias durante publicacao. Getter nao chama APIs que
  peguem outras travas e nao retém ponteiros depois de soltar a trava.
- Apenas chaves reservadas continue_watching/social_activity permitem cold.
  Qualquer catalogo/colecao ja publicado e warm, inclusive fileira com estado
  mas sem itens. Marcadores naLista/naColecao e itens nao atribuidos a janela
  tambem bloqueiam, preservando listas/metadata prontas antes da classificacao.
- Warm sai durante scan curto de fileiras; cold tem no maximo duas fixas e
  poucos itens. Nao introduz scan pesado por quadro; chamado uma vez por ciclo.
- Estado de lista preparado dentro do proprio ciclo permanece anexado pelos
  helpers de publicacao existentes. FonteIntacta/geracao guardam publicacoes
  por catalogo e descarte/reinicio final como antes. Hunk nao relaxa identidade.
- Tests usam implementacao real descoberta/catalogo/homeestado com rede dublada
  e atraso controlado. Cases0-3 separam coldfixas, warmcatalogo, readywatchlist
  sem janela, collectionkey; case4 troca dono+geracao durante rede.

Limites relevantes: case4 usa mesmos ids nos dois donos e perfil fixo1; prova
cancel/restart, nao dados distintos entre perfis/contas reais. Getter decide
uma vez no inicio: dados mesclados posteriormente seguem protocolo existente,
nao uma transacao de classificacao+publicacao. Override parcialNaTela e
publicacao inicialCW sao caminhos herdados sem alteracao de guard pelo hunk;
nao e novo teste abrangente de races de todos os publicadores. Nao rodado TSan,
provider externo ou TV nesta revisao. Logs PASS do implementador e copia
integrada do root sao a evidencia dinamica complementar.
