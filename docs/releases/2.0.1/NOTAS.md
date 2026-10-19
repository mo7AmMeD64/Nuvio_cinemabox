# Nuvio Legacy 2.0.1

A big follow-up to 2.0: fixes, a faster Android TV, and a few things people asked for.

## Added

- **Control Center.** Hold CH+ and the clock island opens into quick settings. Add or remove shortcuts right there.
- **New Settings.** Big category cards, an always readable Frost background, and a search that suggests close matches.
- **Playback speed** from 0.75x to 2x in the Audio sheet. Not available while audio goes to a receiver (passthrough).
- **Pick the exact subtitle.** When a language has several versions, OK opens the list so you choose the one you want.
- **Dismiss notices.** Island cards have Dismiss, and in Notices you can hold OK. Dismissed ones stay gone.
- **Watched and saved titles sync with your profile.** Marks made offline are kept and sent later, and each profile has its own saved list.
- **DTS on LG** (from #259, thanks to sandoval): on webOS 5 and newer, DTS audio the TV refuses is converted to stereo on the TV. The player shows a DTS badge and tells you if the audio is native, converted or not playable.
- **File size range** for automatic playback: Settings › Sources and add-ons › Maximum size and Minimum size.
- **Find people** and **Recommend to a friend**, redesigned.
- **Arabic** as a metadata language, and a language filter in Change artwork.
- Torrent sources show their seed count. A more compact header in the sources list.
- New app icon. A new What's New, with optional ways to support the project.

## Fixed

- **Crashes on LG and Android.** The app could close by itself when Home refreshed, most often while changing Settings.
- **Android TV is much smoother.** Home, profiles, Explore and the title page now hold 60 fps on a TCL that ran at 36 to 45.
- **Dolby Vision and HDR on TCL Android TVs** no longer start dark until you change the aspect ratio.
- **Add-ons behave like official Nuvio.** Slow add-ons get more time, setup kept in the link is no longer dropped, and subtitle add-ons like Subtitle Sync have time to answer.
- **Subtitle add-ons** receive the video's file name, size and hash. More language codes are recognized (#269).
- **Collections.** Accounts with more than 256 folders lost their last collections (#255).
- **Long add-on links** (Comet and others) are no longer cut.
- **Corrupted small images** on some Android TVs.
- **Samsung .tpk:** embedded subtitles inside MKV files are read by the app itself, so they show on TVs where the Samsung player stayed silent (#269). Embedded tracks show language names, and audio shows codec and channels.
- **Trailers on the hero and focused posters** now start on accounts with a single profile (#228). Samsung .tpk trailers re-apply their volume after starting (#281).
- **Arabic** text reads right to left with joined letters.
- **Next episode card** no longer opens in the middle of an episode when a source reports the wrong length.
- **Subtitle AutoSync** handles frame-rate differences, cuts, MP4 files and the second subtitle.
- **LG:** visual effects lower themselves on slower TVs.
- The automatic pick no longer chooses add-on info lines like "support the project".

## Notes

| Platform | File |
| --- | --- |
| LG webOS 3+ | `space.nuvio.native.legacy_2.0.1_arm.ipk` |
| LG with more RAM | `space.nuvio.native.legacy_2.0.1_arm-highcache.ipk` |
| Samsung Tizen 4 / 5 / 5.5 | `Nuvio-2.0.1-NuvioTpk40.tpk` |
| Samsung Tizen 6 | `Nuvio-2.0.1-NuvioTpk60.tpk` |
| Samsung Tizen 6.5 / 7 | `Nuvio-2.0.1-NuvioTpk65.tpk` |
| Samsung Tizen 8 / 9 | `Nuvio-2.0.1-NuvioTpk.tpk` |
| Samsung web app, Tizen 5.5+ | `NuvioTV-2.0.1-tizen.wgt` |
| Android TV / Google TV, Android 7+ | `Nuvio-2.0.1-android.apk` |

**Samsung .tpk:** coming from 2.0, it updates from inside the app. The in-app update only replaces the app's code, not its bundled images, so the new icon and the Ko-fi badge only show after you install the new .tpk by hand, and the trailer volume fix (#281) is complete only with a manual install. Everything else works with the in-app update.

Tested on an LG C9 and a TCL Android TV; the Samsung builds were not run on a Samsung TV, and the DTS conversion was not tested on a webOS 5+ TV. If something breaks, send the log code from Settings › About and help.
