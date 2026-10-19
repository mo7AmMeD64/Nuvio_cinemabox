#!/usr/bin/env bash
# Teste de ponta a ponta do que a tela "Encontrar pessoas" passou a pedir ao
# servico (src/amigos.js, src/index.js): nome + apelido, selo de criador e os
# outros perfis da propria conta fora das listas. Mesma receita de
# teste-amigos.sh, com UMA diferenca: o selo sai da lista `CRIADORES`, e aqui
# ela aponta para uma identidade de teste (nuvio:ccc, "Carolina"):
#
#   bash servidor/recomendacoes/preparar-local.sh
#   npx wrangler@4 dev --local --port 8799 --var CRIADORES:nuvio:ccc \
#       --config servidor/recomendacoes/wrangler.toml
#   bash servidor/recomendacoes/teste-pessoas.sh
#
# PRECISA DE ESTADO LIMPO (preparar-local.sh). Nada aqui fala com o D1 remoto.
#
# PERFIL DA CASA: o mesmo token com `X-Nuvio-Perfil: <n>` e outra pessoa da
# MESMA conta (`nuvio:ggg:2`). E esse par que prova o corte por conta.
set -u
cd "$(dirname "$0")/../.."
BASE="${1:-http://127.0.0.1:8799}"
NV_D1="${NV_D1:-$(find servidor/recomendacoes/.wrangler -name '*.sqlite' ! -name 'metadata.sqlite' 2>/dev/null | head -1)}"
H=(-H "content-type: application/json")
# chamada: quem[:perfil] metodo rota [corpo]
api() { local q="${1%%:*}" n="" m="$2" r="$3" b="${4:-}"
  case "$1" in *:*) n="${1##*:}";; esac
  local P=(); [ -n "$n" ] && P=(-H "x-nuvio-perfil: $n")
  if [ "$m" = GET ]; then curl -s -H "authorization: Bearer tok-$q" -H "x-nuvio-auth: nuvio" ${P[@]+"${P[@]}"} "$BASE$r"
  else curl -s -X POST -H "authorization: Bearer tok-$q" -H "x-nuvio-auth: nuvio" ${P[@]+"${P[@]}"} "${H[@]}" -d "${b:-{\}}" "$BASE$r"; fi; }
ok=0; falhou=0
checa() { if [ "$2" = "$3" ]; then ok=$((ok+1)); printf 'ok   %s\n' "$1"
  else falhou=$((falhou+1)); printf 'FALHOU %s\n  esperado: %s\n  obtido:   %s\n' "$1" "$2" "$3"; fi; }
tem() { printf '%s' "$1" | grep -c -- "$2"; }
sql() { sqlite3 "$NV_D1" "$1"; }
# o cartao de um apelido dentro de uma lista
de() { printf '%s' "$1" | jq -c --arg a "$3" ".$2[]? | select(.apelido == \$a)"; }

[ -n "$NV_D1" ] || { echo "sem sqlite do D1 local (rode preparar-local.sh e o wrangler dev antes)"; exit 2; }

for q in c c:3 d e g g:2; do api $q POST /v1/eu > /dev/null; done
checa "o perfil 2 e outra pessoa da mesma conta" 1 "$(sql "SELECT COUNT(*) FROM pessoa WHERE id='nuvio:ggg:2';")"

api c   POST /v1/perfil '{"apelido":"carol dev","recentes":1}' > /dev/null
api c:3 POST /v1/perfil '{"apelido":"carol sala"}' > /dev/null
api d   POST /v1/perfil '{"apelido":"dani tv"}' > /dev/null
api e   POST /v1/perfil '{"apelido":"elisa cine"}' > /dev/null
api g   POST /v1/perfil '{"apelido":"gui nerd"}' > /dev/null
api g:2 POST /v1/perfil '{"apelido":"gui kids","recentes":1}' > /dev/null

