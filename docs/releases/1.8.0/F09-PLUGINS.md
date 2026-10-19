# F09 — Plugins (#134)

Branch `f09-plugins`. Nuvio JS-scraper repositories as one more stream source next to Stremio addons.

## What exists
- `src/pluginjs.c` + vendored QuickJS-ng 0.17.0 (MIT, `src/vendor/quickjs/LICENSE`) and CryptoJS 4.2.0 (MIT, `src/pluginsjs/LICENSE-crypto-js`). `src/htmlq.c` is the cheerio-like bridge.
- HTTP per call through `plugrede.c`: native = N01 `rede_pedir`; `.wgt` (Emscripten) = worker XHR adapter with `xhr.timeout`. Byte/size caps are checked before allocating; global JS+DOM heap budget, global in-flight response budget, fetch queue and timers capped; deadline + cancellation flag/callback/RedeJob generation.
- `src/plugins.c`: repo list per account+profile on disk, manifest/code cache (6 h), generation advanced on profile/account change (late results discarded at hand-off), off by default per device/profile. Sync ACK by revision+generation: removing the last repo uploads an empty list; a 200 with `[]` is a valid empty list, a non-array is not a list.
- `src/sync.c` `sincronizarPlugins` (table `plugins`, RPC `sync_push_plugins`), `perfis.c`/`sync_iniciar`/`sync_esquecer_usuario` hooks.
- Sources sheet: each scraper is an extra origin in `addons.c` (`progEstadoEx`, order `ADD_MAX + k`), provider name = scraper name; runs in parallel with addons, wide search only.
- Settings: option `AJ_PLUGINS` (appended last) opens `TELA_PLUGINS` (`pluginsui.c`, Glass layout in `ajustes_ux_ilha.inc`): toggle, add repo (URL keyboard), per-repo scrapers toggle, remove (OK twice).

## Tests (Mac host)
`tests/plugins_fontes.sh`, `tests/pluginjs.sh`, `tests/htmlq.sh` against `tests/plugins_server.py` (local only). Run normal, `SANITIZE=1` (ASan+UBSan) and `SANITIZE=thread` (TSan): clean. `tests/ajustes_ux_dados.sh`, `ajustes_ux_interacao.sh`, `tools/idiomas.py`, whole-core Mac compile pass.

## Unverified
- No TV, no real server: the `plugins` table / `sync_push_plugins` RPC existence on the backend is assumed from the web client; CloudStream (DEX) repos are kept in the account but never run here.
- Emscripten (`.wgt`) path and Tizen 4/5 / webOS ARM builds were not compiled (no emsdk/cross toolchain here). Code has no `__thread` and uses the NV_TPK40/NV_WEBOS/EMSCRIPTEN guards.
- Real third-party scrapers were not run; only the synthetic ones in the test server.
- The settings screen was not rendered/screenshotted.
