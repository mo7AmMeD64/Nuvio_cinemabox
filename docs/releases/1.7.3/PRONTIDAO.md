> Publication update: v1.7.3 was published on 2026-10-03 with explicit user authorization. See [PUBLICACAO.md](PUBLICACAO.md); the validation limits below remain applicable.

# 1.7.3 — release review, 2026-10-03

The candidate is packaged and locally tested. This review does not publish or approve a stable release. The current published stable release remains v1.7.2 (2026-10-03 03:26:27 UTC).

## Evidence available

- Final binaries and integrity evidence: [PACOTES.md](PACOTES.md). Android release APK is installed on the TCL; package manager confirms 1.7.3 / 10703 and the activity launched successfully. This establishes installation and startup on that TV, not all manual playback workflows.
- Regression coverage and known exclusions: [REGRESSAO.md](REGRESSAO.md). A subsequent Android patch moves installed-app enumeration off the SDL thread; see [FIX-ANDROID-APPS.md](FIX-ANDROID-APPS.md). It does not establish the root cause of #223 or the entire 995.6ms stall.
- Read-only open issue review: [ISSUES.md](ISSUES.md).
- Recent uploaded diagnostics retrieved from the Worker D1 database: [WORKER-LOGS.md](WORKER-LOGS.md). Raw data is private and outside Git. The SELECT returned rows_written=0, changes=0 and changed_db=false.

## Pending before a stable release claim

| Gate | Current evidence | Missing proof |
| --- | --- | --- |
| Android startup | TCL Android 14 installed and started 1.7.3; one confirmed current diagnostic session | Android 11/Mali-G31 cold-start case from #223 is unresolved; do not claim fixed |
| LG startup and playback | Executable mode and package structure validated | Exact final package launch and representative playback on a physical LG; #211 unproven |
| Samsung startup/player/Home trailer | Native host smoke and source/ROI tests passed | Final package and Home/focused-poster trailer on physical Samsung; #228 unproven |
| Streaming app handoff | Mapping tests and desktop visual checks | Installed-app/store dispatch using a physical TV remote |
| Performance | Current TCL session includes stable 60fps intervals and slower Glass intervals | A current 995.6ms frame was dominated by app update (910.1ms), with cause still unproven. Comparable same-workload measurements and reproduction are pending; no universal performance gain established |

Ratings mismatch #229 remains unproven and should not be listed as fixed. The source-list, original single-window trailer and original LG live-playback issues have human confirmations on previous releases; new addon/subtitle reports must be assessed separately.

No public issue reply, closure, remote push, tag, upload or deployment was performed in this review. Final release notes remain a draft in [NOTAS.md](NOTAS.md).
