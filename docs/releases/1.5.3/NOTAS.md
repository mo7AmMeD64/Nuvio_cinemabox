Fixes for the TV Guide, Continue Watching and subtitles, and animated collection covers on Samsung.

## Added

- **Provider EPG in the TV Guide** (#158): with an Xtream account, the guide also loads your provider's own programme listing and matches channels by their EPG id, so channels outside Brazil, Portugal and Latin America get a schedule. On Samsung this needs an https panel.
- **Animated WebP covers on Samsung** (#141): collection covers delivered as animated WebP now animate like GIF covers.

## Fixed

- **Next episode** (#151): switching sources inside the player no longer loses the episode list, so the Next episode card shows again.
- **Continue Watching** (#151): when Trakt is unavailable, the row keeps its titles instead of going empty.
- **Subtitle encodings**: subtitles saved as Windows-1252 (common for Portuguese, Spanish and French), Windows-1251 (Cyrillic) or UTF-16 now show accents and letters instead of boxes.

## Notes

- Samsung: the package is unsigned on purpose; sign it with your own certificate as before.
- LG with the Homebrew Channel: add `https://github.com/iqui27/nuvio-native-legacy/releases/latest/download/repo.json` as a repository to get updates from the channel.
