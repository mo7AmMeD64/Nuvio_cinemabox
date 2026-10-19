# Validação da candidata 1.7.3

Integração local sobre e1f2936, sem publicação ou instalação em TV.

Passaram: catálogo/interação Settings, política de ícones, coleções e ordem de sincronização, Continuar assistindo (ordem/remoção), fila de texturas, persistência de ajustes por perfil, contrato trailer/hero, diagnóstico Samsung ROI/erro, parser Onde ver com ASan/UBSan, folha de fontes e respostas parciais. Android: processDebugMainManifest e compileDebugKotlin. Hosts C# TPK40/60/65: build Release com .NET 8.0.425.

Os novos textos têm tabela em inglês; idiomas existentes foram sincronizados, usando inglês para entradas ainda sem tradução. Varredura distingue identificadores de apps de textos de interface. Testes i18n conferem ordem, quantidade e formatos.

Compilações de hosts não são pacotes instaláveis nem prova em aparelho. Faltam validação física LG C9, Android TV e Samsung (WGT/TPK), incluindo navegação remota, retorno dos apps e fluidez. Nenhuma taxa de FPS ou latência física é atribuída aos testes no Mac.

Logs de execução locais ficam em /tmp/nuvio-merge-audit; builds C# em /Volumes/ExternalSSD/nuvio-173-host-checks. Os logs remotos foram consultados somente para leitura e estão resumidos em LOGS.md, sem dados pessoais ou URLs privadas.

Capturas de fixture no Mac: [Settings sem placeholder](imagens/settings-sem-placeholder.png) e [Vidro/contorno acessíveis](imagens/aparencia-contorno.png). Estas capturas não são de uma TV.

## Segunda rodada: serviços e pacotes

Netflix e demais serviços estão na aba Onde ver da mesma folha de fontes. Inventário de app instalado abre o ID real; ausente oferece loja; canais Amazon/Apple informativos não dispararam app/loja nos testes. Foram conferidos parser, região, erro offline, catálogo vazio, geração/resposta atrasada e foco durante fontes parciais, com ASan/UBSan.

Pacotes locais efetivamente gerados e conferidos: [artefatos e hashes](PACOTES.md). O resultado da suíte ampla é registrado separadamente após sua conclusão; casos pulados não são validação de plataforma. LG não respondeu ao acesso de diagnóstico; TCL estava dormindo. Nenhuma TV foi acordada ou atualizada.

Referências oficiais conferidas para descoberta/abertura Samsung: [Application API](https://developer.samsung.com/smarttv/develop/api-references/tizen-web-device-api-references/application-api.html) e [ApplicationManager .NET](https://docs.tizen.org/application/dotnet/api/TizenFX/latest/api/Tizen.Applications.ApplicationManager.html). A verificação documental não comprova permissões e respostas do firmware em aparelho.

Captura final de serviços: [Onde ver](imagens/onde-ver.png) e [Onde ver com vidro](imagens/onde-ver-vidro.png). Fixtures offline no Mac, sem logos ou launcher nativo. Revelaram e permitiram corrigir o contraste do cartão selecionado. Nomes e ações conferidos em PT/EN, com foco Netflix/Disney e modo vidro.

Host falso ARM Samsung executou o núcleo final 4ffb8a23 com EGL/GLES llvmpipe: dlopen/config/contexto válidos e 60 trocas de quadro concluídas, com tela de login renderizada. Comando: NV_HOST_LOCALE='' bash tools/tpk-testa.sh 60. Sem a variável, o wrapper antigo tem expansão não definida; definir explicitamente contornou o problema; o wrapper foi corrigido para inicializar esse valor quando ausente. Simulação QEMU/Mesa não mede FPS ou comportamento do firmware Samsung. A captura de login contém pairing temporário e não foi anexada ao repositório.

Resultado final da suíte ampla: [regressão e casos não validados](REGRESSAO.md). Os testes das alterações passaram; a suíte completa mantém uma falha anterior à 1.7.3 e casos não executados, conforme o relatório.

## Subsequent Android installed-app fix

[FIX-ANDROID-APPS.md](FIX-ANDROID-APPS.md) records runtime 0d7ae4e6, additional concurrency/sanitizer tests, clean signed two-ABI rebuild, exact installed APK hash and startup/query evidence on the TCL. These checks supplement the earlier full regression rather than claim that entire suite was rerun on the new commit.
