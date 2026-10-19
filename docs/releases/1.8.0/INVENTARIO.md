# Inventário Git para fechamento1.8

03/10/2026. Consulta local de38 checkouts de desenvolvimento; diretórios de build no SSD e cópias de empacotamento foram excluídos da tabela. O estado é um snapshot, não uma garantia de que nada mudou depois.

-22 HEADs já eram ancestrais da integração;16 não eram ancestrais;11 checkouts tinham alterações locais.
-A comparação usou a integração e6885991; `git cherry` foi aplicado aos candidatos principais para distinguir patch equivalente de commit apenas diferente.
-Arquivos não commitados continuam pendentes de comparação, mesmo numa branch ancestral.
-Checkout principal preservado com90 arquivos modificados/não rastreados. Não fazer merge/commit/reset nele.

## Destino por origem

| Origem | HEAD | Relação | Patches diferentes / equivalentes | WIP | Destino |
|---|---|---|---|---:|---|
| `fix/tpk-selo-hdr` | `2131cb93` | divergente | 1 / 1 | 90 | Auditar o delta Biblioteca2131cb93 e90 WIP por família; não substituir módulos novos ou limpar o checkout. |
| `agente/ajustesvisual` | `011021c6` | divergente | 2 / 0 | 0 | Experimento anterior de Settings; conservar como referência, sem substituir Ajustes v2. |
| `agente/guiaunicode` | `6b0f0659` | divergente | 1 / 0 | 0 | Código e fixture Unicode já presentes na integração; `tests/jscadeia.sh` passou em03/10. Não reaplicar commit inteiro. |
| `agente/i195f` | `2aa724f4` | ancestral | 0 / — | 4 | Código commitado já incorporado; revisar arquivos WIP antes de encerrar esta origem. |
| `agente/icones` | `e6010365` | divergente | 5 / 0 | 0 | Integrar seletivamente os5 commits de ícones/alias e testar Android ao fechar; conferir #226/#234. |
| `teste/ondever-171` | `2bc9659b` | ancestral | 0 / — | 80 | Código commitado já incorporado; revisar arquivos WIP antes de encerrar esta origem. |
| `teste/ondever-android` | `77313de8` | ancestral | 0 / — | 109 | Código commitado já incorporado; revisar arquivos WIP antes de encerrar esta origem. |
| `agente/p2p` | `526eabcf` | divergente | não comparado | 0 | Linha anterior; usar p2p2 como candidata e evitar duplicação. |
| `agente/p2p2` | `b1d20394` | divergente | 13 / 0 | 0 | Portar motor/APIs/builds após revisão de cancelamento/orçamento/limpeza; prova real por plataforma. |
| `agente/plugins` | `1a3dafcd` | divergente | não comparado | 0 | Linha anterior; usar plugins2 como candidata e evitar duplicação. |
| `agente/plugins2` | `e908c412` | divergente | 21 / 0 | 0 | Portar módulos/HTTP por chamada/UI após limites globais/gerações/cancelamento;21 patches exclusivos na comparação feita. |
| `agente/tr166` | `233e3222` | ancestral | 0 / — | 1 | Código commitado já incorporado; revisar arquivos WIP antes de encerrar esta origem. |
| `agente/codex-ajustes-arte` | `b3d870bc` | divergente | 3 / 0 | 0 | Comparar3 deltas de arte/animação com Settings v2 e decisão visual já existente; não importar outro layout. |
| `agente/codex-contexto` | `563c658d` | divergente | não comparado | 0 | Documento de referência; incorporar decisões vigentes, observações de build antiga não são estado atual. |
| `feat/discord-presenca` | `6b6bfe14` | divergente | 1 / 0 | 0 | Comparar semanticamente com Discord1.7.4 já funcional; patch distinto não significa feature ausente. |
| `agente/escala` | `81169b99` | ancestral | 0 / — | 0 | Já incorporado; nenhum novo merge necessário pelo HEAD consultado. |
| `agente/glass-ajustes` | `a57e0dea` | ancestral | 0 / — | 1 | Código commitado já incorporado; revisar arquivos WIP antes de encerrar esta origem. |
| `agente/ilha-tudo` | `44260bda` | ancestral | 0 / — | 1 | Código commitado já incorporado; revisar arquivos WIP antes de encerrar esta origem. |
| `agente/fix-menu` | `94e9a6eb` | ancestral | 0 / — | 0 | Já incorporado; nenhum novo merge necessário pelo HEAD consultado. |
| `agente/fix-modais` | `83cd1e42` | ancestral | 0 / — | 0 | Já incorporado; nenhum novo merge necessário pelo HEAD consultado. |
| `agente/fix-social` | `2e2b28a5` | ancestral | 0 / — | 0 | Já incorporado; nenhum novo merge necessário pelo HEAD consultado. |
| `agente/fix-telas` | `5a60f592` | ancestral | 0 / — | 0 | Já incorporado; nenhum novo merge necessário pelo HEAD consultado. |
| `agente/fontes-cab` | `4afe0390` | ancestral | 0 / — | 0 | Já incorporado; nenhum novo merge necessário pelo HEAD consultado. |
| `feat/glass-ilha` | `16b475b1` | ancestral | 0 / — | 0 | Já incorporado; nenhum novo merge necessário pelo HEAD consultado. |
| `agente/glass-ui` | `430c2dc7` | ancestral | 0 / — | 1 | Código commitado já incorporado; revisar arquivos WIP antes de encerrar esta origem. |
| `agente/player2` | `a7648756` | ancestral | 0 / — | 0 | Já incorporado; nenhum novo merge necessário pelo HEAD consultado. |
| `agente/player3` | `6542d317` | ancestral | 0 / — | 0 | Já incorporado; nenhum novo merge necessário pelo HEAD consultado. |
| `agente/selos-pacote` | `a8b60da8` | ancestral | 0 / — | 0 | Já incorporado; nenhum novo merge necessário pelo HEAD consultado. |
| `codex/cache-home-perfil` | `9b17d38d` | ancestral | 0 / — | 2 | Código commitado já incorporado; revisar arquivos WIP antes de encerrar esta origem. |
| `codex/integration-180-glass` | `e6885991` | ancestral | 0 / — | 58 | Linha de integração ativa; commits por ownership depois dos testes, WIP não é pacote final. |
| `codex/pr-230-integration` | `66af5817` | ancestral | 0 / — | 0 | Já incorporado; nenhum novo merge necessário pelo HEAD consultado. |
| `codex/fix-addons-158` | `5ca5c825` | divergente | 4 / 0 | 0 | 4 commits só de documentação Social; incorporar os contratos úteis, não confundir nome da branch com fix novo. |
| `feat/android` | `77313de8` | ancestral | 0 / — | 0 | Já incorporado; nenhum novo merge necessário pelo HEAD consultado. |
| `feat/home-dinamica-padrao-hero` | `48a4966e` | ancestral | 0 / — | 11 | Código commitado já incorporado; revisar arquivos WIP antes de encerrar esta origem. |
| `fa4e7bd18996eaa27fd2abef10a98d40651e8f5d` | `fa4e7bd1` | divergente | não comparado | 0 | Análise/mockup histórico; conferir por arquivo, não importar36 commits como feature pronta. |
| `webos3` | `373088d1` | divergente | 5 / 1 | 0 | Compatibilidade/arte/diagnóstico: revisar deltas úteis; porte precisa validação própria, não merge integral. |
| `feat/tizen4-coop` | `99c3bed9` | divergente | 5 / 0 | 0 | Experimento antigo cooperativo; conservar separado, sem substituir caminho TPK40 atual. |
| `feat/vidaa` | `a81639d2` | divergente | 7 / 0 | 0 | Porte experimental separado; preservar patches, matriz própria, #135 não implica release garantida. |

