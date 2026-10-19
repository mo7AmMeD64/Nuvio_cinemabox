# PR #230 review

Reviewed 2026-10-03 at head `e0596c7178e58cbe3eb54108901fddf0b5779770`.
Recommendation: address the two reproduced subtitle cases before merging.

## P1: a pending choice overwrites the latest manual choice

In `src/video_tpk.c:549-550`, an immediate selection after the deadline does
not clear `legPend`. Queue subtitle 1, start playback, wait 2600 ms, select
subtitle 0, then pump. The host receives 0 followed by the stale 1; the UI
still reports 0. Clear or replace the pending choice whenever a newer choice
is submitted, including the immediate path. Add this ordering to the test.

Reproduced with the PR's production backend and fake-host fixture in an
external temporary directory; independently reproduced by a second reviewer.
The original fixture passes. The adversarial latest-choice assertion fails.

## P2: default-track cues remain visible during and after the delay

The deferred path sets `legAtual` immediately, while `video_legenda_nativa`
still accepts host cue text. After queuing English track 1 and starting
playback, a default Russian-track cue is returned to the renderer before any
subtitle selection reaches the host. Dispatching the pending selection also
does not clear that cue, so it can remain until its old expiration.

Reproduced by sending a fake-host default cue after the first playback tick:
the host had received zero subtitle selections, yet `video_legenda_nativa`
returned the default cue. Suppress cues while selection is pending and clear
old text when dispatching the chosen track. Host callback attribution after
dispatch remains a separate platform limitation.

## Other observations

- The AVPlay state restriction cited in the new C# comment is from a different
  API. This host uses `Tizen.Multimedia.Player`; TizenFX API9 `Selected`
  explicitly permits Ready, Playing and Paused. The measured hardware timing
  issue may still justify deferral, but the comment should describe that
  measurement rather than claim the AVPlay restriction applies here.
  Source: https://github.com/Samsung/TizenFX/blob/API9/src/Tizen.Multimedia.MediaPlayer/Player/PlayerTrackInfo.cs
- Reconnection resets pending state and restores selected tracks; subtitle
  off cancels pending selection. No additional user-visible defect was proven
  in those paths, late audio enumeration, or closing playback.
- The original fake-host fixture verifies scheduling, not actual demuxer
  selection or the universal sufficiency of a 2.5-second delay. Physical-TV
  success in the PR body is author-reported evidence, not independently
  repeated in this review. Only a GitGuardian check was listed on the PR.

No PR edits, merge, public comment, package installation or release was made.
