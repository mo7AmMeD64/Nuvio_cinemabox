# Regressão ampla 1.7.3

Executada em checkout externo limpo `2ddcc840`; runtime final `4ffb8a23` acrescenta apenas o contraste dos cartões de serviços, validado por capturas PT/EN/vidro e teste da folha. Pacotes foram reconstruídos no runtime final. Correções posteriores de fixtures e documentação não alteram os binários.

O runner avaliou 290 scripts: 219 retornos OK, 9 falhas iniciais e 62 excluídos pela regra do próprio runner. Retorno bruto não é sucesso completo quando um caso foi pulado.

Consolidação após correções e verificações complementares:

| Resultado | Scripts |
| --- | ---: |
| Sucesso completo | 225 |
| Parcial (conta: leitura QR/OpenCV ausente) | 1 |
| Não executado (recomenda_e2e: serviço local indisponível) | 1 |
| Falha preexistente reproduzida na 1.7.2 (detalheanime) | 1 |
| Fora da suíte pelo runner (capturas/diagnósticos específicos) | 62 |

O total de sucesso inclui a prova independente `tpk40_tls.sh` executada pelo empacotador Samsung final: na árvore da suíte ela foi pulada por falta dos binários ARM; no build real passou, sem TLS e com DT_HASH no TPK40, preservando TLS da biblioteca comum.

## Falhas iniciais tratadas

- addonslista/fontecache_vod: fixtures isoladas passaram após stubs de descoberta de provedores, coberta pelo seu próprio teste.
- contaoffline_limites: reset de campo interno estava obsoleto; fixture corrigida e todos os limites/offline passaram.
- salvos_segurar: navegação fixa no Settings já errava na 1.7.2; busca pública do ajuste e acionamento por Enter passaram.
- cachearte-wasm/entrada_texto_wgt/tizen-despedida: dependências Playwright já instaladas disponibilizadas por NODE_PATH; runtimes completos passaram.
- tizen-globalthis: motor Node10.24.1 oficial, em container sem rede, testou o JavaScript real do pacote final. Página/worker falham sem polyfill e funcionam com ele; controle moderno também passou.

A falha detalheanime é a mesma assertion de tipo do item na release/1.7.2 (exit134), sem diferenças nos módulos envolvidos. Não foi alterada a assertion para esconder o resultado. Portanto a suíte não é apresentada como inteiramente verde.

## Evidência e limites

Logs: `/Volumes/ExternalSSD/nuvio-173-regression-suite.log`, `/Volumes/ExternalSSD/nuvio-173-regression-tmp/nuvio-testes.vSRoFG/` e `/Volumes/ExternalSSD/nuvio-173-host-checks/`. Scripts de captura não executados pelo runner têm evidência separada em VALIDACAO.md. Node10/Playwright são dependências de teste, não novas dependências do app.

Nenhum teste deste relatório demonstra controle remoto físico, firmware AVPlay, abertura real de Netflix ou ganho de FPS em TV. LG indisponível e TCL dormindo; instalação e validação física permanecem pendentes.