# --- 1. SELO: so de quem esta em CRIADORES, e pela conta ---------------------------
r=$(api d POST /v1/perfis/buscar '{"q":"car"}')
checa "busca acha os dois perfis da criadora" 2 "$(printf '%s' "$r" | jq '.resultados | length')"
checa "perfil principal leva o selo" criador "$(de "$r" resultados "carol dev" | jq -r .selo)"
checa "o outro perfil da conta tambem" criador "$(de "$r" resultados "carol sala" | jq -r .selo)"
checa "o id da conta nao sai junto com o selo" 0 "$(tem "$r" 'nuvio:')"
r=$(api d POST /v1/perfis/buscar '{"q":"gui"}')
checa "quem nao esta na lista nao tem o campo" 0 "$(tem "$r" '"selo"')"
# apelido igual ao de um criador nao da selo: a regra e a conta
api e POST /v1/perfil '{"apelido":"carol dev"}' > /dev/null
r=$(api d POST /v1/perfis/buscar '{"q":"car"}')
checa "apelido copiado: so a conta certa leva selo" 2 "$(printf '%s' "$r" | jq '[.resultados[] | select(.selo == "criador")] | length')"
checa "e o imitador aparece sem selo" 1 "$(printf '%s' "$r" | jq '[.resultados[] | select(.apelido == "carol dev" and (.selo // "") == "")] | length')"
api e POST /v1/perfil '{"apelido":"elisa cine"}' > /dev/null
pub_c=$(api c GET /v1/perfil | jq -r .pub)
checa "cartao aberto leva o selo" criador "$(api d POST /v1/perfis/ver "{\"pub\":\"$pub_c\"}" | jq -r .selo)"
r=$(api d POST /v1/perfis/comunidade '{}')
checa "comunidade: selo na criadora" criador "$(de "$r" pessoas "carol dev" | jq -r .selo)"
checa "comunidade: sem selo nos outros" "" "$(de "$r" pessoas "gui nerd" | jq -r '.selo // ""')"

# --- 2. NOME: estranho ve so o apelido; amigo ve os dois ------------------------------
r=$(api d POST /v1/perfis/buscar '{"q":"car"}')
checa "estranho: campo nome existe e vem vazio" "" "$(de "$r" resultados "carol sala" | jq -r .nome)"
checa "estranho: o nome da conta nao aparece em lugar nenhum" 0 "$(tem "$r" 'Carolina')"
checa "comunidade para estranho: sem nome" 0 "$(tem "$(api d POST /v1/perfis/comunidade '{}')" 'Carolina')"
api d POST /v1/pedidos/enviar "{\"pub\":\"$pub_c\"}" > /dev/null
r=$(api c GET /v1/pedidos)
checa "pedido recebido: apelido de quem pediu" 1 "$(tem "$r" '"apelido":"dani tv"')"
checa "pedido recebido: ainda sem o nome da conta" 0 "$(tem "$r" 'Daniel')"
pub_d=$(api d GET /v1/perfil | jq -r .pub)
api c POST /v1/pedidos/aceitar "{\"pub\":\"$pub_d\"}" > /dev/null
r=$(api d POST /v1/perfis/buscar '{"q":"car"}')
checa "amigo: nome e apelido juntos" "Carolina|carol dev|amigo" "$(de "$r" resultados "carol dev" | jq -r '.nome + "|" + .apelido + "|" + .relacao')"
checa "o outro perfil dela continua estranho: sem nome" "" "$(de "$r" resultados "carol sala" | jq -r .nome)"
checa "cartao de amigo: nome" Carolina "$(api d POST /v1/perfis/ver "{\"pub\":\"$pub_c\"}" | jq -r .nome)"
checa "comunidade: nome so no amigo" "Carolina" "$(api d POST /v1/perfis/comunidade '{}' | jq -r '[.pessoas[] | .nome | select(. != "")] | join(",")')"
r=$(api d GET /v1/contatos)
checa "lista de amigos: nome, apelido e selo" "Carolina|carol dev|criador" "$(printf '%s' "$r" | jq -r '.contatos[0] | .nome + "|" + .apelido + "|" + .selo')"
r=$(api c GET /v1/contatos)
checa "lista de amigos: quem nao e criador nao tem selo" "Daniel|dani tv|" "$(printf '%s' "$r" | jq -r '.contatos[0] | .nome + "|" + .apelido + "|" + (.selo // "")')"
# "Amigo #<n>" e reserva de quem nao tem nome, nao um nome
sql "UPDATE pessoa SET nome = 'Amigo #9' WHERE id = 'nuvio:ddd';"
checa "reserva 'Amigo #n' nao sai como nome" "" "$(api c POST /v1/perfis/buscar '{"q":"dan"}' | jq -r '.resultados[0].nome')"

