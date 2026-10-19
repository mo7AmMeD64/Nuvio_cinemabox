# 1.7.4 validation

Runtime changes use the shared C implementation. Production detail tests cover
Add action mapping, Modern focus/retract, restart, Back in filmography and
preserving an open detail page while switching titles. GL fixtures cover episode
replay/duration/remaining state, centered cast, smaller seasons, and grouped
hero actions. The new highlights card has keyboard, one-time marker and rendered
preview checks. Translation tables remain aligned across 28 secondary languages.

Catalogue tests reproduce the 64-entry truncation and explicit empty state
failure, then pass after the fix, including ASan/UBSan. Episode metadata tests
cover real runtime, missing/null runtime and preserving existing duration.

The initial Samsung package gate rejected Discord compiler TLS in the Tizen
4/5 library. That target now uses pthread keys with a destructor for per-thread
snapshots. Both normal and NV_TPK40 Discord lifecycle tests pass, including
ASan/UBSan. Final package verification passed: no PT_TLS, TLS relocations or
__tls_get_addr in that library; the modern Tizen library retains its own TLS.

Broad regression is tracked separately; early isolated discovery fixtures needed
SDL clock stubs after Home loading metrics were added. The fixtures were repaired
and rerun. Account tests require configured server values and retain the QR
reader skip where OpenCV is unavailable. The pre-existing anime-detail type
assertion from 1.7.3 remains a recorded failure, not a hidden pass.

Physical Android: user confirmed Discord and next-episode blur. Candidate APK
installation/hash and cold launch are verified separately. Synthetic captures do
not prove the physical remote flow or Samsung/LG playback. #223 remains open and
#233 is not claimed fully reproduced against the reporter's account.

## Final regression accounting

The serial broad run returned 225 passing scripts, 12 failures and 63 scripts
excluded by the runner. Targeted reruns resolved 11 failures: clock stubs in
isolated fixtures, initialized counters when disabled manifests are skipped,
translation alignment, configured account test values, the Node 10 prerequisite,
and selecting the correct subtitle sidecar for libass comparison. This yields
236 OK script returns and one retained anime-detail failure; it is not a wholly
green suite. Account QR recognition remains skipped without OpenCV, and the
recommendation end-to-end test retains its service-dependent skip.

The anime assertion is the same pre-existing type failure documented in the
1.7.3 regression report. It was not weakened or relabeled as a pass.

The subtitle comparison initially selected an arbitrary completed track (10
instead of 41 events). It now selects the sidecar for the fixture URL explicitly.
An overlapping-build rerun missed two timing thresholds; a subsequent rerun
passed with the index at 2422 ms and first dialogue at 3643 ms under the simulated
1.2 s Range delay. All MKV/ASS cases and the final libass comparison passed: 41
events, zero Start difference and no invalid duration. These are host fixtures,
not a new promise about physical playback-start latency.

## Final Android candidate

Runtime commit: `1eee19b64329768891f6776c47bb87a714eb8b3b`.
APK SHA-256: `6492dcd22ca03b7507ffd2d4e3c867d4daa3a5c4a412dc2a31e96042ae59a6d9`.
The signed APK was installed over the existing app with data retained; readback
of the installed base APK matched. Cold activity launch: 2296 ms. The highlights
card was captured on the physical Android TV with complete text in the episode
and Home previews. An earlier capture showed intermittent missing text while
Home/trailers rendered underneath. The modal now draws against an opaque
backdrop without rendering Home, and the physical captures no longer show that
problem. Host preview and keyboard/dismiss tests also passed.

LG standard/high-cache IPKs and all Samsung TPK/WGT/update-library packages are
rebuilt from the same runtime commit. Their contents, versions, executable mode,
certificate/library consistency and SHA-256 values are checked separately.
No final physical LG or Samsung playback validation is claimed.
