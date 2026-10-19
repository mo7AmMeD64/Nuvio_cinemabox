Interface fonts, a configurable sidebar, more titles per row, trailers on the focused poster, and fixes from this week's issues.

## Added

- **Interface font** (Settings › Appearance): Inter, Montserrat, Roboto or Atkinson Hyperlegible Next. The subtitle font is still set in the player.
- **Previews in Settings**: each option shows its effect on posters, the title page and the player.
- **Sidebar items** (#162): Explore, TV Guide, Schedule and Profile & Stats can be removed from the sidebar (Settings › Layout › Home content).
- **Items per row** (#163): 12 (default), 18 or 24 titles per Home row. More titles use more memory; on a TV with 1 GB, Home may get slower. A higher value applies the next time the app opens.
- **Trailer on the focused poster** (#124, Settings › Layout › Poster focus): when the focus rests on a title, its trailer plays muted in the hero. Off by default; it needs Expand poster on focus or Landscape posters, as in the web app.

## Fixed

- **LG, live channels** (#158): on some TVs, opening a channel full screen from the TV Guide preview closed the app.
- **Samsung, playback** (#147): the player no longer asks the TV's media service for the position and duration on every frame.
- **Samsung, Sources panel** (#159): quality badges are packaged at the size they are drawn, so they no longer take about 300 ms each to load.
- **Source names** (#144): styled letters used by some formatters (small caps, bold or circled letters) are drawn as regular letters instead of boxes.
- **Hero** (#164): the "Loading artwork" box no longer flashes when the focused title changes.
- **Hero catalogs** (#160): the setting now shows and switches where the hero titles come from. Discover location is greyed out, as the web app's Discover screen does not exist in this app yet.
- **Long synopses** now end with an ellipsis instead of running off the panel.

## Notes

- Samsung: the package is unsigned on purpose; sign it with your own certificate as before.
- LG with the Homebrew Channel: add `https://github.com/iqui27/nuvio-native-legacy/releases/latest/download/repo.json` as a repository to get updates from the channel.
