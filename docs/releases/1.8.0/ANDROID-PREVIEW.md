# Android preview instalada —03/10/2026

Instalação intermediária autorizada pelo dono, executada pelo chat Luna `01a10412-0de7-7480-9bd5-55a436a37c6c`. Não houve release, tag/push ou deploy.

| Evidência | Valor |
|---|---|
| Fonte congelada |38b0efb0dc9b6cc80ef0382d67b094764ace5d43 |
| Parent/tree |e688599116043057f72a83fbee9087cac1ea973d /7893014b4f8dc7d44b420e022a2238815a2d4733 |
| Cópia de build |/Volumes/ExternalSSD/nuvio-180-android-preview |
| Commits locais de empacotamento |098cd245 (versão),701d4113 (comentário socialvis.h) |
| Aparelho / pacote |Smart TV Pro, Android14 /space.nuvio.nativelegacy |
| Versão instalada |1.8.0 /10800, reconfirmada pelo coordenador via dumpsys |
| ABI no aparelho |armeabi-v7a, APK inclui também arm64-v8a |
| Instalação |`adb install -r`, sem desinstalação/clear-data |
| APK SHA256 |b30cec8c6fe5c35736448dc132bf2db7640fe199591c9252240d4964505a4de9 |

O coordenador reconferiu o hash do APK produzido. Luna conferiu certificado esperado, ABIs e ausência de arquivos pessoais; registrou build/instalação/abertura bem-sucedidos e capturas físicas em `build/release-1.8.0/` dessa cópia. Nenhum crash explícito foi encontrado na janela examinada; isso não prova estabilidade prolongada.

Inclui: merge Glass +1.7.4, Agenda mensal, layout Apple logo→botões→informações sem sinopse/linha de elenco, motor AutoSync/documentos ainda sem ativação no player, Seekr pessoal/quota local e correções reproduzidas de trailer Home. Exclui: WIP#158, #233 e stream-fit do momento em que o snapshot foi feito. A sinopse compacta e a consistência entre telas pedidas depois desta instalação continuam pendentes; esta build intermediária não valida U01–U04 do plano.

Continuam necessários os testes com o controle: menu Apple/Moderna, Ajustes90%, fan/restart, episódios/blur/CW, trailers e reprodução/Seekr. Instalação/abertura/foto não comprovam esses fluxos nem LG/Samsung.

A correção701d4113 transforma em comentário uma declaração acidental solta no header Social; foi copiada à integração. Os arquivos de versão da integração continuam1.7.4: o ajuste1.8.0 do preview não muda a release pública.

[Plano de fechamento](PLANO.md). Pacotes e fotos temporárias estão na cópia do SSD; a fonte final receberá outro manifesto.


## Segunda prévia — fonte validada restaurada

Enviada após nova autorização do dono. Commit local de empacotamento `ab62f65c`, originado da fonte congelada `/Volumes/ExternalSSD/nuvio-180-execution-51fxfkxd` (`manifest.json`, 1262 hashes conferidos pelo executor). Mantém a etiqueta de prévia 1.8.0/10800; não é publicação de release.

Inclui agora os fixes #158/#233, restauração visual dos detalhes 1.7.4 conservando fan/episódios, Home/Biblioteca e diagnóstico de memória/persistência/timeout. StreamFit e AutoSync continuam sujeitos aos limites de integração registrados no plano.

O primeiro link Android detectou objeto incremental antigo após copiar fontes com timestamps preservados. Todas as fontes foram tocadas e recompiladas nas duas ABIs, sem mudança de lógica. APK produzido e `base.apk` extraído novamente do aparelho têm o mesmo SHA256: `eafe136e3b1950abce781af1ff3a6a0ce8fb06646956212a8956d89eca27f8ad`.

`adb install -r` passou; versão 1.8.0/10800 e processo ativo reconfirmados pela raiz no Smart TV Pro `192.168.1.128:5555`. A captura física inspecionada mostra o seletor com os perfis existentes; não houve clear-data ou desinstalação. Captura: `/Volumes/ExternalSSD/nuvio-180-android-preview/build/release-1.8.0/android-tv-capture.png`. Isso comprova instalação/abertura, não aprovação visual de detalhes nem teste de reprodução, sincronização ou performance no aparelho.

## Terceira prévia — continuidade, ratings e Home progressiva

Autorizada pelo dono em 03/10. Fonte congelada em `/Volumes/ExternalSSD/nuvio-180-preview-ui-home-final` com 1275 arquivos src/tests e manifesto SHA256. Luna recompilou as duas ABIs, instalou por `adb install -r` preservando dados e extraiu o APK instalado. Coordenador comparou os dois arquivos: SHA256 `8d012314de5b73d60958dec7f06c1f6620abd7842550d8308f66fb46b03216d4`.

Inclui véu contínuo Apple TV, ratings 1.7.4, elenco/recomendações separados (rota TV/filme, rolagem além de sete itens e pointer), publicação progressiva da Home inicial com CW/social, proteção de preferências locais de desempenho, Seekr Settings e API aditiva N01. OSD Seekr e wiring StreamFit começaram DEPOIS do congelamento e não estão neste APK. Mantém rótulo preview 1.8.0/10800, sem release pública.

Captura física `/Volumes/ExternalSSD/nuvio-180-android-preview/build/release-1.8.0/android-tv-home-settled.png` mostra Home com Continuar assistindo, Amigos e For You; coordenador inspecionou. Logs SDL confirmam abertura. Capturas espaçadas não medem startup/latência das fileiras, nem validam reprodução ou transição animada; ainda precisa teste físico comparável.

Commit local de empacotamento: `6fa8e2d4d8abb9c8b28054d720e9729f8bd7ca2d`. O executor informou uso de `monkey -p … 1` para lançamento; por isso a saída do seletor não é atribuída a retomada automática comprovada. Próximas aberturas devem usar componente explícito com `am start`. Não há comparação física de latência nesta rodada.
