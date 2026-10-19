"""Cobertura do catalogo UX executavel (ver tests/ajustes_secoes.sh)."""
import argparse
import re
import sys
from pathlib import Path


parser = argparse.ArgumentParser()
parser.add_argument("fonte", help="src/ajustes.c, de onde vem o enum OpcaoId")
parser.add_argument(
    "--catalogo",
    default=str(Path(__file__).resolve().parents[1] / "src/ajustes_ux_tela.inc"),
    help="include do catalogo; padrao src/ajustes_ux_tela.inc",
)
args = parser.parse_args()
fonte = Path(args.fonte).read_text(encoding="utf-8")
catalogo = Path(args.catalogo).read_text(encoding="utf-8")

enum = re.search(r"typedef enum \{(.*?)\n\} OpcaoId;", fonte, re.S)
if not enum:
    sys.exit("FALHA: enum OpcaoId nao encontrado em " + args.fonte)
corpo = re.sub(r"//[^\n]*", "", enum.group(1))
corpo = re.sub(r"^\s*#[^\n]*", "", corpo, flags=re.M)  # #ifdef NV_ANDROID slots
opcoes = [n.strip() for n in corpo.split(",") if n.strip() and n.strip() != "AJ_N"]
# Retired from the screen on purpose; the enum slot stays (positional valor[]/CHAVE[]).
RETIRADAS = {
    "AJ_RELOGIO_POS",  # 1.8: clock island is always top-right (inativa() == 1)
    "AJ_DESCOBRIR",    # 2.0.3: the web Discover screen does not exist on the TV (inativa() == 1)
    "AJ_AVANCADAS",    # 2.0.3: lives in the "Avancadas" pill at the top of the index
    # 2.0.3 (owner, 07/10): which ratings MDBList fetches follows "Notas no
    # titulo"; turning a rating on turns its pair on (notaLigarPar in ajustes.c).
    "AJ_MDB_TRAKT", "AJ_MDB_IMDB", "AJ_MDB_TMDB", "AJ_MDB_LETTER",
    "AJ_MDB_TOMATES", "AJ_MDB_AUDIENCIA", "AJ_MDB_META", "AJ_MDB_MAL",
    # 2.0.5 (owner, 12/10): profundidade virou o reflexo do foco do GFX_CARD,
    # com um controle so (AJ_PROF_BRILHO). Os outros AJ_PROF_* (borda, cobertura
    # e os cinco por tipo de cartao) sairam da tela de proposito; os slots ficam
    # no enum para o valor[] salvo manter os indices (migracao profundidade-205).
    "AJ_PROF_BORDA", "AJ_PROF_COBERTURA", "AJ_PROF_POSTERS", "AJ_PROF_CW",
    "AJ_PROF_EPS", "AJ_PROF_ELENCO", "AJ_PROF_TRAILERS",
    # 2.0.5 (#509): o som do trailer virou UM ajuste ("Som do trailer");
    # o slot de "Som do trailer na pagina do titulo" fica no enum pelo valor[]
    # salvo, e a migracao do carregar adota a escolha antiga.
    "AJ_DET_TRAILER_SOM",
    # 2.0.6 (#535): edited inside another row's editor, not as rows of their
    # own (uxOpcaoNaTela in ajustes_ux_desenho.inc): the per-provider limits
    # in "Ordem dos provedores", size/groups in "Filtros de fontes".
    "AJ_FALHA_RD_LIMITE", "AJ_FALHA_TB_LIMITE", "AJ_FALHA_PM_LIMITE", "AJ_FALHA_AD_LIMITE",
    "AJ_FILTRO_TAM_MAX", "AJ_FILTRO_GRUPOS",
    # 2.0.7 (dono, 19/10): cliente so de VOD CinemaBox. Conta/perfis sociais,
    # add-ons/plugins/debrid/servidores, TV ao vivo, debrid, P2P, metadados
    # (TMDB/MDBList/fanart/notas/poster externo), selos da conta, curadoria,
    # registro/apoio e a agenda sairam da tela; o gate e retirada() em ajustes.c.
    "AJ_PERFIL_ATIVO", "AJ_SYNC", "AJ_SAIR", "AJ_PERFIL_PESQ", "AJ_PERFIL_EDITAR", "AJ_HIST_CONTA",
    "AJ_SOCIAL", "AJ_DISCORD", "AJ_TRAKT", "AJ_SIMKL", "AJ_SALVOS_DEST", "AJ_CW_FONTE",
    "AJ_AMIGOS_ESTILO", "AJ_ENQUETES", "AJ_BUSCA_NUVIO", "AJ_BUSCA_ORIGEM", "AJ_ADDONS", "AJ_PLUGINS",
    "AJ_ADDONS_PRINCIPAL", "AJ_BUSCA_CINEMETA", "AJ_FALHA_PROVEDORES", "AJ_FALHA_PROVEDOR_ORDEM", "AJ_FALHA_ADDONS", "AJ_FALHA_ADDON_ORDEM",
    "AJ_FALHA_ADDON_LIMITE", "AJ_FONTE_ESCOPO", "AJ_FONTE_ADDONS_PERM", "AJ_FONTE_PLUGINS_PERM", "AJ_FONTE_OUTROS", "AJ_FONTE_ORDEM",
    "AJ_FONTE_ORDEM_USO", "AJ_FONTE_ORDEM_ADDON", "AJ_DEBRID_RD", "AJ_DEBRID_TB", "AJ_DEBRID_PM", "AJ_DEBRID_AD",
    "AJ_DEBRID_AD_TESTAR", "AJ_DEBRID_RD_TESTAR", "AJ_DEBRID_TB_TESTAR", "AJ_DEBRID_PM_TESTAR", "AJ_PROVEDORES_ESCOPO", "AJ_JF_LIGADO",
    "AJ_JF_SERVIDOR", "AJ_JF_ENTRAR", "AJ_JF_SAIR", "AJ_EM_SERVIDOR", "AJ_EM_ENTRAR", "AJ_EM_SAIR",
    "AJ_PX_ENTRAR", "AJ_PX_SERVIDOR", "AJ_PX_SAIR", "AJ_XTREAM_SERVIDOR", "AJ_XTREAM_USUARIO", "AJ_XTREAM_SENHA",
    "AJ_XTREAM_CONTA", "AJ_XTREAM_CONEXOES", "AJ_XTREAM_LIMPAR", "AJ_STALKER_PORTAL", "AJ_STALKER_MAC", "AJ_STALKER_LIMPAR",
    "AJ_EPG_PAIS", "AJ_LIVETV_SUGERIR", "AJ_LIVETV_DIAG", "AJ_LIVETV_RES", "AJ_LIVETV_ESPERA", "AJ_LIVETV_FORMATO",
    "AJ_LIVETV_MODO", "AJ_LIVETV_PROXY", "AJ_URL_LISTA", "AJ_URL_M3U_ADD", "AJ_URL_XMLTV_ADD", "AJ_URL_REM",
    "AJ_P2P_LIGADO", "AJ_P2P_URL", "AJ_P2P_TESTAR", "AJ_P2P_LIMITE", "AJ_P2P_DESTINO", "AJ_TMDB_LIGADO",
    "AJ_TMDB_IDIOMA", "AJ_TMDB_ARTE", "AJ_TMDB_BASICO", "AJ_TMDB_FICHA", "AJ_TMDB_DATAS", "AJ_TMDB_ELENCO",
    "AJ_TMDB_PROD", "AJ_TMDB_REDES", "AJ_TMDB_EPS", "AJ_TMDB_TRAILERS", "AJ_TMDB_MAIS", "AJ_TMDB_COL",
    "AJ_TMDB_CW", "AJ_MDB_LIGADO", "AJ_MDB_CHAVE", "AJ_FANART_CHAVE", "AJ_DET_META_EXT", "AJ_DET_SO_CINEMETA",
    "AJ_NT_IMDB", "AJ_NT_TOMATES", "AJ_NT_AUDIENCIA", "AJ_NT_META", "AJ_NT_METAUSER", "AJ_NT_TRAKT",
    "AJ_NT_TMDB", "AJ_NT_LETTER", "AJ_NT_MAL", "AJ_NT_EBERT", "AJ_NT_SCORE", "AJ_NT_ORDEM",
    "AJ_NT_EP", "AJ_POSTER_PROV", "AJ_POSTER_INST", "AJ_POSTER_TOKEN", "AJ_POSTER_EXTRA", "AJ_POSTER_CHAVE",
    "AJ_POSTER_MODELO", "AJ_POSTER_TESTAR", "AJ_COL_ARTE_CONTA", "AJ_SELOS_CORES", "AJ_SELOS_PACOTE", "AJ_SELOS_PACOTE_ADD",
    "AJ_SELOS_PACOTE_REM", "AJ_CURADORIA_URL", "AJ_MENU_GUIA", "AJ_MENU_AGENDA", "AJ_MENU_PERFIL", "AJ_LANDSCAPE",
    "AJ_APOIAR", "AJ_NOVIDADES20", "AJ_VER_REGISTRO", "AJ_ENVIAR_LOG", "AJ_ENVIO_AUTO", "AJ_BAIXAR_REGISTRO",
}
opcoes = [op for op in opcoes if op not in RETIRADAS]

