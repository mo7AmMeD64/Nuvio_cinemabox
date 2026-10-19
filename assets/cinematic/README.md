# Cinematic collection heroes

The approved visual reference is the generated three-banner study, not the
flat SVG exports in `assets/editorial`. Preserve the final imagegen pixels:
dimensional forms, textures, engraving, controlled shadows, restrained accents.
Do not redraw these as simplified SVG geometry or stretch them into rectangles.

Scope: Directors, Awards, Streaming, Themes and Genres, 73 identities. Each has
a separate home and collection-detail image. Movie/show backdrops are untouched.

`prompts.json` stores the final prompt set. `provenance.jsonl` records source,
native output resolution and SHA-256 for each delivered image. Production PNGs
live in `deploy/app/art/cinematic`. They are copied unchanged from built-in
imagegen, not upscaled and advertised as 4K.

The loader activates a new identity only when its complete pair exists. The
renderer fits the original aspect, anchored right, within a 500px home header
or 320px detail header. Edge fading is performed in one GPU pass. Artwork must
never extend behind shelves, title text, posters or the item information panel.

```sh
node tests/cinematic-images.mjs --partial # reports actual progress, not completion
node tests/cinematic-images.mjs           # requires every requested image
bash tests/cinematic.sh                  # also tests real loader pair selection
```

Generation is through the built-in image tool. `plan-cinematic-heroes.mjs` plans
prompts only; `record-cinematic-output.mjs` copies verified generator outputs.
Neither script fabricates replacement artwork.
