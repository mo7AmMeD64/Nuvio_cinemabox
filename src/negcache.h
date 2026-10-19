// Cache NEGATIVO de rede, por endereco (D1 de logs de 22/09 a 05/10: theintrodb
// 404/400 em 130 pessoas, tiffara 400 em 77, cinemeta 404 em 50, themoviedb 404
// em 47 — cada abertura de titulo repetia o pedido que ja se sabia vazio e
// gravava a mesma linha de erro no log).
//
// So vale para uma LISTA FIXA de APIs de metadados (theintrodb, tiffara,
// cinemeta, themoviedb). Addon do usuario, Trakt, Supabase e arte ficam de fora:
// 404 ali pode ser transitorio ou ter outro dono (arte e do tex_cache).
//   - 404/400/410 numa dessas APIs: o endereco e lembrado por 24 h, em disco.
//   - timeout/DNS/conexao: o HOST recua (30 s, dobra a cada falha seguida, teto
//     10 min) e o primeiro sucesso zera. Nada vai para o disco.
// A chave nunca guarda o endereco: so um hash 64 bits (a URL do TMDB leva a
// api_key na query, e ela sai da chave antes do hash).
#ifndef NV_NEGCACHE_H
#define NV_NEGCACHE_H

// 1 = nao pedir (o chamador devolve NULL em silencio). `agora` em segundos.
int  negcache_barra(const char *url, long agora);
// Resultado de um pedido: `curl` = codigo de transporte (0 = respondeu),
// `http` = status (0 se nao houve resposta). Chamar para toda resposta.
void negcache_nota(const char *url, int curl, int http, long agora);

// Persistencia: o app registra dados_ler/dados_gravar_leve no arranque (sem o
// registro o cache vale so na sessao).
void negcache_disco(char *(*ler)(const char *), int (*gravar)(const char *, const char *));
// A carga e preguicosa; a gravacao
// e espacada (no maximo uma por minuto). negcache_gravar forca.
void negcache_gravar(void);

// Testes.
void negcache_zerar(void);
int  negcache_n(void);
int  negcache_serializar(char *dst, int tam);
void negcache_carregar_texto(const char *texto, long agora);
#endif
