#ifndef NV_IMDBNOTA_H
#define NV_IMDBNOTA_H
// Session-only verified IMDb ratings, keyed by base title identity.
// Catalog fallback is never used for TMDB-only identities.
int imdbnota_obter(const char *identity, int catalogRating, int series);
void imdbnota_publicar(const char *identity, int rating);
void imdbnota_alias_tmdb(const char *imdb, long tmdb, int series);
#endif
