> Publication update: v1.7.3 was published on 2026-10-03 with explicit user authorization. See [PUBLICACAO.md](PUBLICACAO.md); the validation limits below remain applicable.

# Pacotes locais 1.7.3

Runtime LG/Samsung: `4ffb8a23abdf80347ba0cf11150957e511ff8ebb`. APK Android atualizado no commit `0d7ae4e67929d1a9bd7eba4dd43b751bb7b001a1`, com enumeração de apps em segundo plano. Todos em `/Volumes/ExternalSSD/nuvio-173-final/`. Nenhum publicado; o APK foi instalado na TCL preservando os dados e seu hash instalado foi conferido.

| Artefato | Bytes | SHA256 |
| --- | ---: | --- |
| Nuvio-1.7.3-NuvioTpk.tpk | 28926290 | `11471ae19ee8a14f55f0b6294da7350a0b73eb00b1bb3b9495984034d47cf40a` |
| Nuvio-1.7.3-NuvioTpk40.tpk | 28930460 | `5de79774935387496ba1ba6f43347d84cddb2103cf750471ba3960bc343aaa16` |
| Nuvio-1.7.3-NuvioTpk60.tpk | 28925380 | `f61d91710de5d67e379367d5cf635e4882ff560214db021e0941a1837abeb7ad` |
| Nuvio-1.7.3-NuvioTpk65.tpk | 28925559 | `a84885b83f414fd2d52f1517598c8ab15e207ee7a76555b9bfbc389752d39529` |
| Nuvio-1.7.3-android.apk | 44629227 | `b24459201e7b85d1e16200da785cc05bf751973eabc718261242ee16fc37552d` |
| NuvioTV-1.7.3-tizen.wgt | 29077672 | `100952bdea8c345166c0631af3fcc27b838afafac62abc7fb41a15c46f4c8471` |
| libnuvio-1.7.3-tpk-arm.so | 9905284 | `a79497bbf8d6c8f64dd66d1938ca1c99c969c9f139caec5e07f5f9e85b185121` |
| libnuvio-1.7.3-tpk40-arm.so | 9909376 | `07b146cf225d7afd189f3cf47753983693be6c28514c8c761bdb00f8290600ee` |
| space.nuvio.native.legacy_1.7.3_arm-highcache.ipk | 53005048 | `353e79fe907e3d224f51bd7e05ad8aba1834d937e8e067f80c65887e0c3a79ba` |
| space.nuvio.native.legacy_1.7.3_arm.ipk | 53005324 | `625f88988ce571fe8bff263b56a59d1f1b59c4c7eef86f8e0673f45947ee379d` |

Geradores: arm.sh --ipk --build (padrão e high-cache), release-samsung.sh e release-android.sh. Gerados também repo.json, webosbrew.manifest.json e SHA256SUMS.

LG: 1.7.3 no appinfo, ELF32 ARM e execução 755 conferidos; pacote exclui arquivos pessoais. Android: assinatura fixa, versão 1.7.3/10703, duas ABIs, seis bibliotecas obrigatórias por ABI, aliases e provider conferidos. Samsung: quatro hosts compilados/empacotados, versão 1.7.3, CRC/duplicatas/arquivos pessoais e igualdade dos anexos .so conferidos. TPK40 sem TLS e com DT_HASH; biblioteca comum mantém TLS. WGT gerado **sem assinatura de dispositivo**, conforme fluxo do empacotador; instalação depende do método/certificado da TV. Os TPK incluem arquivos de assinatura.

TCL atualizada para 1.7.3/10703 com hash do APK exato conferido, inicialização concluída e processo ativo. A consulta de apps completou em 736 ms fora do fio da interface, com 42 apps. Validação manual de fontes/playback e validação física LG/Samsung permanecem pendentes.

[Manifesto completo](/Volumes/ExternalSSD/nuvio-173-final/FINAL-MANIFEST.json) · [Checksums](/Volumes/ExternalSSD/nuvio-173-final/SHA256SUMS). A origem de runtime final inclui a correção visual confirmada; os dez binários foram regerados após ela.
