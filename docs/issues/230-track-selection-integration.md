# PR #230 integration

Integrates PR head `e0596c7178e58cbe3eb54108901fddf0b5779770` with the
current master while retaining numeric host-error diagnostics.

Before integration, production-backend fake-host tests reproduced two
subtitle defects beyond the PR's original scheduling test:

- An immediate new selection after the settle deadline was overwritten by
  the old deferred selection on the next pump.
- Default-track text was visible while the intended selection was pending,
  and old cached text remained at dispatch.

Every new choice now cancels the previous pending choice. A mutex protects
cue suppression and cached text across the host callback and app threads.
Pending/off selections suppress callbacks; dispatch clears old text, and
stop/new session reset the pending state. Host calls run outside the mutex.
New diagnostics are in English. Comments describe measured firmware timing,
not the unrelated web AVPlay contract.

Validation: selection scheduling/adversarial lifecycle tests, normal and
canary TPK ROI tests, library routing, reconnection policy, MKV track mapping
and whitespace checks pass on the Mac host. The TPK40 TLS binary gate was
skipped because compiled TPK libraries are absent in this checkout; no TLS
code changes or TV-package builds were performed.

The 2.5-second delay remains a hardware workaround based on the PR author's
reported test. These host tests do not prove the demuxer switched on every
Samsung model. Cue callbacks do not identify their track: callbacks arriving
after dispatch cannot be attributed independently, and the host's queued
selection does not acknowledge demuxer completion. Physical-TV retesting
remains necessary; this integration does not publish a new release.
