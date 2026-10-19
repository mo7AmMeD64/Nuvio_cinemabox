// Provedor CinemaBox embutido (cinema.albox.co).
//
// O app nao tem mais addons de terceiros: a lista de addons nasce com UMA
// entrada interna, base "cinemabox://cb", e todo pedido dela e respondido AQUI,
// antes de a rede entrar no circuito (rede.c). Os caminhos sao os do protocolo
// Stremio (/manifest.json, /catalog/<tipo>/<id>.json, /meta/<tipo>/<id>.json,
// /stream/<tipo>/<id>.json), entao descoberta, busca, detalhe e fontes
// continuam funcionando sem saber que a fonte e interna.
//
// Ids: o titulo e "cb:<id do show>" e cada episodio "cb:<id do player>" — dois
// segmentos, que e o formato que idbase.h aceita para id que nao e do IMDb.
#ifndef NV_CINEMABOX_H
#define NV_CINEMABOX_H

// Base do unico addon do app (semeado em addons.c, respondido em rede.c).
#define CINEMABOX_BASE "cinemabox://cb"

// Responde um pedido "cinemabox://". *nosso recebe 1 quando a URL e deste
// provedor (mesmo sem resposta): a rede NAO deve tentar baixa-la. Devolve corpo
// em memoria nova (free), ou NULL quando o pedido falhou. *nosso=0: nao e nosso,
// siga o caminho normal.
char *cinemabox_responder(const char *url, int *nosso);

#endif
