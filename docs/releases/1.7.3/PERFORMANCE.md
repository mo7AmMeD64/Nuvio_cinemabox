# Desempenho por plataforma após a integração 1.7.3

A integração funcional está no commit 99198697. Esta rodada posterior separa correções de trabalho redundante de resultados físicos ainda não medidos.

| Plataforma | Ação e evidência local | Limite |
| --- | --- | --- |
| Android TV | Aplicar janela compara os parâmetros reais da SurfaceView antes de alocar LayoutParams ou pedir layout. Mil repetições idênticas não criam novas alocações, atribuições ou recriações; resize, troca de superfície e falha seguida de retry são testados em JVM. | Ainda requer teste TCL/Android TV com compositor e decoder reais. |
| LG webOS | Deduplicação ACB existente preservada e exercitada pelo caminho C real: 1001 pedidos iguais causam uma chamada nativa simulada; transições PiP/tela cheia continuam aplicadas. Escala 1080p/4K e pausa/screensaver também passam. | Contagem de chamadas simuladas não mede FPS físico. |
| Samsung WGT | Preservado cache de metadados AVPlay: 1000 leituras repetidas do menu não repetem getTotalTrackInfo. Mantidos dedup de retângulo e retry após recusa existentes. Contrato AVPlay passa. | Não reduzir retries ou mudar heap sem medição no firmware. |
| Samsung TPK | Recuperação deliberada reaplica ROI porque algumas TVs ignoram pedidos aceitos. Deduplicar só por ausência de exceção quebraria essa proteção; manter comportamento e medir custo agregado. | Redução efetiva de chamadas não aplicada; necessita protocolo de confirmação/force e teste físico. |

Android ganha identidade própria nos perfis e no relatório (platform=android). Mantém os orçamentos conservadores anteriores, tetos por RAM, ajustes manuais e regra de rollback; nenhuma alteração automática agressiva de memória/threads foi introduzida.

## Validação física pendente

Usar mesma versão/fontes e cinco repetições por cenário: Home fria/quente, navegação contínua entre fileiras, abrir/fechar player, PiP/tela cheia, legendas e seek. Registrar modelo/firmware, RAM, build e perfil; comparar p50/p95 de quadro, quadros acima de 33/50 ms, abertura do player, arte pronta, uso de memória e erros. Só promover ajuste automático se o reteste demonstrar ganho e não piorar estabilidade.

Modelos físicos LG, Android e Samsung não estavam validados nesta rodada. Não há promessa de ganho de FPS ou latência de reprodução baseada no host.

Validação posterior: teste JVM Android, suite Diagnóstico e dispatch LG/WGT/TPK/Android, caminhos reais C de pausa/janela LG, contrato AVPlay WGT e métricas TPK passaram. Hosts TPK40/60/65 recompilados sem erros; TPK60/65 mantêm avisos de SDK/target existentes. Métricas TPK agregam requests/repeated/applied/failed e tempo nativo por sessão, sem log ou alocação extra por pedido; 10.000 pedidos explícitos continuam aplicados.
