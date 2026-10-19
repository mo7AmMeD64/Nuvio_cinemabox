// Jellyfin item identity, header-only so modules and tests that never link
// jellyfin.c (catalogo.c, discord.c, player.c tests) can apply the same rule.
//
// A personal-server item is NOT an IMDb/TMDB title. Its id is opaque, belongs
// to one server and one user, and must never collide with addon ids, reach the
// Nuvio account progress, Trakt, the social feed or Discord. The catalogue id is
//   "jf:<8 hex of the server id>.<32 hex item id>"
// One ':' only, so idbase.h treats the whole string as the base id (no fake
// season/episode suffix), and 44 bytes fit CatItem.imdb[64] and alvoId[64].
// The 8-hex server tag is checked against the connected server before any
// request: an id from another server (or a previous connection) is refused,
// never resolved against the wrong one.
#ifndef NV_JFID_H
#define NV_JFID_H
#include <stddef.h>
#include <string.h>

#define JFID_PREFIXO "jf:"
#define JFID_TAG 8
#define JFID_ITEM 32
#define JFID_MAX (3 + JFID_TAG + 1 + JFID_ITEM + 1)

static inline int jfid_hex(char c) {
  return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
}

// Namespaces: "jf:" Jellyfin, "em:" Emby, "px:" Plex. Same shape for all three
// (8 hex of the server id, '.', the opaque item id), so the same privacy gate
// and the same parser serve every personal server.
#define JFID_PREFIXO_EMBY "em:"
#define JFID_PREFIXO_PLEX "px:"

// 1 for any id in a personal-server namespace, even malformed: callers use
// this as a privacy gate ("never send to Trakt"), so prefix alone decides.
static inline int jfid_e(const char *id) {
  if (!id || !id[0] || !id[1] || id[2] != ':') return 0;
  return (id[0] == 'j' && id[1] == 'f') || (id[0] == 'e' && id[1] == 'm') ||
         (id[0] == 'p' && id[1] == 'x');
}

// Lowercase hex without dashes, at most `max` digits. 1 if valid and not empty.
static inline int jfid_hex_limpo(const char *in, char *out, size_t max) {
  size_t n = 0;
  if (!in || !*in) return 0;
  for (; *in; in++) {
    char c = *in;
    if (c == '-') continue;
    if (c >= 'A' && c <= 'F') c = (char)(c - 'A' + 'a');
    if (!jfid_hex(c) || n >= max) return 0;
    out[n++] = c;
  }
  out[n] = 0;
  return n > 0;
}

// Builds the catalogue id with an explicit 3-char prefix ("jf:", "em:", "px:").
// 0 when either part is not hex or does not fit.
static inline int jfid_montar_p(const char *prefixo, char *dst, size_t tam,
                                const char *servidorId, const char *itemId) {
  char srv[64], item[JFID_ITEM + 1];
  if (!dst || tam < JFID_MAX) return 0;
  if (!jfid_hex_limpo(servidorId, srv, sizeof srv - 1) || strlen(srv) < JFID_TAG) return 0;
  if (!jfid_hex_limpo(itemId, item, JFID_ITEM)) return 0;
  srv[JFID_TAG] = 0;
  memcpy(dst, prefixo, 3);
  memcpy(dst + 3, srv, JFID_TAG);
  dst[3 + JFID_TAG] = '.';
  memcpy(dst + 4 + JFID_TAG, item, strlen(item) + 1);
  return 1;
}
static inline int jfid_montar(char *dst, size_t tam, const char *servidorId,
                              const char *itemId) {
  return jfid_montar_p(JFID_PREFIXO, dst, tam, servidorId, itemId);
}

// Splits a catalogue id. Trailing ":season:episode" is refused: Jellyfin
// episodes have their own item id, so a suffix means someone built the id by
// hand and it does not name a server object.
static inline int jfid_partes_p(const char *prefixo, const char *id, char tag[JFID_TAG + 1],
                                char item[JFID_ITEM + 1]) {
  size_t i, n;
  if (!id || memcmp(id, prefixo, 3)) return 0;
  id += 3;
  for (i = 0; i < JFID_TAG; i++) if (!jfid_hex(id[i])) return 0;
  if (id[JFID_TAG] != '.') return 0;
  n = strlen(id + JFID_TAG + 1);
  if (!n || n > JFID_ITEM) return 0;
  for (i = 0; i < n; i++) if (!jfid_hex(id[JFID_TAG + 1 + i])) return 0;
  memcpy(tag, id, JFID_TAG); tag[JFID_TAG] = 0;
  memcpy(item, id + JFID_TAG + 1, n + 1);
  return 1;
}
static inline int jfid_partes(const char *id, char tag[JFID_TAG + 1],
                              char item[JFID_ITEM + 1]) {
  return jfid_partes_p(JFID_PREFIXO, id, tag, item);
}

#endif
