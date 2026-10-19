# Nuvio Legacy 2.0

The first time you open it, a short guide walks through what changed. It stays in Settings › About and help › What's new in 2.0.

**Samsung (.tpk): 2.0 is not offered as an in-app update.** It is a big change to the app's .NET part, which the in-app update does not replace, so install the new .tpk by hand once. In-app updates keep working after 2.0.

## Added

- **Glass UI.** Every panel is now an island, like the clock. Glass or solid, 18 accent colors, and art, blurred art or Frost as the background.
- **New look by default.** Everyone starts 2.0 with the accent taken from the art, Logo color and the Frost background. Change it in Settings › Appearance.
- **New logo and startup.** The Classic logo is still there. Three ways to open the app: Default, Fade only, Direct.
- **Screensaver.** Showcase from your catalog, Clock with the next premiere, or just dim. It never starts while something is playing.
- **Home and menu.** Floating rail in Modern, full-height bar in Default. Library, Search (with People) and a monthly Schedule were redone.
- **Title page.** Logo, actions, info and Ratings in one card, a Watch trailer button, friends who watched it, and season charts for series.
- **Player.** Audio and Subtitles open from the corner island. Second subtitle track, subtitle AutoSync, a delay slider, and seeking that speeds up while you hold.
- **Sources.** "Best for this TV" first, groups by quality, filters for MP4 only, Cached and Dubbed. StreamFit measures your network before lowering quality.
- **Social.** Trakt, Simkl and Letterboxd become one person, without duplicated friends. Recommend titles, answer "Already watched", polls on the clock.
- **Profiles.** New Movies background built from what each profile watched, plus Light and Projector. Shows Continue watching for the focused profile.
- **Personal servers.** Jellyfin (login or Quick Connect), Emby and Plex (PIN) as rows on Home, with resume and watched state sent back.
- **Plugins and P2P (experimental, off by default).** Nuvio scraper repositories as an extra source, and a built-in engine for torrents you pick by hand.
- **Settings.** Shorter menu, search, a live preview for each option, one "Show advanced options" switch, and a new keyboard with QR typing from your phone.
- **Arabic.** Letters join and read right to left in titles and subtitles. Samsung TVs now carry an Arabic font, and Windows-1256 subtitles are decoded.
- **Badge packs** for quality and audio in the source sheet and the player.

## Fixed

- Android TV: black screen after "Preparing QR" on sign-in (#223).
- Continue watching: the same episode repeated, and "watched" not removing it (#244). The made-up "14" rating is gone (#243).
- Samsung: account catalogs and their order now follow the account (#233).
- Stalker/Xtream portal entry accepts "/" and "_" (#237).
- "Wait for add-ons" is no longer locked by the depth effect (#238).
- Next episode card stays over the full-screen video in series (#249).
- Seek waits a moment before applying when Seekr thumbnails are on (#235).
- New option to leave Cinemeta out of search (#231).
- Samsung .tpk: subtitles timed from the first frame (#251, thanks KeijoMika).
- Search found only people after the add-on list changed.
- Samsung: Blue and CH+ no longer type an "s" in Search, and the Guide button opens the app's TV Guide instead of leaving the app.
- Home opens faster and the catalog on disk went from 26.9 MB to 1.9 MB.

## Notes

![Nuvio Legacy 2.0](https://raw.githubusercontent.com/iqui27/nuvio-native-legacy/master/docs/releases/2.0.0/banner.jpg)

**Samsung .tpk: install this version by hand.** The in-app update only swaps the app's core library; 2.0 also changes the .NET host and the bundled images and fonts, so it has to be a full install. Later versions update from inside the app again.

**Samsung: also on [Apps2Samsung](https://github.com/Apps2Samsung/tizen-community-packages).** Nuvio is now in Tizen Community Packages, which picks up each release automatically (thanks, Patrick), so you can install it from there too.

| Platform | File |
| --- | --- |
| LG webOS 3+ | `space.nuvio.native.legacy_2.0.0_arm.ipk` |
| LG with more RAM | `space.nuvio.native.legacy_2.0.0_arm-highcache.ipk` |
| Samsung Tizen 4 / 5 / 5.5 | `Nuvio-2.0.0-NuvioTpk40.tpk` |
| Samsung Tizen 6 | `Nuvio-2.0.0-NuvioTpk60.tpk` |
| Samsung Tizen 6.5 / 7 | `Nuvio-2.0.0-NuvioTpk65.tpk` |
| Samsung Tizen 8 / 9 | `Nuvio-2.0.0-NuvioTpk.tpk` |
| Samsung web app, Tizen 5.5+ | `NuvioTV-2.0.0-tizen.wgt` |
| Android TV / Google TV, Android 7+ | `Nuvio-2.0.0-android.apk` |

Tested on an LG C9 and a TCL Android TV. The Samsung builds were not run on a Samsung TV before release: if something breaks there, send the log from Settings › About and help.

Not in this release: Arabic interface and RTL layout (#250, #260), DTS on webOS (#259).