## WIP do checkout principal por responsabilidade

| Família | Comparação necessária |
|---|---|
| Addons/parser/JS/rede | Guardas de limites/corpo/URLs, fontes parciais e buffers versus a base atual e novos plugins/stream-fit; preservar testes de limite |
| Player/ASS/Tizen | MKV/legenda/tracks/HDR/ROI/TLS, libs e manifests versus PR230 e fixes já publicados |
| Conta/progresso/cache | Conta/perfil, syncprog/vistoep/progresso e cache/threads versus #233 e CW novo |
| Home/Settings/Biblioteca/ícones | Unicode, fim de fileira, scrolling, arte e opções sem reintroduzir layout antigo |
| Ferramentas/versões/documentação | Separar experimento/build gerado de runtime; versão de teste não é versão da release |

Não stagear todos os arquivos por conveniência. Cada família termina com origem, diferença efetiva, teste e destino; resíduos gerados são investigados antes de qualquer limpeza.

## Candidatos prioritários e risco

-Plugins2 e908c412:21 patches exclusivos nesta comparação. HTTP genérico deve ser portado em hunks aditivos; manter TLS/headers/final URL/TPK40/Discord atuais. Revisar alocação DOM/JS, fila e código antes de aceitar.
-P2P2 b1d20394:13 patches exclusivos. Teto de disco, cancelamento, escritores/limpeza e toolchains ainda exigem correção/prova; não habilitar pelo sucesso de stub ou teste SKIP.
-Ícones e6010365:5 patches exclusivos. Aliases/splash/menu precisam teste de navegação e conservação de dados; não criar nova assinatura Android.
- Guia Unicode6b0f0659: comportamento já integrado; teste jscadeia passou. Biblioteca2131cb93: hunk de viewport/fade portado à Biblioteca Glass, preservando onda e menu contextual; sintaxe passou, captura integrada pendente.
-Documentação Social5ca5c825:4 commits de decisões do dono; incorporar à implementação canônica, não contar como Social pronto.

## Gates de consolidação

-[ ] Reconciliar os11 WIP de checkouts de desenvolvimento, por arquivo; manter trabalho de outros donos.
-[ ] Registrar cada patch escolhido, equivalente ou substituído no commit que fecha a família.
-[ ] Terminar correções atuais e executar um core build serial da integração.
-[ ] Congelar fontes e usar um manifesto separado para cada pacote de teste/release.

Plano de execução: [PLANO.md](PLANO.md). Nenhuma branch foi apagada/arquivada por este inventário.
