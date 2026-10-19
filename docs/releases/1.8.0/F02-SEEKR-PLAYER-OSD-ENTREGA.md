# F02 Seekr player OSD — stable host delivery

Integration source: `/Users/hrocha/.codex/worktrees/integration-180-glass/nuvio-native-legacy`, 2026-10-03.

During scrubbing, the real player now renders a compact material pill for blocked previews: installation quota, provider quota, rejected key, missing key, unavailable network, storage failure, invalid clock, or title without preview. Local quota reports 50/day in UTC and the actual reset in local time; provider retry remains a separate cause. Clock rollback suppresses misleading wall-clock promises. Fixed translated labels and numeric usage/time only enter the OSD; no response body, credential or URL is rendered. There are no new toasts/logs or dispatches.

The pill only appears while scrubbing, never from release smoothing alone or when the local option is disabled. Existing ready/loading sprite drawing is retained. A separate `ajustes_seekr_habilitado()` getter exposes the local option without requiring a key; `ajustes_seekr_ligado()` keeps its effective-key contract for dispatch, preview layout and other callers. This fixes missing-key status being swallowed by the old combined guard.

Exact files from this task:

- `src/player.c`: `seekrStatusLinhas`, `seekrStatus`, narrow `seekrMiniatura` state/option guard.
- `src/ajustes.c`: only new getter and delegation in existing getter.
- `src/ajustes.h`: only declaration of new getter.
- New `tests/player_seekr_status.c`, `.sh`, `tests/player_seekr_status_shot.c`, `.sh`.

Other dirty/staged hunks in these files belong to prior/root work and are preserved. No stage/commit/install occurred.

## Verification

`bash tests/player_seekr_status.sh` PASS: actual helper + actual Seekr labels/local-time formatter; distinct local/provider causes, UTC count, local reset/retry, clock rollback, storage degradation, no-key and unknown enum, no raw secret-bearing text.

`bash tests/player_seekr_status_shot.sh /tmp/nuvio-f02-player-status` PASS: actual full player GL, eight fixed causes × glass/solid (16 captures); exact one Seekr material per direct positive render, no material after release or disabled. Fixture intercepts requests and supplies dummy capture-key hook; no provider I/O or quota use. Input visibility resets between cases to avoid fixture auto-hide. Missing-key case actually clears the key hook. Initial invalid fixture attempts were corrected before this final PASS.

`git diff --check` PASS. Logs: `/tmp/nuvio-180-validation/f02-player-status.log`, `/tmp/nuvio-180-validation/f02-player-status-gl.log`.

Inspected PNGs: `/tmp/nuvio-f02-player-status/seekr-local-vidro.png`, `seekr-storage-solido.png`, `seekr-no-key-vidro.png`: clear readable pill, above seek bar, with three-line local quota and one-line storage/no-key causes. Provider/key/network/clock/empty captures share the same constrained geometry.

Host evidence only. The existing frozen TV snapshot predates this work; device font/render/timing behavior and WGT provider header exposure still require their respective device/integration proof. This fixture does not test HTTP response classification; that is the already-tested Seekr/network layer.

M3 write attempted via local attribution helper; unavailable tunnel returned connection refused. No memory save claimed.
