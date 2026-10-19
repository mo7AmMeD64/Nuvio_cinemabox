# Servidores de mídia — proposta para Nuvio 1.8

Pesquisa em 03/10/2026, baseada em documentação oficial atual e código Native. Estado: especificação; nenhum servidor autenticado, API privada, transcode ou aparelho foi testado nesta tarefa. Nada implementado ou prometido como pronto.

## Prioridade e fronteira

Proposta: Jellyfin primeiro, Emby segundo, Plex terceiro. A ordem decorre de integração técnica e independência do servidor, não de números de popularidade. Iniciar com bibliotecas de filmes/séries, detalhe, temporadas/episódios, seleção de versão/áudio/legenda, reprodução e progresso. Música, liveTV/DVR, downloads offline, administração, watch parties e controle remoto ficam fora do primeiro incremento.

| Sistema | Papel e razão de prioridade | Particularidade |
|---|---|---|
| Jellyfin | Servidor pessoal; bom primeiro adaptador HTTP/JSON compartilhado entre TVs | Login próprio e Quick Connect; descoberta LAN opcional |
| Emby | Servidor pessoal; abstrações parecidas, mas contrato próprio | Autenticação de usuário; não substituir por chave administrativa |
| Plex | Servidor pessoal com descoberta/autenticação associada ao plex.tv | Tokens por servidor, decision/transcode e políticas de acesso próprias |
| Kodi | Aplicação de media center com JSON-RPC | Biblioteca/controle remoto e URLs de arquivos não equivalem ao contrato de streaming dos três servidores |
| Finamp | Cliente de música Jellyfin | Não é um quarto backend de servidor para vídeo |
| SMB/NFS/WebDAV/DLNA | Protocolos/origens possíveis | Não fornecem por si só metadados/progresso/transcode equivalentes; investigação separada |

