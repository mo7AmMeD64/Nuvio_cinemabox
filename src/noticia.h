// O TEXTO DE UMA NOTICIA, para o modal de noticia da Agenda.
//
// noticias.h da a LISTA (manchete, veiculo, data, link do Google News); este
// modulo vai do link ate a pagina do veiculo e tira dela o texto e a capa
// (leitura.h). Um fio, uma noticia por vez, e a fila so guarda o que foi pedido
// e ainda nao saiu — o modal aberto e a linha em foco no painel de manchetes.
//
// O CAMINHO, fora da Samsung:
//   1. link do Google News -> URL do veiculo (leitura.h: formato antigo em
//      base64, ou a pagina do artigo + POST batchexecute);
//   2. a pagina do veiculo, cortada em 512 KB, 10 s de prazo;
//   3. o extrator, com a URL final como base para a imagem.
// NA SAMSUNG (WASM) o navegador barra os tres pedidos por CORS — o mesmo motivo
// de /v1/noticias. La vai UM pedido ao worker, /v1/noticia?u=<link>, que faz o
// mesmo caminho do lado do servidor com as mesmas regras e devolve JSON; a
// imagem volta ja apontando para /v1/noticia/img, que repassa so image/*.
//
// FALHA NAO E ERRO DE TELA: sem URL resolvida, pagina bloqueada (403/202 de
// anti-robo, medido no IMDb e no MacMagazine em 29/09/2026) ou HTML sem texto,
// o estado vira NTC_FALHOU e o modal mostra a manchete, o veiculo e o QR
// "Abrir no celular" — o que da para fazer honestamente sem o texto.
#ifndef NV_NOTICIA_H
#define NV_NOTICIA_H
#include "leitura.h"

enum { NTC_NADA = 0, NTC_BUSCANDO, NTC_PRONTA, NTC_FALHOU };

typedef struct {
  int  estado;          // NTC_*
  char url[700];        // URL do veiculo; "" quando nao deu para resolver
  Leitura l;            // valida em NTC_PRONTA (em NTC_FALHOU pode ter so metas)
} NoticiaTexto;

// Pede o texto de `link` (o <link> do item do RSS). Repetir e gratis: o que
// esta na fila ou pronto nao e pedido de novo. FIO PRINCIPAL.
void noticia_pedir(const char *link);

// O estado atual de `link`. Em NTC_PRONTA/NTC_FALHOU devolve o registro (o
// ponteiro vale ate o proximo noticia_pedir); nos outros, NULL e *estado diz
// qual. FIO PRINCIPAL.
const NoticiaTexto *noticia_texto(const char *link, int *estado);

// SO PARA TESTE: troca a rede. `corpo` NULL = GET; senao, POST com esse corpo.
typedef char *(*NtcBaixar)(const char *url, const char *corpo, int *status);
void noticia_rede_teste(NtcBaixar f);

#endif
