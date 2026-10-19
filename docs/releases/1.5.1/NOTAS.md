Fixes for Settings, subtitles, cast photos and episode descriptions, and more collection GIFs that animate on Samsung.

## Fixed

- **Send logs automatically** (#149): turning it on no longer switches back off after opening Settings. It is on by default and turns back on once after this update; you can switch it off again and it stays off. On Samsung, answering no on the first-run card is still respected.
- **Cast photos** (#153): photos and character names are matched to the actor by name, so they no longer land on the person next to them.
- **Episode descriptions** (#150): they now follow your TMDB language when TMDB has a translation.
- **Long subtitles** (#156): a long single-line subtitle wraps onto more lines instead of ending in "…".
- **Collection GIFs on Samsung** (#141): long GIFs like Apple TV+ were kept still because of their length. The limit now looks at how much work a GIF takes per second, so most of them animate on 2 GB TVs.
- **Samsung**: an error when closing the app is gone.

## Changed

- The log now says why the "Next episode" card didn't show (#151), to find the cause.

## Notes

- Samsung: the package is unsigned on purpose; sign it with your own certificate as before.
- LG with the Homebrew Channel: add `https://github.com/iqui27/nuvio-native-legacy/releases/latest/download/repo.json` as a repository to get updates from the channel.
