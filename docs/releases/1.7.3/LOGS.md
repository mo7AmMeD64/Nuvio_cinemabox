# Logs consultados para a 1.7.3

Consulta D1 remota somente leitura, SELECT de 300 uploads recentes identificados 1.7.1 ou 1.7.2. Evidência de retorno: rows_written=0, changes=0, changed_db=false. Corpos brutos privados fora do Git; relatório não inclui pessoas, tokens, URLs de fontes ou endpoints pessoais.

O recorte contém 76 uploads identificados 1.7.2: 20 webOS, 17 tizen-tpk, 39 Android. Recebimento entre 2026-10-03 01:53:36 e 04:53:20 UTC; IDs da consulta inteira 19441–19958. Parte antecede a publicação estável às 03:26:27 UTC. Rótulo de versão não comprova pacote publicado/commit instalado. Corpos são distintos, mas vários repetem trechos de uma mesma reprodução: não contar uploads como sessões ou incidentes independentes.

## Achados

- Nenhum padrão explícito FATAL EXCEPTION, Fatal signal, Segmentation fault, RuntimeError ou Aborted no recorte 1.7.2. Isso não comprova ausência de crash nem estabilidade geral.
- Falhas de origem/addons (408, 4xx, 5xx), Trakt 401 e ausência de dados/arte 404. HTTP 404 de introdução/Seekr pode significar conteúdo não disponível; não tratar automaticamente como defeito do cliente. URLs não expostas neste documento.
- Samsung registra erros de player 0xfe6c0031, 0xffffffff, 0xfe6c0026 e 0xfe6c002e. Em contexto de um upload, a abertura falha e o Guia já tenta a fonte seguinte, mas o diagnóstico lê erro vazio. A candidata passa a conservar o código sem atribuir causa não demonstrada.
- Atrasos de arte na Home e taxas de quadros baixas em alguns trechos. Tempos extremos podem atravessar player/suspensão; contadores de despejo não são OOM. O cache atual já contém prioridade, proteção da tela e orçamento por aparelho; não alterar limites automaticamente com base só nessa amostra.
- Seekr respondeu tanto 200 com cues quanto 404 sem prévia. Quotas por TV, chave pessoal e política de fallback seguem o escopo 1.8.

## Validação necessária

Reproduzir erros Samsung com fonte/codec e identidade do host confirmados; verificar expiração Trakt com sessão consentida; comparar latência de arte e quadros em aparelhos sob a mesma carga. Dados insuficientes para atribuir todas as falhas ao Nuvio ou prometer ganho de performance nesta candidata.
