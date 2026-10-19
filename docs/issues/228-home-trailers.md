# Issue #228: Home trailers on Samsung Q80A

## Evidence

Read-only Worker D1 lookup found record 21018, manually uploaded at
2026-10-03 13:00, matching the issue's reported time and model:
QE65Q80AATXXN, Tizen 6.0, native TPK, Nuvio 1.7.2. The issue has no comments.
The database queries reported zero writes. Raw logs remain private and are
not included in this repository.

The session starts in normal mode after a clean prior exit; safe mode is not
the explanation for this recording. Lines 602-603 show the Home hero and
focused-poster trailer toggles changing from 1 to 0 (0 means enabled).
IMDb trailer sources resolve for five titles visited earlier. No Home
trailer open or current-session native video event appears in the recording.
The log does not record the Home eligibility gates, focused layout, or all
effective settings; it cannot identify which condition prevented playback.

Home trailer scheduling, player integration and source policy are unchanged
between the published 1.7.2 and 1.7.3. Upgrading alone is not a demonstrated
fix. Native TPK follows the native source policy and must not be confused
with the WGT iframe restrictions.

## Confirmed code defect

The Home eligibility check excludes catalog/social rows using the retained
row index even when the hero itself has focus. Returning to the hero by
pointer or D-pad can leave that index referring to a non-content row and
prevent hero autoplay. Row eligibility should apply only when a row actually
has focus. This is a reproducible code defect, but the supplied recording
does not establish that it is the reporter's cause or explain both toggles.

Added transition-only gate diagnostics to identify remaining conditions
without logging media URLs or user/account data. Reasons distinguish top
eligibility, disabled settings/hero, hidden dynamic-layout poster, poster
wait, art transition, non-content rows, missing identity and unsupported
playback. The top gate groups overlays and competing player states; it does
not identify the specific overlay. Effective settings cannot distinguish an
off preference from a dependency or safe-mode override.

## Validation boundary

Existing Home scheduling and source-policy tests passed on the Mac host.
They simulate catalog/network/player edges and do not validate playback on
the Q80A. The existing Home test originally stubbed focused-poster playback
off, so it did not cover that path. Added regressions pass for hero focus
after retained catalog/social rows, actual focused non-content row blocking,
overlay reporting, and 100 unchanged frames without repeated gate logs.
`tests/home-trailer-timer.sh`, `tests/trailer-fonte.sh` and
`tests/trailer-hero-contract.sh` pass. No firmware workaround, physical-TV
success, package publication or issue closure is claimed.
