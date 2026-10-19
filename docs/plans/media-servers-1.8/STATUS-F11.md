# F11 status (Jellyfin slice)

Implemented in src/jellyfin.c/.h, src/jfid.h, tests/jellyfin.c + tests/jellyfin_server.py (fake server).

- Connect: manual URL (reverse-proxy prefix kept), /System/Info/Public check, Quick Connect (POST Initiate, GET Connect) or username/password. Password wiped after the request.
- Home: one fixed row per movie/series library; rows vanish on logout, 401, profile switch.
- Title page: filled from the server item only (no Cinemeta/TMDB/addon call gets a server id); series seasons/episodes.
- Playback: POST PlaybackInfo with a per-backend DeviceProfile; DirectPlay > DirectStream > Transcode, source list shows the method. api_key in the player URL (LG/Samsung cannot attach headers).
- Resume: server UserData percent drives the existing resume seek; always StartTimeTicks 0.
- Check-ins: Playing once, Progress every 10 s, pause/resume immediately, Stopped + transcode release; source switch stops the old session.
- Isolation: ids are jfid.h-tagged; they never reach account progress, Trakt, Discord, social, addons.
- Settings > Integrations > Personal servers (experimental, local, default off). Token in jellyfin-p<N>.txt (0600), excluded from .ipk/.tpk packaging by glob.

Not done / unverified: no real Jellyfin server, no TV, no HLS auth/Range/redirect proof on LG/Samsung, no Emscripten (.wgt) compile (unavailable there by design), transcode seek offset, PGS/ASS server subtitles, Emby, Plex, LAN discovery.
