// EXTRAS DO STREMIO NA BUSCA DE LEGENDAS (#201, #269).
//
// O protocolo de addons manda, junto do pedido de legenda, o que se sabe do
// ARQUIVO que toca: `filename`, `videoSize` e `videoHash` (o hash do
// OpenSubtitles), como um segmento a mais no caminho:
//   <base>/subtitles/<tipo>/<id>/videoHash=..&videoSize=..&filename=...json
// https://github.com/Stremio/stremio-addon-sdk/blob/master/docs/api/requests/defineSubtitlesHandler.md
//
// O app pedia so `<base>/subtitles/<tipo>/<id>.json`. Addon que casa legenda
// pelo arquivo (Stremio Community Subtitles, a familia do OpenSubtitles) ficava
// sem como saber QUAL release tocava: respondia vazio ou com uma legenda
// generica de outro corte. O Nuvio oficial passou a mandar os tres em
// NuvioMedia/NuvioMobile#2102 (mesma ordem e mesma codificacao daqui).
//
// Este modulo e so a regra, sem rede e sem estado, para o teste do Mac.
#ifndef NV_LEGEXTRAS_H
#define NV_LEGEXTRAS_H

#include <stddef.h>
#include <stdint.h>

// "videoHash=..&videoSize=..&filename=..", cada valor em percent-encoding
// (so A-Z a-z 0-9 - . _ ~ ficam como estao; espaco vira %20, UTF-8 vira um
// %XX por byte). Campo vazio/zero fica de fora. Devolve o tamanho escrito, 0
// quando nao ha extra nenhum e -1 quando nao coube (dst vira "").
int legextras_segmento(char *dst, size_t n, const char *arquivo,
                       uint64_t tamanho, const char *hash);

// O pedido inteiro. `seg` vazio ou NULL = o formato antigo, sem extras.
// Devolve 1 quando coube, 0 quando nao (dst vira "").
int legextras_url(char *dst, size_t n, const char *base, const char *tipo,
                  const char *id, const char *seg);

// Hash do OpenSubtitles: tamanho + soma (modulo 2^64) das palavras de 64 bits
// little-endian dos primeiros e dos ultimos 64 KiB. `out` recebe 16 digitos
// hexadecimais minusculos. Devolve 0 quando os trechos nao tem 64 KiB cada ou
// o arquivo e menor que 128 KiB (o hash nao e definido para ele).
#define LEGEXTRAS_BLOCO 65536
int legextras_hash(const unsigned char *ini, size_t nIni,
                   const unsigned char *fim, size_t nFim,
                   uint64_t tamanho, char out[17]);

// Nome do arquivo pelo ULTIMO segmento do caminho da URL, decodificado, so
// quando tem cara de video (.mkv, .mp4...). Serve a fonte que nao manda
// behaviorHints.filename mas cujo link do debrid termina no nome. 0 = nada.
int legextras_nome_da_url(const char *url, char *dst, size_t n);

// Vale medir o hash por Range? So http(s) que nao seja o proprio aparelho: o
// servidor local do P2P e o arquivo baixado nao respondem como o CDN, e ler
// 128 KiB de um torrent que ainda nao tem esses pedacos faria o motor buscar
// o fim do arquivo antes da hora.
int legextras_url_remota(const char *url);

#endif
