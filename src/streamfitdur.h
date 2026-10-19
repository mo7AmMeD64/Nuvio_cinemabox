// StreamFit runtime provenance: strict parsing of a REAL metadata runtime.
// Pure functions: no I/O, no catalog, no player. Anything that is not a
// clearly stated duration answers 0 (unknown) — never a presumed "45 min",
// never a season/show total, never the player's default constant.
#ifndef NV_STREAMFITDUR_H
#define NV_STREAMFITDUR_H

// A whole runtime field as metadata publishes it: "142 min", "45min", "1h 52min",
// "2h", "1 h 5 m", ISO "PT1H52M", or bare digits (minutes, the TMDB/Cinemeta
// numeric runtime). Seconds in (60, 86400]; 0 = not a runtime.
double streamfitdur_texto(const char *s);

// CatItem.meta of a MOVIE: "2024 · 142 min". Only a " · " segment that carries
// an explicit unit counts (a bare "2024" is the year). 0 = no runtime.
double streamfitdur_meta_filme(const char *meta);

#endif
