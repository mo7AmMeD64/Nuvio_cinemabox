# Nuvio 1.7.4

## Português

- **Discord:** vincule sua conta para mostrar o que está assistindo no seu perfil.
- **Detalhes mais leves:** no Apple TV e no Moderno, foque em adicionar para revelar os demais botões. “Assistir do começo” expande ao receber foco.
- **Episódios:** duração ao lado do play, ícone de reassistir nos já vistos e tempo restante após a barra nos episódios em andamento. A duração vem dos metadados disponíveis.
- **Elenco e filmografia:** nomes centralizados, Voltar corrigido e a filmografia permanece visível enquanto os dados do próximo título são buscados.
- **Apple TV:** melhor leitura no carrossel, menu mais legível, temporadas compactas e badge de próximo episódio mais discreta.
- **Home:** a ilha mostra o carregamento das fileiras, abre informações e confirma quando a Home está pronta. Add-ons desativados deixam de bloquear a espera pelos manifestos.
- **Título localizado:** o nome escrito aparece quando falta o logo ou quando a arte está em outro idioma confirmado, sem duplicar o logo durante o carregamento.
- **Player:** miniatura do próximo episódio respeita o blur configurado; avanço evita o aviso de pausa durante a retomada e o botão de créditos fica separado do próximo episódio.
- **Legendas e notas:** romeno `ron` reconhecido, “Todas as legendas” inclui mais idiomas e notas IMDb verificadas são compartilhadas entre as telas.
- **Conta:** corrigido o limite de leitura de 64 catálogos e o tratamento de uma configuração explicitamente vazia de ordem/ocultação.

## English

- **Discord:** link your account to show what you are watching on your profile.
- **Lighter details:** on Apple TV and Modern layouts, focus Add to reveal the other actions. Restart expands on focus.
- **Episodes:** duration beside Play, a replay icon for watched episodes, and time left after the progress bar. Duration uses available metadata.
- **Cast and filmographies:** centered labels, fixed Back handling, and the filmography stays visible while the next title's metadata loads.
- **Apple TV:** improved carousel readability, clearer sidebar, smaller season pills and a more subtle next-episode badge.
- **Home:** the island shows row loading, opens more information and confirms completion. Disabled add-ons no longer block manifest waits.
- **Localized titles:** the written title appears when the logo is missing or confirmed to be in another language, without duplicating it during loading.
- **Playback:** next-episode thumbnails respect the blur setting; seeking avoids the paused overlay while resuming, and credits controls stay clear of the next-episode card.
- **Subtitles and ratings:** Romanian `ron` support, more languages in All subtitles, and shared verified IMDb ratings across screens.
- **Account:** fixed the 64-catalogue parsing limit and explicit empty order/hidden-row configuration handling.

## Packages

| Platform | File |
| --- | --- |
| LG webOS 3+ | `space.nuvio.native.legacy_1.7.4_arm.ipk` |
| LG with more RAM | `space.nuvio.native.legacy_1.7.4_arm-highcache.ipk` |
| Samsung Tizen 4 / 5 / 5.5 | `Nuvio-1.7.4-NuvioTpk40.tpk` |
| Samsung Tizen 6 | `Nuvio-1.7.4-NuvioTpk60.tpk` |
| Samsung Tizen 6.5 / 7 | `Nuvio-1.7.4-NuvioTpk65.tpk` |
| Samsung Tizen 8 / 9 | `Nuvio-1.7.4-NuvioTpk.tpk` |
| Samsung web app, Tizen 5.5+ | `NuvioTV-1.7.4-tizen.wgt` |
| Android TV / Google TV, Android 7+ | `Nuvio-1.7.4-android.apk` |

## Validation boundaries

Discord and next-episode blur were confirmed by the user on Android TV. Other
UI changes have production-source regression tests and host rendering captures;
installation and startup do not prove every physical remote/playback flow.
Final LG/Samsung physical checks remain pending. The account parser fixes are
reproduced with synthetic responses, not the Samsung reporter's account; #233
is not claimed fully resolved. The Android 11 black-screen report #223 remains
open. No universal playback-start latency or FPS improvement is claimed.
New subtitle/audio AutoSync, personal media servers and other 1.8 features are
not included. Newly added card text is Portuguese and English, with English
fallback for the other UI languages.

The broad regression run and targeted reruns retain one pre-existing anime-detail
test failure and skipped account QR/service-dependent checks. See the repository
validation report for the full accounting; the entire suite is not claimed green.
