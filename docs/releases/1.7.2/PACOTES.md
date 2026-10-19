# Pacotes locais 1.7.2

**Inclusão posterior:** screensaver webOS `8a49956` integrado por `857f83e3` e ajustado em `11835ffa`. Código atual `ab281e99`; o lote abaixo de `b71471bc` não inclui o fix. Ele permanece tecnicamente conferido, mas é intermediário para a candidata ampliada. Novo empacotamento pendente após congelamento.

Conjunto técnico anterior concluído, sem publicação ou instalação nesta rodada. Todos os pacotes derivam da fonte congelada `b71471bc20e36806de9665145e4898c5e8e67884`, incluindo a fila durável de addons e a correção de identidade do cartão da ilha. Desenvolvimento em `/private/tmp/nv-172`, branch `release/1.7.2`; eventual commit posterior só de documentação não altera a origem compilada.

## Conjunto atual

Pasta de anexos: `/Volumes/ExternalSSD/nuvio-rel-172/final`. São 12 arquivos de produto/metadados e `SHA256SUMS`, totalizando 13 anexos. Compilação, conferência e os 12 checksums passaram. Manifesto de evidência fora da pasta de anexos: `/Volumes/ExternalSSD/nuvio-rel-172/FINAL-MANIFEST.json`.

- Android: versão 1.7.2/10702, certificado fixo esperado, duas ABIs com seis bibliotecas cada; configuração efetiva conferida apenas como presença e nenhum arquivo pessoal/snapshot/temporário.
- LG normal/highcache: versão 1.7.2 no manifesto/control, ARM32/libass e configuração efetiva presentes; marcador de cache ampliado somente no highcache. Metadados Homebrew conferem hash/tamanho do IPK normal.
- Samsung: WGT e quatro TPKs 1.7.2; dois núcleos anexos idênticos aos respectivos núcleos empacotados. Tizen 4/5 sem PT_TLS/relocações TLS, com DT_HASH e sem dependência drminfo; proveniência da configuração/WASM e compatibilidade Chrome69 do WGT conferidas pelo script. Listagem completa dos ZIPs sem arquivos pessoais/snapshots/temporários. WGT distribuído sem assinatura; quem instala o assina com o certificado da própria TV.
- Cópias de build Android/LG/Samsung no SSD externo, limpas e na mesma fonte. Nenhuma TV foi controlada para gerar estas evidências.

Relatórios locais sem valores de configuração:

- `/Volumes/ExternalSSD/nv172-b71471bc-android-verification.json`
- `/Volumes/ExternalSSD/nv172-b71471bc-lg-verification.json`
- `/Volumes/ExternalSSD/nv172-b71471bc-samsung-verification.json`

Logs: `/Volumes/ExternalSSD/nv172-b71471bc-android-build.log`, `/Volumes/ExternalSSD/nv172-b71471bc-lg-normal-build.log`, `/Volumes/ExternalSSD/nv172-b71471bc-lg-highcache-build.log` e `/Volumes/ExternalSSD/nuvio-rel-172/build-samsung-b71471bc.log`. A conferência consolidada foi executada por `verify-final-b71471bc.py`, ao lado do manifesto.

