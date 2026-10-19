// Catalogo de apresentacao. Incluir depois de Item/SEC/ROT/OPC/MAIS.
// A ordem do enum, CHAVE e valor[] nao participa deste mapa: reordenar aqui nao
// mexe em nada que esta no disco ou na conta.
//
// 2.0.3 (estudo ajustes-ux, aprovado pelo dono em 07/10): as categorias seguem
// com os NOMES que a comunidade conhece, mas na ordem da tarefa — Assistir
// (Reproducao, Idiomas e legendas), Telas (Tela inicial, Aparencia, Pagina do
// titulo, Cartazes e trailers) e Sistema (Esta TV, Sobre e ajuda). O que e
// tecnico fica DEPOIS do MAIS() da categoria: uma linha "Mais opcoes" fechada no
// fim da lista, que o OK abre no lugar (sem terceira tela). A pilula "Avancadas"
// do indice abre todas de uma vez. uxAvancada() le esta tabela; nao ha outra
// lista.
//
// Cliente so de VOD CinemaBox (dono, 19/10): conta, add-ons, debrid, TV ao vivo,
// P2P e metadados externos sairam da tela. As opcoes continuam no enum (valor[]
// e CHAVE[] sao posicionais) e o gate e `retirada()` em ajustes.c. Ver tambem
// RETIRADAS em tests/ajustes_secoes.py.
static const Item TELA[] = {
  // ---- ASSISTIR ------------------------------------------------------------------
  SEC("Reprodução", "Qualidade, som e o que o player faz", "aj_circle-play"),
    ROT("Imagem e som"),
      OPC(AJ_QUALIDADE), OPC(AJ_FONTE_HDR), OPC(AJ_DV), OPC(AJ_ATMOS),
#ifdef NV_TPK
      OPC(AJ_AUDIO_OFFSET),
#endif
    ROT("Escolha da fonte"),
      OPC(AJ_FONTE_MANUAL), OPC(AJ_FONTE_PRIORIDADE), OPC(AJ_FONTE_IDIOMA_AUDIO), OPC(AJ_FONTE_PRAZO),
      OPC(AJ_FONTE_REPOR), OPC(AJ_FILTROS_FONTES), OPC(AJ_TAM_MAX),
      OPC(AJ_FONTE_GRUPO), OPC(AJ_FONTE_GRUPO_SESSOES),
      OPC(AJ_LINK_REUSAR), OPC(AJ_LINK_PRAZO),
    ROT("Durante e depois"),
      OPC(AJ_PAUSA_OVERLAY), OPC(AJ_TELA_ABERTURA), OPC(AJ_ABERTURA_ADDON), OPC(AJ_ABERTURA_FUNDO), OPC(AJ_ABERTURA_ESTILO), OPC(AJ_ESTAT_TAMANHO), OPC(AJ_ESTAT_LADO), OPC(AJ_PLR_CLASSIF), OPC(AJ_PULAR_ABERTURA), OPC(AJ_PULAR_RESUMO),
      OPC(AJ_PULAR_CREDITOS), OPC(AJ_PROXIMO_AUTO), OPC(AJ_PROXIMO_CONTAGEM), OPC(AJ_REACAO_CREDITOS), OPC(AJ_CW_CONCLUIDO), OPC(AJ_CW_MINIMO),
      OPC(AJ_SAIDA_PLAYER), OPC(AJ_PROPORCAO_PADRAO), OPC(AJ_BRILHO_PLAYER), OPC(AJ_HDR_UI),
    ROT("Miniatura ao avançar"),
      OPC(AJ_SEEKR_LIGADO), OPC(AJ_SEEKR_CHAVE), OPC(AJ_SEEKR_TESTAR), OPC(AJ_SEEKR_FITA),
    MAIS(),
    ROT("Escolha da fonte"),
      // #310: o "Auto-play streams" do oficial (fonteregra.h).
      OPC(AJ_FONTE_REGEX), OPC(AJ_FONTE_REGEX_PADRAO), OPC(AJ_FONTE_REGEX_MODELO),
      OPC(AJ_FONTE_AUTO), OPC(AJ_FONTE_TOCAR_CONFERINDO), OPC(AJ_FONTE_AQUECER),
      OPC(AJ_FONTE_CONFERIR_VARIAS), OPC(AJ_FONTE_PREPARAR),
#ifdef NV_ANDROID
      OPC(AJ_FONTE_PRECARREGAR),   // so o Android tem o preload do player
#endif
      OPC(AJ_TAM_MIN), OPC(AJ_FONTE_TEXTO),
    ROT("Som e vídeo desta TV"),
      OPC(AJ_DTS_AC3),
#if !defined(NV_TPK) && !defined(NV_ANDROID)
      OPC(AJ_DV_MKV),
#endif
      OPC(AJ_CACHE_SEEK), OPC(AJ_MANTER_VIDEO), OPC(AJ_RETOMADA_MORNA), OPC(AJ_MANTER_PRAZO), OPC(AJ_SEEKR_AJUSTE),
#if defined(NV_ANDROID) || (!defined(NV_TPK) && !defined(__EMSCRIPTEN__))
      OPC(AJ_BUFFER),   // webOS: DTS/Dolby Vision conversion read-ahead
#endif
#ifdef NV_ANDROID
      OPC(AJ_PLAYER_EXTERNO),
      OPC(AJ_PLAYER_ANIME),
#endif

  SEC("Idiomas e legendas", "Idioma da interface, áudio e legenda", "aj_languages"),
    ROT("Idiomas"),
      OPC(AJ_IDIOMA), OPC(AJ_AUD_LINGUA), OPC(AJ_AUD_LINGUA2), OPC(AJ_LEG_LINGUA), OPC(AJ_LEG_FORCADA),
      OPC(AJ_LEG_MOSTRAR), OPC(AJ_LEG_SYNC_AUTO),
#ifdef NV_ANDROID
      // Sincronia pelo audio (F06): so o player do Android entrega o PCM. Na LG e
      // na Samsung a linha so diria "indisponivel nesta plataforma" (decisao do
      // dono, 04/10).
      OPC(AJ_LEG_SYNC_AUDIO),
#endif
    ROT("Legenda principal"),
      OPC(AJ_LEG1_TAMANHO), OPC(AJ_LEG1_FONTE), OPC(AJ_LEG1_COR), OPC(AJ_LEG1_OPACIDADE),
      OPC(AJ_LEG1_FUNDO), OPC(AJ_LEG1_POSICAO), OPC(AJ_LEG1_BORDA), OPC(AJ_LEG1_COR_BORDA),
      OPC(AJ_LEG1_NEGRITO), OPC(AJ_LEG1_ATRASO), OPC(AJ_LEG1_RESTAURAR),
    ROT("Segunda legenda"),
      OPC(AJ_LEG_LINGUA2),
      OPC(AJ_LEG2_TAMANHO), OPC(AJ_LEG2_FONTE), OPC(AJ_LEG2_COR), OPC(AJ_LEG2_OPACIDADE),
      OPC(AJ_LEG2_FUNDO), OPC(AJ_LEG2_POSICAO), OPC(AJ_LEG2_BORDA), OPC(AJ_LEG2_COR_BORDA),
      OPC(AJ_LEG2_NEGRITO), OPC(AJ_LEG2_ATRASO), OPC(AJ_LEG2_RESTAURAR), OPC(AJ_LEG2_POS),
      OPC(AJ_LEG_CONTORNO_COR),

  // ---- TELAS ---------------------------------------------------------------------
  SEC("Tela inicial", "Estilo, fileiras, barra lateral e Continuar assistindo", "aj_layout-dashboard"),
    ROT("Estilo"),
      OPC(AJ_HOME_LAYOUT), OPC(AJ_HERO), OPC(AJ_HERO_CHEIO),
      OPC(AJ_FUNDO_DIN), OPC(AJ_FUNDO_DIN_COR),
      OPC(AJ_HERO_CATALOGOS), OPC(AJ_HERO_GENERO),
      OPC(AJ_HERO_SINOPSE), OPC(AJ_HERO_NOTAS), OPC(AJ_HERO_ANO), OPC(AJ_HERO_DURACAO),
      OPC(AJ_HERO_SINOPSE_TAM),
    ROT("Fileiras"),
      OPC(AJ_FIL_ORDEM), OPC(AJ_FIL_LIMITE), OPC(AJ_ITENS_FILEIRA),
      OPC(AJ_ESPACO_FILEIRAS), OPC(AJ_ESPACO_TITULOS),
      OPC(AJ_OCULTAR_NLANC), OPC(AJ_NOTAS_HOME), OPC(AJ_SELO_VISTO), OPC(AJ_ABAS_ESTILO),
      OPC(AJ_OCULTAR_FILMES_VISTOS),
    ROT("Coleções"), OPC(AJ_COLECAO_EXIBICAO),
    ROT("Continuar assistindo"),
      OPC(AJ_CW_LIGADO), OPC(AJ_CW_OK), OPC(AJ_CW_ESTILO), OPC(AJ_CW_THUMB),
      OPC(AJ_CW_BLUR_PROX), OPC(AJ_CW_ORDEM), OPC(AJ_CW_NAO_EXIBIDOS),
      OPC(AJ_CW_RETIDO_TAMBEM), OPC(AJ_CW_PROXIMO),
    ROT("Barra lateral"),
      OPC(AJ_MENU_TOPO), OPC(AJ_RAIL_MODERNA), OPC(AJ_RAIL), OPC(AJ_MENU_EXPLORAR),
    ROT("Escolha de perfil"), OPC(AJ_PS_FUNDO), OPC(AJ_PS_CONTINUAR),
    MAIS(),
    ROT("Estilo"), OPC(AJ_HERO_TRANSICAO),
    ROT("Fileiras"), OPC(AJ_ROTULOS), OPC(AJ_NOME_ADDON), OPC(AJ_SUFIXO_TIPO),
    ROT("Continuar assistindo"), OPC(AJ_CW_FURTHEST),
    ROT("Barra lateral"), OPC(AJ_RAIL_BLUR),
#if (defined(NV_TPK) || defined(__EMSCRIPTEN__)) && !defined(NV_TPK40)
      OPC(AJ_MIC_BUSCA),
#endif

  SEC("Aparência", "Cor, transparência, fonte, animações e relógio", "aj_palette"),
#ifdef NV_TOUCH_UI
    OPC(AJ_INTERFACE_MODO),
#endif
#ifdef NV_TECLADO_SIS
    OPC(AJ_TECLADO_ANDROID),
#endif
    ROT("Cores e vidro"),
      OPC(AJ_TEMA), OPC(AJ_ARTE_TOM), OPC(AJ_FUNDO), OPC(AJ_COR_LOGO), OPC(AJ_VIDRO), OPC(AJ_VIDRO_CONTORNO), OPC(AJ_FOCO_ACENTO),
    ROT("Fileiras"), OPC(AJ_TITULO_FIL), OPC(AJ_PALCO_FUNDO),
    ROT("Logo e abertura"),
      OPC(AJ_LOGO_APP), OPC(AJ_ABERTURA), OPC(AJ_ICONE_APP),
    ROT("Texto e tamanho"),
      OPC(AJ_FONTE_UI), OPC(AJ_TAMANHO_UI), OPC(AJ_ESCALA_TEXTO), OPC(AJ_DET_ESPACO), OPC(AJ_TAMANHO_AJUSTES), OPC(AJ_LAYOUT_AJUSTES),
    ROT("Movimento e relógio"),
      OPC(AJ_ANIM), OPC(AJ_RELOGIO), OPC(AJ_RELOGIO_12H),
    ROT("Tela de descanso"),
      OPC(AJ_ESMAECER), OPC(AJ_DESCANSO_ESTILO), OPC(AJ_DESCANSO_FONTE),
    MAIS(),
    ROT("Cores e vidro"), OPC(AJ_VIDRO_OPAC), OPC(AJ_VIDRO_FOSCO), OPC(AJ_VIDRO_QUALIDADE),

  // Notas, TMDB e MDBList sairam com os metadados externos (dono, 19/10).
  SEC("Página do título", "O que aparece na página de cada título", "aj_file-text"),
    ROT("O que aparece"),
      OPC(AJ_DET_VEU), OPC(AJ_DET_DATA_CHEIA), OPC(AJ_DET_BLUR_NAO_VISTOS), OPC(AJ_EP_TAMANHO),

  // Cartazes e arte + Trailers numa categoria so (mockup v3, tela 01).
  SEC("Cartazes e trailers", "Cartaz em foco, de onde vem a arte e trailers", "aj_clapperboard"),
    ROT("Cartaz em foco"),
      OPC(AJ_EXPANDIR), OPC(AJ_EXPANDIR_ATRASO), OPC(AJ_BORDA_FOCO), OPC(AJ_MENU_CARTAZ),
    ROT("Forma"),
      OPC(AJ_LARGURA_DP), OPC(AJ_RAIO_DP), OPC(AJ_PROF),
    ROT("De onde vem a arte"),
      OPC(AJ_HERO_FUNDO), OPC(AJ_HERO_ARTE_DIF),
    ROT("Trailers"),
      OPC(AJ_HERO_TRAILER), OPC(AJ_FOCO_TRAILER), OPC(AJ_DET_TRAILER_AUTO),
      OPC(AJ_DET_TRAILER),
    ROT("Som e imagem"),
      OPC(AJ_HERO_TRAILER_SOM), OPC(AJ_TRAILER_FONTE),
#ifdef __EMSCRIPTEN__
      OPC(AJ_TRAILER_AUDIO),   // the dubbed trailer is a YouTube id: native players cannot play it
#endif
#ifdef NV_TPK
      OPC(AJ_TRAILER_ZOOM_TPK),
#endif
      OPC(AJ_LOGO_TRAILER),
    MAIS(),
    ROT("Cartaz em foco"), OPC(AJ_GRAD_CLASSICO), OPC(AJ_MURAL_TITULOS),
    ROT("Forma"),
      OPC(AJ_PROF_BRILHO),
    ROT("De onde vem a arte"),
      OPC(AJ_ADDON_POSTER), OPC(AJ_ADDON_FUNDO), OPC(AJ_ADDON_LOGO),
    ROT("Trailers"),
      OPC(AJ_HERO_TRAILER_ESPERA), OPC(AJ_TRAILER_QUAL), OPC(AJ_TRAILER_ASPECTO),

  // ---- SISTEMA -------------------------------------------------------------------
  SEC("Esta TV", "Resolução, imagens, efeitos e diagnóstico", "aj_monitor-cog"),
    OPC(AJ_RESOLUCAO), OPC(AJ_GPU_EFEITOS), OPC(AJ_AMPLIAR_UI), OPC(AJ_QUALIDADE_IMG),
    OPC(AJ_NAV_RAPIDA), OPC(AJ_MEDIDOR), OPC(AJ_DIAGNOSTICO), OPC(AJ_VELOCIDADE), OPC(AJ_ESPACO),
    MAIS(),
    ROT("Memória"), OPC(AJ_TEX_MB),

  SEC("Sobre e ajuda", "Guia de uso, versão e atualização", "aj_info"),
    OPC(AJ_GUIA),
    OPC(AJ_DICASCONTROLE),
    OPC(AJ_AVISO_LEGAL),
    ROTS("App", ""), OPC(AJ_VERSAO_I), OPC(AJ_ATUALIZAR),
};
#define AJ_N_TELA (int)(sizeof TELA / sizeof *TELA)
#define AJ_MAX_SECOES 16

// Texto exibido no painel ao selecionar uma categoria. Mesma ordem dos SECs.
static const char *SECAO_AJUDA[] = {
  "Qualidade máxima, Dolby Vision e Atmos, e o que acontece durante e depois do vídeo.",
  "Idioma da interface, preferências de áudio e legenda.",
  "Estilo da home, fileiras, Continuar assistindo, barra lateral e escolha de perfil.",
  "Cor de destaque, transparência, tipografia, animações e relógio na tela.",
  "Informações visíveis na página de cada título.",
  "Cartaz em foco, de onde vem a arte e os trailers da home e da página do título.",
  "Recursos e diagnóstico deste aparelho.",
  "Versão instalada, atualização e apresentação do app.",
};

// AVANCADA = vem depois do MAIS() da propria categoria. Calculado uma vez em
// montarTela (ajustes.c); a busca e os atalhos perguntam por aqui.
static unsigned char uxAvancadaTab[AJ_N];
static void montarTela(void);
static int uxAvancada(int op) {
  montarTela();
  return op >= 0 && op < AJ_N && uxAvancadaTab[op];
}
