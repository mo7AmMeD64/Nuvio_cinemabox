# Draft — local 1.7.2, not published

## Added

- Reorganized Settings with local search, clearer categories and confirmation before saving option changes.
- Touch navigation and email sign-in on Android phones and tablets.
- Sources appear as each addon responds, with a configurable wait for automatic selection.
- A smoother clock island transition and quick return to a paused movie for two minutes.

## Fixed

- Resume Android playback at a valid saved position during preparation, avoiding a second seek.
- Keep the LG screensaver inactive during playback, allowing it after pause or stop.
- Keep pending addon changes across restarts and failed sync, without letting an older response overwrite them.
- Clear the resume card and its pending actions when switching accounts or profiles.
- Keep watched history separate for each account and profile; ignore late responses from a previous session.
- Prevent catalog updates from changing a different title after a refresh.
- Keep the phone-input icon available after a Samsung native core update.
- Check native playback sources with the required headers and reject failed HTTP responses.
- Correct an overflow in the subtitle prefetch timing log.

## Notes

- Local test build. Physical playback, remote navigation and performance still need the owner's checks on LG, Samsung and TCL.
- Paused movie retention uses the single video pipeline; Home trailers wait until the movie is released.
- Plugins and P2P remain planned for 1.8.
- Samsung native packages are optional; the WGT remains available.

## Which file do I need?

| TV | System | File |
|---|---|---|
| LG 2016 and newer | webOS 3 or newer | `space.nuvio.native.legacy_1.7.2_arm.ipk` |
| LG with 2 GB RAM or more | webOS 3 or newer | `space.nuvio.native.legacy_1.7.2_arm-highcache.ipk` |
| Samsung 2018–2020 | Tizen 4.0 / 5.0 / 5.5 | `Nuvio-1.7.2-NuvioTpk40.tpk` |
| Samsung 2021 | Tizen 6.0 | `Nuvio-1.7.2-NuvioTpk60.tpk` |
| Samsung 2022–2023 | Tizen 6.5 / 7.0 | `Nuvio-1.7.2-NuvioTpk65.tpk` |
| Samsung 2024 and newer | Tizen 8.0 / 9.0 | `Nuvio-1.7.2-NuvioTpk.tpk` |
| Samsung 2020 and newer | Tizen 5.5 or newer | `NuvioTV-1.7.2-tizen.wgt` |
| Android TV / Google TV | Android 7 or newer | `Nuvio-1.7.2-android.apk` |

The `.so` files, `repo.json`, `webosbrew.manifest.json` and `SHA256SUMS` support self-update, the Homebrew Channel and checksum verification.