| Anexo | Bytes | SHA256 |
|---|---:|---|
| `Nuvio-1.7.2-NuvioTpk.tpk` | 27489770 | `d648e295306db06adea2508231bd9820063227e2aa8dc9bdeabd862814a422a4` |
| `Nuvio-1.7.2-NuvioTpk40.tpk` | 27494227 | `667a0c290b3943ccb24fb8a24c558e1ecf3a9d72f40cf546021c9612f33b1d1d` |
| `Nuvio-1.7.2-NuvioTpk60.tpk` | 27488809 | `b0834d29d166713a3847071ad6e30f3d654f219dd75f86167187b878afa42dee` |
| `Nuvio-1.7.2-NuvioTpk65.tpk` | 27488999 | `6c4242e32091ae6af8b6cedc4d0dc8f453e9d7a3a259277276626a7874787182` |
| `Nuvio-1.7.2-android.apk` | 43558895 | `f876c7dd44418d224826905c9a46148424f200281e59b6add08c8d0c33c4740f` |
| `NuvioTV-1.7.2-tizen.wgt` | 27097119 | `b760399947a36804e85e7e9b3c0f8bf361eea885328c219e8667e315931621bc` |
| `libnuvio-1.7.2-tpk-arm.so` | 9863240 | `a448c376baa8fef0d3dd3bb0e6590126e43609783daf35da492530a0658944c8` |
| `libnuvio-1.7.2-tpk40-arm.so` | 9867320 | `9e4f1d7d46d23bc13c49fd3c83ba1fdd7c3be1abda6f7b49324106a84a0de834` |
| `repo.json` | 1310 | `f2236e5e040791ded28a6e3aaaa8465c66ebaa963a08dc9a405c0db17686becf` |
| `space.nuvio.native.legacy_1.7.2_arm-highcache.ipk` | 51056670 | `44836dfc39b459b51903bb46cce350357ff6e585d1bbdf1f7bf8f56a156b894a` |
| `space.nuvio.native.legacy_1.7.2_arm.ipk` | 51056254 | `edcb17a9393f85363e4ee14b5b1e45c5a0062a923da7ec1ce3f09c6d434b42a4` |
| `webosbrew.manifest.json` | 633 | `d56446fa0e1fc45dcbf596ec97b19a28b4ac1ce5be83cbad4715a67e9e699813` |

O décimo terceiro anexo é `SHA256SUMS`; `shasum -a 256 -c SHA256SUMS` aprovou os 12 arquivos. Logs, relatórios e configurações não são anexos públicos.

## Conjuntos intermediários preservados

- `intermediaria-32e70e48/`: primeira candidata conferida, anterior à fila durável entre processos.
- `intermediaria-fc711f6b/`: 13 anexos conferidos, com fila durável; substituídos após a revisão da ilha encontrar isolamento incompleto entre conta/perfil. Manifesto, hashes e evidências próprios estão nessa pasta.

Nenhum conjunto intermediário deve substituir a candidata `b71471bc` nos testes finais.

## Temporários e espaço

A primeira compilação Samsung produziu os TPKs, mas a extração de conferência falhou por disco cheio; essa tentativa não foi aprovada. No Mac, mktemp sem template ignorou TMPDIR mesmo em bash filho. Templates explícitos sob TMPDIR corrigiram Android/ARM/TPK/conferência Samsung (`fc711f6b`, com ajuste ARM já integrado). A regressão de caminho no SSD passou.

O guard standalone TPK recebeu `06d915f6`: grep -q encerrava cedo e SIGPIPE podia mascarar arquivo pessoal sob pipefail. Fixture com 12 mil entradas reproduziu retorno141; consumo completo da lista aprova o caso limpo e rejeita o contaminado. A candidata atual já inclui essa ferramenta; todos os ZIPs Samsung também foram conferidos pela leitura completa da listagem.

Builds antigos da worktree publicada/mesclada nv-166b foram preservados em `/Volumes/ExternalSSD/nuvio-build-170-preservado`; ass-wasm permaneceu na origem para conservar referências. Builds descartáveis1.7.1/abrirrapido removidos conforme handoff. Gradle, novas builds e temporários estão no SSD. Cache dos dez pacotes publicados1.7.1 transferido com SHA256 para `/Volumes/ExternalSSD/nuvio-rel-171-cache`; caminhos originais continuam acessíveis por links válidos.

## Gates restantes

Conferência técnica de pacote não comprova controle remoto, reprodução, GPU, HDR nem latência nas TVs. Dono precisa validar C9/TCL/Samsung e os casos de VALIDACAO-LOCAL.md. A correção da ilha passou em fixture ASan/UBSan e revisão independente; propostas de UX do mockup continuam separadas e não foram implementadas nesses pacotes. Issues dependentes de fonte/configuração/aparelho precisam de reprodução específica. Nenhum push/tag/publicação/comentário autorizado nesta rodada.
