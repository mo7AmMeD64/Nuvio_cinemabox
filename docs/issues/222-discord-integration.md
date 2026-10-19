# Discord presence integration

The existing #222 feature is integrated into the current Settings layout, under Accounts and services. AJ_DISCORD is appended after the existing options so positional defaults and persisted option indices remain stable. Discord and Seekr build definitions are both retained across Android, ARM, and TPK configuration paths; the Samsung browser build keeps its WebSocket library.

The three new Settings strings have English translations. Portuguese remains Portuguese; other language tables currently use explicit English fallback for these new entries. Existing translations are unchanged.

Checks: Settings data/catalog and interaction fixtures, complete language-table alignment/order/format tests, Android and TPK Settings syntax dispatch, shell syntax, and configured build-macro generation without printing values. These checks do not prove OAuth authorization, account presence, browser OAuth CORS, or physical-TV operation.

Runtime hardening is now integrated in `f515a5cc`. Native sends retain partial
frames in a bounded queue and never wait for writable sockets on the render
thread. Auth/profile callbacks are synchronized and generation-checked;
cancel, unlink and shutdown invalidate old work. Token files use native mode
0600 and packaging rejects their temporary files as well. Public poster
publishing is restricted to query-free TMDB/MetaHub CDN URLs.

The deterministic WebSocket, auth lifecycle and verified-TLS fixtures pass
with ASan/UBSan. The public Gateway test passes HELLO, heartbeat ACK and
invalid-token close on the Mac, without authorizing a user account. The full
core builds locally with all candidate changes. The bundled Mozilla trust
roots and staging guards are documented in `docs/vendor/discord-ca.md`.

Remaining release validation: actual account linking, refresh, playback and
disconnect on LG, Samsung TPK and Android; browser OAuth CORS for WGT; target
latency and package contents. The direct OAuth Gateway/external-assets path
is a compatibility implementation, not a claim of official SDK platform
support. No new release or TV package has been published.
