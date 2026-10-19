Where to watch in the source picker, cleaner Settings and reliability fixes.

## Added

- Where to watch in the source picker: see regional streaming services such as Netflix and open their installed apps. Store availability depends on the TV.
- Optional app icons using the existing provisional supporter setting.

## Fixed

- Android queries installed streaming apps in the background, preserving the last complete list while refreshing.

- Streaming-service cards keep readable text when selected.
- Cleaner Settings explanations without placeholder pictures; glass outline is easier to find.
- Collections snapshots now handle object responses and stay available after network failures.
- Samsung playback errors retain their diagnostic code.
- Device diagnostics report platform identity consistently.
- Informational subscription channels do not open unrelated apps or stores.

## Notes

Availability comes from TMDB / JustWatch. Opening a service opens its app; it does not promise title-specific playback or subscription access. Streaming-app handoff and final LG/Samsung device validation remain pending. Android installation and startup were checked on a TCL TV; the Android 11 blank-launch report (#223) remains unresolved. Samsung Home autoplay (#228) and rating consistency (#229) are not claimed fixed. No universal performance gain is promised.

New subtitle/audio sync and personal media servers remain planned for 1.8.

## Which file do I need?

| TV | System | File |
| --- | --- | --- |
| LG 2016 and newer | webOS 3 or newer | `space.nuvio.native.legacy_1.7.3_arm.ipk` |
| LG with 2 GB of RAM or more | webOS 3 or newer | `space.nuvio.native.legacy_1.7.3_arm-highcache.ipk` |
| Samsung 2018–2020 | Tizen 4.0 / 5.0 / 5.5 | `Nuvio-1.7.3-NuvioTpk40.tpk` |
| Samsung 2021 | Tizen 6.0 | `Nuvio-1.7.3-NuvioTpk60.tpk` |
| Samsung 2022–2023 | Tizen 6.5 / 7.0 | `Nuvio-1.7.3-NuvioTpk65.tpk` |
| Samsung 2024 and newer | Tizen 8.0 / 9.0 | `Nuvio-1.7.3-NuvioTpk.tpk` |
| Samsung 2020 and newer | Tizen 5.5 or newer | `NuvioTV-1.7.3-tizen.wgt` |
| Android TV / Google TV | Android 7 or newer | `Nuvio-1.7.3-android.apk` |

Samsung native is optional. The WGT uses the TV's installation/signing workflow. Update libraries and Homebrew metadata are generated locally; users do not install those manually.


Validation: 225 complete regression checks passed, with a pre-existing anime-detail failure, one partial account test and one service-dependent skip documented. Additional Android background-query and sanitizer checks passed. Package integrity and update libraries were verified.
