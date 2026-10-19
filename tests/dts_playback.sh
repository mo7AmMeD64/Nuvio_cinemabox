#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
cc -std=c11 -D_GNU_SOURCE -Wall -Wextra -Werror -Isrc tests/dts_playback.c src/dts/dts_playback.c src/js.c src/linguas.c -pthread -lm -Wl,--wrap=clock_gettime -o "$tmp/test"
for scenario in preparefail pause backpressure delayedid cancel seek seeksegment pausedseek pacing ordinal filtered ambiguous audio audioerror eof eoferror pausedreload eofpause overflow subtitles reordered core nocore volume cpu pausedcore controls stallpaced stallrecover stallbound stallnoclock stalltransient stallpause stallseek stallpausedseek stallrecovernoclock stallrecovercancel stallseeknoclock stallrecoverunloaded sustained; do
 "$tmp/test" "$scenario"
done
