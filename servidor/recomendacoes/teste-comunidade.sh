#!/usr/bin/env bash
# Teste da lista "Comunidade Nuvio Native" (POST /v1/perfis/comunidade, em
# src/amigos.js) contra `wrangler dev --local`. Mesma receita de teste-amigos.sh,
# e tambem PRECISA DE ESTADO LIMPO:
#
#   bash servidor/recomendacoes/preparar-local.sh
#   npx wrangler@4 dev --local --port 8799 --config servidor/recomendacoes/wrangler.toml
#   bash servidor/recomendacoes/teste-comunidade.sh
#
# O que ele prova: so quem publicou aparece; desligar tira na hora; nada da
# conta (nome, id, horario) sai; "assistindo agora" nunca sai; bloqueio esconde
# dos dois lados; quem nao se mostra nao ve a lista; paginacao.
set -u
cd "$(dirname "$0")/../.."
BASE="${1:-http://127.0.0.1:8799}"
NV_D1="${NV_D1:-$(find servidor/recomendacoes/.wrangler -name '*.sqlite' ! -name 'metadata.sqlite' 2>/dev/null | head -1)}"
H=(-H "content-type: application/json")
api() { local q="$1" m="$2" r="$3" b="${4:-}"
  if [ "$m" = GET ]; then curl -s -H "authorization: Bearer tok-$q" -H "x-nuvio-auth: nuvio" "$BASE$r"
  else curl -s -X POST -H "authorization: Bearer tok-$q" -H "x-nuvio-auth: nuvio" "${H[@]}" -d "${b:-{\}}" "$BASE$r"; fi; }
ok=0; falhou=0
checa() { if [ "$2" = "$3" ]; then ok=$((ok+1)); printf 'ok   %s\n' "$1"
  else falhou=$((falhou+1)); printf 'FALHOU %s\n  esperado: %s\n  obtido:   %s\n' "$1" "$2" "$3"; fi; }
tem() { printf '%s' "$1" | grep -c -- "$2"; }
sql() { sqlite3 "$NV_D1" "$1"; }
com() { api "$1" POST /v1/perfis/comunidade "${2:-{\}}"; }

[ -n "$NV_D1" ] || { echo "sem sqlite do D1 local (rode preparar-local.sh e o wrangler dev antes)"; exit 2; }
for q in a e f g h; do api $q POST /v1/eu > /dev/null; done

# --- quem nao se mostra nao ve -----------------------------------------------
r=$(com g)
checa "G sem perfil publicado recebe a lista fechada" 1 "$(tem "$r" '"fechado":1')"
checa "e vazia" 1 "$(tem "$r" '"pessoas":\[\]')"

# --- so quem publicou aparece ------------------------------------------------
api f POST /v1/perfil '{"apelido":"Fabi Cine","bio":"fa de terror","generos":["terror"],"avatar":1,"recentes":1}' > /dev/null
api g POST /v1/perfil '{"apelido":"Gui Nerd"}' > /dev/null
# E ligou SO o antigo "aparecer" (descobrivel sem apelido): nao tem nome escolhido
api e POST /v1/descobrivel '{"descobrivel":1}' > /dev/null
r=$(com g)
checa "G (publicado) ve a lista" 0 "$(tem "$r" '"fechado"')"
checa "F publicada aparece" 1 "$(tem "$r" '"apelido":"fabi cine"')"
checa "G nao aparece para si mesmo" 0 "$(tem "$r" 'gui nerd')"
checa "H (nunca ligou) nao aparece" 0 "$(tem "$r" 'Helena\|helena')"
checa "E (descobrivel sem apelido) nao aparece" 1 "$(printf '%s' "$r" | jq '.pessoas | length')"
checa "nao vaza nome da conta" 0 "$(tem "$r" 'Fabiana\|Souza\|Elisa')"
checa "nao vaza id da conta" 0 "$(tem "$r" 'nuvio:')"
checa "nao vaza horario nem codigo" 0 "$(tem "$r" '"ativo"\|"criado"\|"visto"\|"codigo"')"