tabela = re.search(r"static const Item TELA\[\] = \{(.*?)\n\};", catalogo, re.S)
if not tabela:
    sys.exit("FALHA: static const Item TELA[] nao encontrado em " + args.catalogo)
corpo_tela = re.sub(r"//[^\n]*", "", tabela.group(1))
itens = re.findall(r'\b(SEC|GRP|ROT|OPC)\(\s*("(?:[^"\\]|\\.)*"|AJ_[A-Z_0-9]+)', corpo_tela)

falhas = []
if not itens:
    falhas.append("nenhum item encontrado em TELA[]")
elif itens[0][0] != "SEC":
    falhas.append("TELA[] nao comeca por uma categoria (SEC)")
if any(t == "GRP" for t, _ in itens):
    falhas.append("TELA[] deve usar apenas blocos fixos ROT, sem GRP recolhivel")

na_tela = [v for t, v in itens if t == "OPC"]
for op in opcoes:
    n = na_tela.count(op)
    if n == 0:
        falhas.append("%s nao aparece em TELA[]: a opcao some da TV" % op)
    elif n > 1:
        falhas.append("%s aparece %d vezes em TELA[]" % (op, n))
for op in na_tela:
    if op not in opcoes:
        falhas.append("TELA[] cita %s, que nao existe no enum" % op)