Kodi expõe `VideoLibrary.GetMovies` e namespaces Player/Files via JSON-RPC; um caminho local `smb://` ou `file://` obtido por RPC não se torna URL HTTP reproduzível na TV. Finamp se apresenta oficialmente como cliente Jellyfin de música. Essas distinções evitam multiplicar adaptadores fictícios. Fontes: [Kodi JSON-RPC](https://kodi.wiki/view/JSON-RPC_API/v13.5), [Finamp](https://github.com/finamp-app/finamp).

## Modelo comum no Native

Criar adaptadores propostos `mediasrv.c/h`, `jellyfin.c/h`, `emby.c/h`, `plex.c/h` em etapas futuras. HTTP usa `rede.c`/JSON existente, sempre em worker cancelável. Android pode usar SDK como referência; não criar implementação Kotlin independente que LG/Samsung não compartilhem.

Identidades: `(tipo, serverId, userId, itemId)` para catálogo/progresso, `(serverId, mediaSourceId)` para versão, e `playSessionId` para reprodução. Não usar só IMDb/TMDB: arquivo sem ID externo e duas versões/cortes podem representar objetos diferentes. Metadados externos enriquecem o detalhe, não substituem a identidade do servidor.

Novos registros separados:

- `MediaServerConnection`: tipo, origem/base path, serverId, userId, deviceId, referência local à credencial e versão/capacidades observadas.
- `MediaServerItem`: ID opaco, tipo, relações episódio/série, duração int64, imagens, IDs externos opcionais e estado remoto.
- `MediaServerPlayback`: item/source/session, URL temporária, cabeçalhos, método real escolhido, faixas, posição inicial e offset da timeline transcodificada.

`streams.h` hoje entrega URL4096/cabeçalhos512/metadados de add-on. Não enfiar login completo nesses campos nem persistir URL com token como identidade da fonte. Um adaptador resolve a URL imediatamente antes de tocar e fornece o contexto ao player; cabeçalhos precisam de auditoria de capacidade e truncamento. Preservar fluxo de add-ons e origem da biblioteca na UI: uma falha no servidor não apaga a biblioteca de add-ons.

## Jellyfin — contrato inicial

Jellyfin disponibiliza HTTP8096, HTTPS8920 quando habilitado e descoberta UDP7359 na sub-rede. Base URL pode conter prefixo de reverse proxy; preservar `/jellyfin/` em todas as rotas. Entrada manual de URL é obrigatória e funciona quando broadcast é bloqueado. Descoberta é conveniência, nunca login automático. Fonte: [Networking oficial](https://jellyfin.org/docs/general/post-install/networking/).

Login sugerido: Quick Connect na TV, usuário/senha como alternativa. O SDK oficial descreve `authenticateUserByName` e `authenticateWithQuickConnect`, retornando token/usuário. Senha usada apenas no pedido, depois descartada; manter token local por servidor/usuário. Fonte: [autenticação SDK](https://kotlin-sdk.jellyfin.org/guide/authentication.html). Quick Connect depende de aprovação em cliente já autenticado e pode estar desligado no servidor; expiração/cancelamento não deixam polling ativo. Fonte: [Quick Connect](https://jellyfin.org/docs/general/server/quick-connect/).

Rotas para implementação com schema pinado:

| Operação | Contrato candidato |
|---|---|
| Identificar servidor | `GET /System/Info/Public`; conferir id/nome/versão antes de credencial |
| Login senha | `POST /Users/AuthenticateByName` com corpo JSON e identificação cliente/device |
| Quick Connect | `/QuickConnect/Enabled`, `/QuickConnect/Initiate`, `/QuickConnect/Connect`, seguido de `/Users/AuthenticateWithQuickConnect` |
| Biblioteca/detalhe/episódios | Operações Items/UserViews/TVShows do schema da versão alvo, paginadas |
| Negociar reprodução | `POST /Items/{itemId}/PlaybackInfo` com DeviceProfile e escolhas no corpo |
| Estado playback | Família `/Sessions/Playing`, `/Progress`, `/Stopped` conforme schema da versão alvo |
| Legenda externa | URL/faixa oferecida no resultado MediaSource; negociar formato compatível |

IMPORTANTE: no controller oficial `master` examinado, Initiate é **POST**, Connect é GET; não copiar tutoriais que assumem GET para ambos. `master` não é prova de versão estável instalada: pin e testes por versão do servidor antes da implementação. Não tentar todos os verbos de autenticação em rajada. Fonte: [QuickConnectController](https://github.com/jellyfin/jellyfin/blob/master/Jellyfin.Api/Controllers/QuickConnectController.cs).

PlaybackInfo no controller atual aceita DeviceProfile no corpo e vários parâmetros de query estão marcados obsoletos. Há endpoint de teste de bytes `/Playback/BitrateTest` e abertura/fechamento de live source, úteis para investigação posterior; não são autorização para stress test ou liveTV no MVP. O resultado real escolhe mídia/URL e sessão, em vez de montar URL de transcode por heurística. Fonte: [MediaInfoController](https://github.com/jellyfin/jellyfin/blob/master/Jellyfin.Api/Controllers/MediaInfoController.cs).

## Emby — contrato separado

API usa `http[s]://host:port/emby/{apipath}`, JSON disponível. Login interativo deve usar autenticação de usuário; chave API destina-se a integrações sem contexto individual. Não reutilizar token Jellyfin nem presumir que cada rota/campo permaneceu igual após a separação dos projetos. Fonte: [Emby API](https://dev.emby.media/doc/restapi/index.html), [User Authentication](https://dev.emby.media/doc/restapi/User-Authentication.html).

Negociação: `POST /Items/{Id}/PlaybackInfo` com UserId, MediaSourceId, MaxStreamingBitrate, StartTimeTicks, índices de áudio/legenda, flags DirectPlay/DirectStream/Transcoding e DeviceProfile. Esse profile contém contratos de formatos, codecs e legendas; gerar a partir do backend/aparelho verificado. Não copiar um profile genérico de navegador para o player nativo. Fonte: [PlaybackInfo oficial](https://dev.emby.media/reference/RestAPI/MediaInfoService/postItemsByIdPlaybackinfo.html).

Enviar started/progress/stopped com ItemId, MediaSourceId e PlaySessionId. Documentação recomenda atualização automática a cada10s e imediatamente após interação; não fazer POST a cada frame. Posição usa ticks; marcar assistido é operação específica e independente. Fonte: [Playback Check-ins](https://dev.emby.media/doc/restapi/Playback-Check-ins.html).

Descoberta/Emby Connect são incrementos posteriores: começar pela URL explícita e autenticação documentada, sem assumir broadcast idêntico ao Jellyfin. Conferir permissões/termos e recursos dependentes de licença do servidor antes da entrega; esta pesquisa não validou contas/licenças Emby.

## Plex — contratos e política atuais

Solicitar JSON explicitamente, pois XML é padrão. Identificar cliente com X-Plex-Client-Identifier; login PIN oficial possui fluxo clássico e fluxo JWK/JWT documentado. Escolher um fluxo vigente e pinado, mostrar código/QR e cancelar polling/expiração. Token plex.tv e tokens dos PMS são distintos: resources retorna servidores/conexões/tokens; preferir conexão local, depois direta remota, relay como último recurso. Usar URLs retornadas, sem desativar TLS de plex.direct.

Para tocar, usar metadata do item e `/video/:/transcode/universal/decision` com profile/capacidade/qualidade, depois recurso start indicado ou Part directplay. Reportar `/:/timeline` por sessão; preservar índice de mídia/part e unidade em milissegundos. Não compartilhar token entre servidores nem inferir transcode de extensão.

Fonte dessas operações: [Plex API oficial](https://developer.plex.tv/pms/). A documentação completa excedeu o limite do visualizador web; endpoints/fluxos acima foram conferidos nos trechos oficiais indexados. Antes de implementar, baixar/pinar OpenAPI e conferir schemas/versão/termos.

Acesso remoto a vídeo pessoal tem requisitos de Plex Pass/Remote Watch Pass conforme conta/administrador. Tratar resposta de entitlement como estado de acesso, nunca simular plataforma oficial para contornar. Relay tem teto documentado de2Mbps; informar origem/limitação e usar demanda compatível, não chamar isso de medição da internet da TV. Fontes: [requisitos remotos](https://support.plex.tv/articles/200931138-troubleshooting-remote-access/), [Relay](https://support.plex.tv/articles/216766168-accessing-a-server-through-relay/).

## Playback e capabilities por aparelho

Proposta comum: DirectPlay quando container/vídeo/áudio/legenda e bitrate passam pelas capacidades verificadas; DirectStream/remux quando converter container/áudio/legenda resolve; vídeo transcode quando necessário e permitido. A documentação Jellyfin distingue esses caminhos e ressalta custo de burn-in. As tabelas de clientes oficiais não comprovam capacidades do Nuvio em uma LG C9 ou Samsung. Fonte: [Codec Support](https://jellyfin.org/docs/general/clients/codec-support/).

Guardar profile por TV/backend: Android Media3/decoder/hardware e rota de áudio; LG uMS/firmware; Samsung WGT/AVPlay; Samsung TPK separado. Codec nominal não basta: perfil/nível, bit depth, HDR/DV, interlacing, canais e passthrough alteram o resultado. Capacidade desconhecida não deve ser anunciada como suportada só para evitar transcode.

Aplicar orçamento conservador de vazão real ao MaxStreamingBitrate quando houver medida confiável do mesmo caminho servidor/rede. Usuário pode escolher qualidade original; informar provável demanda e método. Um servidor lento/transcoder saturado não prova Wi-Fi lento. Evitar loop infinito de fallback: tentativa directplay, alternativa negociada, diagnóstico final com motivo e escolha manual.

Provar headers em mídia progressiva e em TODAS requisições HLS (manifest/segmento/key/subtitle), redirects e Range. LG/Samsung URI fechada pode exigir URL autenticada query que o servidor suporta; se necessário usar proxy local controlado e demonstrar viabilidade, nunca enviar token a proxy público. Não registrar URL secreta. Ao parar/trocar sessão, avisar servidor e liberar recurso de transcode/live aberto, sem ficar prendendo CPU do servidor.

Legenda tem origem servidor distinguida de embutida/add-on. Preferir extração/legenda externa quando suportada para evitar burn-in involuntário; PGS/ASS e estilos exigem prova. AutoSync de1.8 não deve corrigir duas vezes a timeline: vídeo transcodificado que inicia em StartTimeTicks requer conversão para tempo absoluto do item. Segunda legenda e idioma são seleção de usuário, não desculpa para forçar transcode sem informar.

## Watch state, perfis e privacidade

Para itens dessas bibliotecas, servidor é autoridade do progresso/assistido. Guardar cache local com origem completa e fila de eventos específica; não mandar IDs opacos ao progresso Nuvio que hoje identifica conteúdo de add-ons. Conta Nuvio e usuário de servidor têm mapeamento explícito: mesmo perfil Nuvio pode conectar usuários diferentes em TVs diferentes, nunca adivinhar que nomes iguais representam mesma identidade.

Eventos com geração de login/perfil/item/session: started só após playback real; progress com posição confirmada, seek/pausa e flags; stopped com última posição válida. Falha de rede preserva estado local e não repete started/stopped cegamente em outra sessão. Retomar usa segundos↔ticks int64 e duração do item original. Reconciliação compara eventos recentes e não aplica pull antigo por cima de progresso recém-gravado; não transportar monotonic counters entre processos como timestamps universais.

Tokens ficam por aparelho e servidor/usuário, sem sync em conta Nuvio/telemetria/M3. Não guardar senha após login. Android secure storage pode proteger token; LG/WGT/TPK storage local não equivale a um cofre e deve ser descrito honestamente. Logout apaga token, polling, jobs, snapshots sensíveis e URLs. Redação em logs English com tipo/método/status/latência, nunca segredo/base privada/título pessoal completo.

Verificar origem/basepath e limitar redirect autenticado: não reenviar token para host arbitrário. HTTPS confiável por padrão; servidor LAN HTTP explícito não recebe credenciais de outro servidor. ServerId ou certificado alterado requer reconexão consciente, não associação automática com cache antigo. URLs obtidas por descoberta são não confiáveis até checagem do servidor. Imagens autenticadas precisam da mesma política de headers/cache e sanitização de nomes/caminhos.

## Provas e sequência

1. Pin schemas e versões reais dos três servidores; fixtures públicas mínimas, sem dados pessoais. Conferir SDK/client licenses antes de copiar código; uso REST não exige embutir server. API proprietária/termos Plex/Emby precisam revisão no estágio da implementação.
2. Jellyfin: login/cancelamento, biblioteca paginada, detalhe e filme DirectPlay no host, depois4 backends. Emby reusa UI/modelo com fixtures próprias; não transportar schemas implicitamente.
3. PlaybackInfo, subtitles, resume e check-ins; testar remux/transcode e encerramento de sessão. Plex só após recursos/auth e decision completarem esse contrato.
4. Integração Home/Continuar assistindo com origem visível e perfis isolados. Relação com [plano do player](../player-1.8/README.md) para bitrate, legenda e capabilities.

Testes significativos: prefixo URL/reverse proxy, descoberta falsa, token401 versus rede503, loginQR expirado/cancelado, paginação grandes bibliotecas, IDs semIMDb, multiversão, ticks int64, áudio semsuporte, PGS/ASS, HLS autenticado/redirect/Range, transcode offset/seek, parada/alternância, outagefila e perfis/contas trocados durante request. Conferir exclusão logout e que logs não contêm tokens/URLs assinadas.

Benchmark por aparelho e versão do servidor:10 repetições frias/quentes, mediana/p95 startup, scroll/search, seek/rebuffer, RAM app e recursos servidor/transcode. Comparar DirectPlay/remux/transcode separadamente, mesma mídia/hash/rede. Registrar HDR/áudio/legenda efetivamente entregues, não só flags metadata. Medir chamadas por sessão e ausência de polling depois de sair. Aprovação visual e prova nas TVs são gates separados de testes host; esta pesquisa não satisfaz esses gates.
