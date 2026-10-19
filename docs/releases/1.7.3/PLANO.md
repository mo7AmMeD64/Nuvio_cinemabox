# Preparação local da 1.7.3

Base: release/1.7.2, e1f2936b8695b90c9c9b545a2b2763d1bd33f83e. Branch de integração: codex/release-1.7.3, worktree gerenciado release-173-integration. Pedido do usuário em 2026-10-03: priorizar fixes derivados dos logs, Settings sem imagens provisórias, features úteis ainda ausentes entre trabalhos dispersos e ícones opcionais.

## Escopo 1.7.3

- Remover ilustrações provisórias do Settings, mantendo explicação, alcance e padrão.
- Auditar acesso às funções antigas. O enum da 1.7.1 não perde opções na 1.7.2: o catálogo tinha 184 opções, 61 avançadas antes da correção de acesso ao contorno; agora são 60; a nova opção de ícone foi acrescentada no fim sem deslocar índices persistidos.
- Recuperar Onde ver por hunks, preservando fontes parciais, reconexão e estado atuais. Lista regional de serviços TMDB/JustWatch e abertura do app instalado/loja; não promete deep link para o título nem comprova assinatura do usuário.
- Corrigir sincronização de coleções quando a RPC responde objeto e a contagem do array raiz é zero. Persistir snapshot por perfil e preservá-lo em falha da rede.
- Incluir ícones opcionais com o portão existente de apoiador, ainda provisório via ambiente/arquivo. Sem integração de pagamento/Patreon nesta entrega. LG/Samsung alteram marca interna; Android também agenda alias/banner do launcher para a saída.
- Conservar código de erro do player Samsung para diagnóstico e mensagem de falha da fonte. Antes o log tinha o código, mas video_erro_texto() devolvia vazio; isso não resolve por si só o erro do fornecedor/decoder.

## Trabalho antigo já incorporado

O patch 30b2c079 de Continuar assistindo não será reaplicado: player_encerrar já chama desc_refazer_continuar e descoberta.c preserva a janela atualizada entre publicações concorrentes. Home antiga, cache/perfis e fontes parciais têm implementações mais recentes na release: não copiar os arquivos antigos por inteiro. O cache atual já prioriza artes urgentes e protege texturas recentemente usadas; não portar a proteção antiga de oito segundos sem demonstrar vantagem. Mudanças locais de streams do player Glass são visuais e pertencem à 1.8.

## Escopo reservado à 1.8

Social/Discord, Plugins/P2P, Glass UI e o novo pacote AutoSync/áudio, segundo idioma, buffer de seek, boost, ajuste de fontes pela conexão, Seekr 50/dia e chave pessoal, perfis de desempenho por aparelho, diagnósticos/speed test e seletor simplificado de legendas. Especificação: ../../plans/player-1.8/README.md. Chave pessoal Seekr e limite ficam juntos nesse escopo, para não reabrir uso sem a política solicitada.

Servidores pessoais Jellyfin, Emby e Plex também entram na investigação da 1.8: [proposta de integração](../../plans/media-servers-1.8/README.md).

## Publicação e aparelhos

Esta árvore prepara uma candidata local. Nenhuma alteração ao servidor, publicação, tag, instalação ou controle de TV faz parte desta rodada. Testes no Mac e compilação não comprovam comportamento LG C9, Samsung ou Android TV real. Registrar separadamente fonte, build, versão, host/núcleo e cenário antes de validar aparelhos. O appinfo.json é a versão canônica para Android e env; scripts TPK reescrevem manifest de build e restauram o original.