# Categorias e blocos ROT precisam conter ao menos uma opcao.
abertos = {}
for t, v in itens + [("SEC", "<fim>")]:
    fecha = ("SEC", "ROT") if t == "SEC" else ("ROT",) if t == "ROT" else ()
    for tipo in fecha:
        if tipo in abertos:
            nome, n = abertos.pop(tipo)
            if n == 0:
                falhas.append("%s %s vazio(a) em TELA[]" % (tipo, nome))
    if t in ("SEC", "ROT"):
        abertos[t] = [v, 0]
    elif t == "OPC":
        for tipo in abertos:
            abertos[tipo][1] += 1

n_sec = sum(1 for t, _ in itens if t == "SEC")
# 19/10: 8 categorias — conta, add-ons/TV ao vivo e conta e servicos sairam com
# o cliente so de CinemaBox.
if n_sec != 8:
    falhas.append("esperadas 8 categorias, encontradas %d" % n_sec)

ajuda = re.search(r"static const char \*SECAO_AJUDA\[\] = \{(.*?)\n\};", catalogo, re.S)
if not ajuda:
    falhas.append("SECAO_AJUDA[] nao encontrado no include")
else:
    n_ajuda = len(re.findall(r'^\s*"', ajuda.group(1), re.M))
    if n_ajuda != n_sec:
        falhas.append("%d categorias e %d frases em SECAO_AJUDA" % (n_sec, n_ajuda))

# 2.0.3: avançada = vem depois do MAIS() da propria categoria (a linha "Mais
# opcoes"). Cada categoria tem no maximo um, e ele nao fica vazio.
avancadas = []
mais_por_sec, depois, sec_atual = {}, False, None
for t, v in re.findall(r'\b(SEC|ROT|OPC|MAIS)\(\s*("(?:[^"\\]|\\.)*"|AJ_[A-Z_0-9]+)?', corpo_tela):
    if t == "SEC":
        sec_atual, depois = v, False
    elif t == "MAIS":
        mais_por_sec[sec_atual] = mais_por_sec.get(sec_atual, 0) + 1
        depois = True
    elif t == "OPC" and depois:
        avancadas.append(v)
for sec, n in mais_por_sec.items():
    if n > 1:
        falhas.append("categoria %s tem %d MAIS()" % (sec, n))
if "AJ_SEEKR_AJUSTE" not in avancadas:
    falhas.append("AJ_SEEKR_AJUSTE deve permanecer avancada")
if "AJ_SAIDA_PLAYER" in avancadas:
    falhas.append("AJ_SAIDA_PLAYER deve permanecer basica")
if "AJ_QUALIDADE" in avancadas:
    falhas.append("AJ_QUALIDADE deve permanecer basica")

if falhas:
    for f in falhas:
        print("FALHA: " + f)
    sys.exit(1)

print("PASS: %d opcoes de Ajustes em %d categorias, cada uma exatamente uma vez (%d avancadas)."
      % (len(opcoes), n_sec, len(avancadas)))
