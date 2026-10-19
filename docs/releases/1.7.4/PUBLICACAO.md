# 1.7.4 publication

Published as the normal latest release on 2026-10-03:
https://github.com/iqui27/nuvio-native-legacy/releases/tag/v1.7.4

Runtime commit and annotated tag: `1eee19b64329768891f6776c47bb87a714eb8b3b`.
The unpublished draft was replaced with the final modal-rendering fix before
publication. No public 1.7.3 artifact was changed.

All 14 uploaded asset sizes and GitHub SHA-256 digests match the local final
files. SHA256SUMS, repo.json and webosbrew.manifest.json were downloaded back
and compared byte-for-byte. The final artifact identities are in PACOTES.json.
Both webOS packages have the correct app version and executable ELF mode, and
package gates excluded personal files. Samsung gates checked all four manifest
versions, signatures, update-library consistency and the Tizen 4/5 no-TLS ELF.
Android signing certificate and both ABIs passed the release gate.

The Android APK was installed over the existing app, preserving data. Its
installed hash matched the published artifact. Cold activity start took 2296 ms;
this is not the latency from selecting a movie to playback. The physical TV
showed the final highlights modal, including complete episode and Home previews.
Final physical LG/Samsung playback checks remain pending.

The serial suite and follow-ups retain the documented pre-existing anime-detail
failure and skipped cases. See VALIDACAO.md for the full regression accounting.
Issue reports remain open where reporter/platform confirmation is needed.

## Issue replies

- [#158](https://github.com/iqui27/nuvio-native-legacy/issues/158#issuecomment-5972591678)
- [#222](https://github.com/iqui27/nuvio-native-legacy/issues/222#issuecomment-5972591891)
- [#223](https://github.com/iqui27/nuvio-native-legacy/issues/223#issuecomment-5972592161)
- [#226](https://github.com/iqui27/nuvio-native-legacy/issues/226#issuecomment-5972592442)
- [#227](https://github.com/iqui27/nuvio-native-legacy/issues/227#issuecomment-5972592656)
- [#228](https://github.com/iqui27/nuvio-native-legacy/issues/228#issuecomment-5972592916)
- [#229](https://github.com/iqui27/nuvio-native-legacy/issues/229#issuecomment-5972593177)
- [#231](https://github.com/iqui27/nuvio-native-legacy/issues/231#issuecomment-5972593444)
- [#232](https://github.com/iqui27/nuvio-native-legacy/issues/232#issuecomment-5972593734)
- [#233](https://github.com/iqui27/nuvio-native-legacy/issues/233#issuecomment-5972594025)
