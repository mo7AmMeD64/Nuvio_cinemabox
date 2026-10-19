Faster returns to your movie, more resilient account loading, and fixes for watched history and Live TV.

## Added

- **Type from your phone**: open the phone button beside a text field and scan its QR code on the same network. Available in native builds.
- **Seekr thumbnails**: optional seek previews with your own Seekr key, plus manual timing adjustment.
- **Player island**: leave a movie for Home and return from the clock island, with a shorter, smoother transition.

## Fixed

- **Watched badges** (#212): posters and title details use watched history, including Trakt movie libraries larger than 100 titles. Marking a title watched updates its badge.
- **Continue Watching** (#213): Trakt's next episode is checked before offering it; localized artwork and descriptions are kept between sessions.
- **End time and startup** (#213): long translated end-time labels fit, and startup uses Nuvio artwork.
- **Account outages** (#214, #215): temporary server errors preserve your session and the last successful account data for the current profile. Incomplete replies cannot replace a complete saved library.
- **Live TV on LG** (#158): a second player connection no longer cancels the main Xtream stream.
- **Live TV categories**: channel genres with accents from some add-ons no longer show escape codes (e.g. "Notu00edcias").
- **Artwork and stability**: improved GIF cover decoding, recovery from damaged image caches, concurrent WebP loading, and handling of long catalogue identifiers and incomplete network replies.
- **Crashes**: native Samsung no longer treats leaving with Exit or power off as a crash (which could revert settings); fixed a catalogue race that crashed some Tizen 9 TVs while loading Home; Android TVs that restart the app in the same process relaunch cleanly.
- **Paused session**: the kept session is no longer released on the first frame after leaving.
- **Source lookup**: recently fetched movie and episode sources can be reused briefly when returning to a title. Reload always asks providers again.

## Notes

- Fast return keeps one paused movie session for up to two minutes on LG and Samsung WGT. It falls back to normal loading when the session is unavailable.
- Phone input needs the phone and TV on the same network; browser-based Samsung WGT builds cannot host the QR input service.
- Account server availability and the availability of third-party streams remain outside the app's control.
- Native Samsung packages are optional; you can continue using the WGT version. Sign Samsung packages with your own certificate as before.

## Which file do I need?

| TV | System | File |
|---|---|---|
| LG 2016 and newer | webOS 3 or newer | `space.nuvio.native.legacy_1.7.1_arm.ipk` |
| LG with 2 GB of RAM or more | webOS 3 or newer | `space.nuvio.native.legacy_1.7.1_arm-highcache.ipk` (bigger image cache) |
| Samsung 2018–2020 | Tizen 4.0 / 5.0 / 5.5 | `Nuvio-1.7.1-NuvioTpk40.tpk` (native) |
| Samsung 2021 | Tizen 6.0 | `Nuvio-1.7.1-NuvioTpk60.tpk` (native) |
| Samsung 2022–2023 | Tizen 6.5 / 7.0 | `Nuvio-1.7.1-NuvioTpk65.tpk` (native) |
| Samsung 2024 and newer | Tizen 8.0 / 9.0 | `Nuvio-1.7.1-NuvioTpk.tpk` (native) |
| Samsung 2020 and newer | Tizen 5.5 or newer | `NuvioTV-1.7.1-tizen.wgt` (web version) |
| Android TV / Google TV | Android 7 or newer | `Nuvio-1.7.1-android.apk` |

You do not need `libnuvio-*.so`, `repo.json`, `webosbrew.manifest.json` or `SHA256SUMS`: they support self-update, the Homebrew Channel and checksum verification.