# --- o que esta assistindo: so o publico, nunca "agora" ------------------------
api f POST /v1/perfil/atividade '{"nivel":2}' > /dev/null
api f POST /v1/atividade '{"imdb":"tt0111161","titulo":"Um Sonho de Liberdade","agora":0}' > /dev/null
r=$(com g)
checa "mostra o ultimo visto recente publico" 1 "$(tem "$r" '"vendo":"Um Sonho de Liberdade"')"
api f POST /v1/atividade '{"imdb":"tt0468569","titulo":"Batman Agora","agora":1}' > /dev/null
checa "'assistindo agora' (so de amigo) nao sai" 0 "$(tem "$(com g)" 'Batman Agora')"
api f POST /v1/perfil '{"apelido":"Fabi Cine","bio":"fa de terror","generos":["terror"],"avatar":1,"recentes":0}' > /dev/null
r=$(com g)
checa "recentes desligado: vendo vazio" 1 "$(tem "$r" '"vendo":""')"
checa "e nenhum titulo sai" 0 "$(tem "$r" 'Sonho')"

# --- ordem por atividade recente ------------------------------------------------
api h POST /v1/perfil '{"apelido":"Helena Tv","recentes":1}' > /dev/null
sql "UPDATE perfil SET atualizado = atualizado - 5000 WHERE pessoa IN ('nuvio:fff','nuvio:hhh');"
checa "mais recente primeiro (sem atividade: data do perfil)" "fabi cine" "$(com g | jq -r '.pessoas[0].apelido')"
api h POST /v1/atividade '{"imdb":"tt0068646","titulo":"O Poderoso Chefao","agora":0}' > /dev/null
checa "visto recente publico sobe a pessoa" "helena tv" "$(com g | jq -r '.pessoas[0].apelido')"

# --- relacao e bloqueio ---------------------------------------------------------
pub_f=$(com g | jq -r '.pessoas[] | select(.apelido=="fabi cine") | .pub')
api g POST /v1/pedidos/enviar "{\"pub\":\"$pub_f\"}" > /dev/null
checa "relacao 'enviado' na lista" "enviado" "$(com g | jq -r '.pessoas[] | select(.apelido=="fabi cine") | .relacao')"
checa "e 'recebido' do outro lado" "recebido" "$(com f | jq -r '.pessoas[] | select(.apelido=="gui nerd") | .relacao')"
api f POST /v1/bloquear '{"id":"nuvio:ggg"}' > /dev/null
checa "bloqueado nao ve quem bloqueou" 0 "$(tem "$(com g)" 'fabi cine')"
checa "quem bloqueou nao ve o bloqueado" 0 "$(tem "$(com f)" 'gui nerd')"
api f POST /v1/desbloquear '{"id":"nuvio:ggg"}' > /dev/null

# --- desligar tira na hora ---------------------------------------------------------
api h POST /v1/perfil/despublicar > /dev/null
checa "despublicou: some da lista na hora" 0 "$(tem "$(com g)" 'helena')"
api h POST /v1/descobrivel '{"descobrivel":0}' > /dev/null
checa "e quem despublicou perde a lista" 1 "$(tem "$(com h)" '"fechado":1')"

# --- paginacao ----------------------------------------------------------------------
for i in $(seq 1 25); do sql "INSERT OR IGNORE INTO pessoa (id, nome, descobrivel, criado, visto) VALUES ('nuvio:x$i','Conta Real $i',1,1,1);"; done
for i in $(seq 1 25); do
  sql "INSERT OR IGNORE INTO perfil (pessoa, pub, apelido, apelido_norm, atualizado) VALUES ('nuvio:x$i', printf('p%09d', $i), 'pessoa $i', 'pessoa$i', 100);"; done
r=$(com g)
checa "pagina 0 tem 20" 20 "$(printf '%s' "$r" | jq '.pessoas | length')"
checa "e avisa que ha mais" 1 "$(printf '%s' "$r" | jq '.mais')"
r=$(com g '{"pagina":1}')
checa "pagina 1 tem o resto (26 - 20)" 6 "$(printf '%s' "$r" | jq '.pessoas | length')"
checa "e nao ha mais" 0 "$(printf '%s' "$r" | jq '.mais')"
checa "paginas nao repetem ninguem" 26 "$( (com g | jq -r '.pessoas[].pub'; com g '{"pagina":1}' | jq -r '.pessoas[].pub') | sort -u | wc -l | tr -d ' ')"
checa "pagina negativa vira 0" 20 "$(com g '{"pagina":-3}' | jq '.pessoas | length')"
checa "nome da conta das 25 nao sai" 0 "$(tem "$(com g)" 'Conta Real')"

checa "sem token da 401" 401 "$(curl -s -o /dev/null -w '%{http_code}' -X POST -d '{}' "$BASE/v1/perfis/comunidade")"

printf '\n%d ok, %d falharam\n' "$ok" "$falhou"
[ "$falhou" -eq 0 ]
