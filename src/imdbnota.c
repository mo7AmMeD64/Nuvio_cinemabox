#include "imdbnota.h"
#include "idbase.h"
#include <pthread.h>
#include <string.h>
#define IMDBNOTA_MAX 256
static struct { char identity[96]; int rating; } ratings[IMDBNOTA_MAX];
static unsigned nextRating, ratingCount;
static pthread_mutex_t ratingLock = PTHREAD_MUTEX_INITIALIZER;
static int keyFor(const char *identity, char key[72]) {
  size_t n = idbase_len(identity);
  if (!n || n >= 72) return 0;
  idbase_copiar(identity,key,72);
  return 1;
}
void imdbnota_publicar(const char *identity, int rating) {
  char key[72]; unsigned i;
  if (rating <= 0 || rating > 100 || !keyFor(identity,key)) return;
  pthread_mutex_lock(&ratingLock);
  for (i=0;i<ratingCount;i++) if (!strcmp(ratings[i].identity,key)) break;
  if (i==ratingCount) { i=nextRating++ % IMDBNOTA_MAX; if(ratingCount<IMDBNOTA_MAX) ratingCount++; }
  memcpy(ratings[i].identity,key,strlen(key)+1);ratings[i].rating=rating;
  pthread_mutex_unlock(&ratingLock);
}
int imdbnota_obter(const char *identity, int catalogRating, int series) {
  char base[72], key[96]; unsigned i; int rating=0;
  if (!keyFor(identity,base)) return 0;
  snprintf(key,sizeof key,"%s%s",base,!strncmp(base,"tmdb:",5) ? (series ? "|tv" : "|movie") : "");
  pthread_mutex_lock(&ratingLock);
  for (i=0;i<ratingCount;i++) if (!strcmp(ratings[i].identity,key)) { rating=ratings[i].rating;break; }
  pthread_mutex_unlock(&ratingLock);
  if (rating) return rating;
  return strncmp(key,"tmdb:",5) && catalogRating>0 && catalogRating<=100 ? catalogRating : 0;
}

void imdbnota_alias_tmdb(const char *imdb, long tmdb, int series) {
  char key[96]; unsigned i; int rating;
  if (tmdb<=0 || !idbase_e_imdb(imdb)) return;
  rating=imdbnota_obter(imdb,0,series);
  if (!rating) return;
  snprintf(key,sizeof key,"tmdb:%ld|%s",tmdb,series ? "tv" : "movie");
  pthread_mutex_lock(&ratingLock);
  for(i=0;i<ratingCount;i++) if(!strcmp(ratings[i].identity,key)) break;
  if(i==ratingCount) { i=nextRating++ % IMDBNOTA_MAX; if(ratingCount<IMDBNOTA_MAX) ratingCount++; }
  memcpy(ratings[i].identity,key,strlen(key)+1);ratings[i].rating=rating;
  pthread_mutex_unlock(&ratingLock);
}
