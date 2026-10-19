#ifndef NV_FICHAMETA_H
#define NV_FICHAMETA_H
// Fichas de meta (titulo, sinopse, duracao, nota, videos[]) persistidas entre
// execucoes, para a primeira volta da descoberta ja ter o texto sem rede.
// 64 vagas de arquivo, chave por hash: o tamanho em disco e limitado por
// construcao (64 x ate 256 KB). Falha de rede nunca entra aqui.
// Devolve um corpo novo (free) ou NULL se nao ha, e outra chave, ou venceu.
char *fichameta_ler(const char *chave, long ttlSeg);
void  fichameta_gravar(const char *chave, const char *corpo);
#endif
