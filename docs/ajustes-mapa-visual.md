# Mapa visual dos Ajustes v2

Owner, 04/10: images must use real art and depict the right thing. Every preview fills a 16:9 frame with real art
(account catalogue, or the bundled sample in deploy/app/art before it loads) and the mini-UI is drawn on top in the
island material. No placeholder icon, no stray option icon tile, no floating value label.

Code: `ajVisualCena` (src/ajustes_ux_visual.inc) = category image; `ajPrevia` (src/ajustes_ux_previa.inc) = option
preview. The `case` order of `ajVisualCena` follows the SEC order of src/ajustes_ux_tela.inc (`AJS_*`).

## Category image (index preview, and layer 2 when the category header is focused)

| Category | Image |
|---|---|
| Home screen | the user's Home in miniature (`ajPreviaHome`: layout, hero, rail, clock) |
| Title page | title page: art, logo, meta, rating chips, Play/Trailer/Save buttons (backdrop dimming live) |
| Posters and artwork | poster row with the focused poster (expand, border, depth, corner radius live) |
| Trailers | hero with the trailer playing (logo hidden when "hide logo" is on) |
| Appearance | glass island with "Aa" in the interface font, accent swatches, toggle, clock pill, poster |
| Playback | player: title, 4K / Dolby Vision / Atmos marks (dim when off), timeline |
| Languages and subtitles | interface / audio / subtitle rows with language chips |
| Live TV | channel playing on top, programme guide with now-line below |
| Accounts and services | profiles (real avatars) and Trakt / sync / keys |
| Performance on this TV | FPS meter and memory/image bars |
| About and help | wordmark, version, update button, guide QR |

## Art per category and per submenu

Every category and every submenu (the `ROT` blocks of src/ajustes_ux_tela.inc) has its own backdrop: `AJ_ARTE_SEC`
in src/ajustes_ux_visual.inc (row = category, column = block; column 0 is the category image and also serves the
first block). 34 of the 40 bundled samples are used, none twice (11, 24, 28, 29, 34, 36 are free). A new `ROT`
needs a column there; tests/ajustes_ux_dados.c fails otherwise. With the account catalogue loaded the index picks
title N of the catalogue instead of sample N.

Scene changes go through `ajCenaTroca` (src/ajustes_ux_secoes.inc): the previous scene stays on screen until the
textures of the new one are ready (never the grey placeholder), then the new one fades in over 0.22 s. One layer at
rest, two only during the fade; while the arrow key is held the fade waits for the focus to settle. Scenes of
different heights, and options that share the same image (`ajCenaChave`), switch directly.

## Option preview (layer 2 left column)

| Option(s) | Preview |
|---|---|
| Home layout | three Home miniatures (Modern / Standard / Dynamic) |
| Accent color | `aj2PreviaCor` (live color card + contrast measures) |
| Logo color | `ajPreviaCor` |
| Background | Settings miniature on the chosen background |
| Interface size | four player screens at 100/120/130/150%, current one lit with accent ring |
| Settings size | three Settings strips at 80/90/100% (values in the segmented control below) |
| Interface font | "Aa" + title in the chosen font, list of fonts each in its own face |
| Poster width / corner | posters at the chosen size + slider |
| Continue Watching (all CW rows) | three CW cards in the chosen style, on the category art |
| Max quality, Dolby Vision, Dolby Atmos | Playback image, focused mark dotted |
| Title ratings (IMDb, RT, ...) / MDBList sources | Title page image, rating chips on/off, focused one dotted |
| Interface / audio / subtitle / metadata language | Languages image, matching row highlighted |
| Hide logo during trailer | Trailers image (logo hidden when on) |
| Trakt, Simkl | service card on the category art |
| App icon | launcher icon gallery on the category art |
| Storage, image memory | image-memory panel |
| Logs (view / send / auto) | log tail on the category art |
| everything else | the category scene of its own category, on the art of its submenu |
