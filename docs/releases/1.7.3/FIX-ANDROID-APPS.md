# Android — installed-app lookup before 1.7.3 release

Runtime commit: `0d7ae4e67929d1a9bd7eba4dd43b751bb7b001a1`.

Opening the source sheet previously called PackageManager synchronously on the SDL update thread. The lookup now runs in one detached worker; repeated requests coalesce. A successful complete snapshot replaces the list under a mutex. Failure preserves the previous snapshot and allows retry; a successful empty response clears it. While the first response is pending, service entries remain informational rather than incorrectly offering the store. JNI String signature is unchanged; Kotlin returns null on query failure instead of conflating failure with an empty successful list.

## Validation

- Gated slow query confirms the production method returns before enumeration completes; 1,000 repeated requests create no extra queries.
- Snapshot preservation, failure/retry, valid empty result and thread-creation failure covered. Normal and ASan/UBSan runs pass.
- Existing Onde ver mapping/parser tests pass on desktop and TPK.
- Clean release build passes actual Android SDK/Kotlin compilation and packages six required libraries for both arm64-v8a and armeabi-v7a; pinned release signer, version and personal-file checks pass.
- Exact final APK installed on TCL over the existing app without clearing data. Installed-file SHA256 matches `b24459201e7b85d1e16200da785cc05bf751973eabc718261242ee16fc37552d`. Version 1.7.3 / 10703, cold activity launch Status ok (2441ms), process remains active.
- On this exact build the installed-app query completed with 42 apps in 736ms, on the worker. No explicit fatal marker appeared in the immediate process log. This is startup/query evidence, not a full remote/playback benchmark.

Another same-version APK appeared during the first device verification and differed from this candidate. It was retained privately for recovery, then the exact fixed APK was reinstalled and checked immediately by hash. Version number alone is not sufficient to identify this test build.

LG/Samsung artifacts retain their previous runtime provenance; the runtime change is Android-specific. Final artifact manifest/checksums were updated and all entries verified. The original APK is retained privately outside Git.

The change removes a known synchronous update-thread lookup. It does not prove this lookup caused all 910ms of the reported stall, does not fix the unresolved Android 11 blank-launch report #223, and does not establish uniform Glass performance gains. No public release was made.