# --- 3. OS OUTROS PERFIS DA MINHA CONTA NAO SAO "PESSOAS" -----------------------------
checa "busca: estranho ve os dois perfis da casa" 2 "$(api d POST /v1/perfis/buscar '{"q":"gui"}' | jq '.resultados | length')"
checa "busca: o principal nao ve o perfil 2" 0 "$(api g POST /v1/perfis/buscar '{"q":"gui"}' | jq '.resultados | length')"
checa "busca: o perfil 2 nao ve o principal" 0 "$(api g:2 POST /v1/perfis/buscar '{"q":"gui"}' | jq '.resultados | length')"
cod_g2=$(sql "SELECT codigo FROM pessoa WHERE id='nuvio:ggg:2';")
checa "busca pelo codigo do proprio perfil 2: nada" 0 "$(api g POST /v1/perfis/buscar "{\"q\":\"$cod_g2\"}" | jq '.resultados | length')"
r=$(api g POST /v1/perfis/comunidade '{}')
checa "comunidade: sem o perfil 2 da propria conta" 0 "$(tem "$r" 'gui kids')"
checa "comunidade: o resto continua la" 1 "$(tem "$r" 'dani tv')"
checa "comunidade do perfil 2: sem o principal" 0 "$(tem "$(api g:2 POST /v1/perfis/comunidade '{}')" 'gui nerd')"
checa "comunidade de um estranho: os dois" 2 "$(api d POST /v1/perfis/comunidade '{}' | jq '[.pessoas[] | select(.apelido | startswith("gui"))] | length')"
# gosto parecido: o perfil 2 publicou tres vistos; o principal pergunta com os mesmos
for t in tt0111161 tt0068646 tt0468569; do api g:2 POST /v1/atividade "{\"imdb\":\"$t\",\"titulo\":\"x\"}" > /dev/null; done
Q='{"imdbs":["tt0111161","tt0068646","tt0468569"]}'
checa "gosto parecido: um estranho recebe o perfil 2" 1 "$(tem "$(api d POST /v1/perfis/sugeridos "$Q")" 'gui kids')"
checa "gosto parecido: o principal da conta nao" 0 "$(tem "$(api g POST /v1/perfis/sugeridos "$Q")" 'gui kids')"
# amigo de amigo: G e amigo de E, E e amiga do perfil 2 de G
sql "INSERT OR IGNORE INTO contato (a, b, criado, via) VALUES
  ('nuvio:ggg','nuvio:eee',1,'codigo'),('nuvio:eee','nuvio:ggg',1,'codigo'),
  ('nuvio:eee','nuvio:ggg:2',1,'codigo'),('nuvio:ggg:2','nuvio:eee',1,'codigo'),
  ('nuvio:eee','nuvio:ccc:3',1,'codigo'),('nuvio:ccc:3','nuvio:eee',1,'codigo');"
r=$(api g POST /v1/sugestoes '{}')
checa "sugestoes: sem o perfil 2 da propria conta" 0 "$(tem "$r" 'gui kids')"
checa "sugestoes: amigo de amigo continua vindo, com selo" criador "$(printf '%s' "$r" | jq -r '.sugestoes[] | select(.nome == "carol sala") | .selo')"

printf '\n%d ok, %d falharam\n' "$ok" "$falhou"
[ "$falhou" -eq 0 ]
