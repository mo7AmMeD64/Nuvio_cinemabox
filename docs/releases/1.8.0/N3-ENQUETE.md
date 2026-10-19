# N3 — Enquete na ilha do relogio (Nuvio 2.0)

Mockup aprovado: `mockups-20/04-enquete-ilha.html`. Feito so com o que a ilha ja
faz (pilula, aviso com a tecla azul, modal que cresce da pilula, `ilha_acao` com
Desfazer, `ilha_aviso_pediu`). Nada foi publicado: **nenhum deploy, nenhuma
migracao aplicada, nenhuma enquete semeada em producao.**

## Cliente

| Arquivo | O que faz |
|---|---|
| `src/enquete.c/.h` | Estado, fio de rede (fila de 4, um fio), convite uma vez, bolinha, voto, resultado, opt-out e espelho. |
| `src/ilha.c/.h` | `IlhaModal.resultado/pct/escolha` (linha de resultado com trilho de 6 px), `ilha_ponto_enquete` (bolinha de acento no relogio) e `ilha_ponto_pediu` (AZUL no relogio parado). Titulo de modal que nao cabe quebra em 2 linhas. |
| `src/ilhasinais.c` | Entrega os botoes das chaves `enquete:*` e chama `enquete_passo` por quadro. |
| `src/ajustes.c` + `ajustes_ux_*.inc` | `AJ_ENQUETES` ("Receber enquetes", ligado por padrao) no FIM do enum e dos vetores paralelos. Fica em **Contas e servicos > Notificacoes** (nao existia uma secao Notificacoes). |
| `src/app.c` | `enquete_perfil_trocado()` na troca de perfil. |
| `src/idioma_*.h` | 18 chaves novas, 28 idiomas (`tools/idiomas.py` passa). |

Fluxo: o primeiro `GET /v1/enquete` sai 15 s depois de o app assentar (fio proprio,
nunca bloqueia a abertura; repete a cada 6 h e a cada troca de perfil). Enquete nova
e valida: aviso curto "Tem uma enquete para voce" que abre sozinho no modal-convite
(Responder / Agora nao / Nao receber mais enquetes). O convite sai **uma vez por
enquete e perfil** (`enquete.txt`). "Agora nao" ou Voltar deixam a bolinha no
relogio; a AZUL no relogio parado reabre direto nas opcoes (com a bolinha ligada,
a AZUL no relogio parado nao abre a central — ela volta quando a enquete e
respondida, vence ou o opt-out liga). Opcoes = botoes (ate 3), arte 480x270 do
servidor, prazo na base. Voto unico; depois o modal vira resultado (texto,
porcentagem, trilho, total de votos). Ja votou em outra TV: sem convite nem
bolinha. Prazo vencido: some.

Opt-out: o botao do convite e a opcao de Ajustes levam a escolha para a **conta**
(`POST /v1/enquete/optout`) e para o espelho local (`enquetesLocal`). Pela ilha ha
Desfazer (`ilhaacao`). Se o toque foi feito sem rede, `enquete-optout.txt` guarda a
pendencia e ela vence o espelho ate o servidor confirmar; sem pendencia, o servidor
vence (outra TV mudou).

Troca de perfil: sobe uma geracao; pedidos em voo sao descartados, fila e caixa
zeradas, convite e bolinha somem e o perfil novo busca de novo.

## Servidor (`servidor/recomendacoes`)

- `migracao-009-enquete.sql` — `enquete`, `enquete_opcao`, `enquete_voto`
  (chave primaria `enquete, pessoa`: um voto por pessoa), `enquete_optout`. So
  aditiva, idempotente, **nao semeia nada**.
- `src/enquete.js`, ligado em `src/index.js` depois da autenticacao (mesma de todas
  as rotas Social, perfil por `X-Nuvio-Perfil`):
  - `GET /v1/enquete` -> `{optout, enquete}`; `enquete` e a ativa
    (`ativa=1 AND inicio<=agora<fim`) com `id, pergunta, arte, fim, opcoes[], voto`.
    So quem ja votou recebe `resultado[]` e `total` (contagens agregadas; nunca ids).
  - `POST /v1/enquete/voto` `{id, opcao}` -> 200 (idempotente na mesma opcao),
    409 outra opcao, 410 encerrada/futura, 404 desconhecida, 400 invalida.
  - `POST /v1/enquete/optout` `{optout:1|0}` -> `{optout}`.
- Testes: `node --test servidor/recomendacoes/teste-enquete.mjs` (SQLite em memoria
  contra o worker inteiro, sem rede).

Criar uma enquete (operador, fora da migracao; exemplo para o dono rodar quando
quiser — **nao foi executado**):

```sql
INSERT INTO enquete (id, pergunta, arte, inicio, fim, ativa) VALUES
  ('logo-2026', 'Qual logo você prefere?', 'https://.../logo-480x270.png',
   strftime('%s','now'), strftime('%s','now') + 7*86400, 1);
INSERT INTO enquete_opcao (enquete, idx, texto) VALUES
  ('logo-2026', 1, 'Novo'), ('logo-2026', 2, 'Clássico renovado'), ('logo-2026', 3, 'Clássico');
```

## Ordem de deploy (nada disto foi feito)

1. **Migracao primeiro**, da raiz do repositorio:
   `npx wrangler@4 d1 execute nuvio-recomendacoes --remote --config servidor/recomendacoes/wrangler.toml --file servidor/recomendacoes/migracao-009-enquete.sql`
   (worker velho continua funcionando: nao conhece as tabelas novas).
2. **Depois o worker**: `npx wrangler@4 deploy --config servidor/recomendacoes/wrangler.toml`.
   Worker novo contra banco sem a 009 so quebra `/v1/enquete*` (500), nao as demais rotas.
3. So entao inserir a enquete (SQL acima) com `d1 execute ... --remote --command`.
4. Cliente: sem enquete ativa nada aparece; TV velha ignora o recurso.
Reverter: `UPDATE enquete SET ativa = 0`.

## Testes e capturas

- `bash tests/enquete.sh` — cliente contra servidor de mentira em C
  (`tests/enquete_servidor.inc`): espera antes do primeiro pedido, convite uma vez
  (e depois de reabrir o app), bolinha, voto unico, resultado agregado, falha de voto,
  prazo vencido, opt-out com Desfazer e pendencia, espelho de outra TV, troca de
  perfil com pedido em voo.
- `tests/ajustes_ux_dados.c` (ultima opcao = `AJ_ENQUETES`), `tests/ajustes_ux_interacao`.
- `bash tests/enquete_shot.sh <prefixo>` — capturas GL (Montserrat) pelo caminho real.
