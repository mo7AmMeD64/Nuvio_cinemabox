// O QUE O DONO ENSINOU sobre onde os creditos de uma serie comecam: quando ele
// aperta "Comecar agora" no cartao do proximo episodio, guardamos quanto
// faltava para o fim (duracao - posicao). Um arquivo pequeno, local.
#ifndef NV_CREDAPRENDE_H
#define NV_CREDAPRENDE_H
// Segundos que faltavam para o fim, ou 0 quando nada foi aprendido desta serie.
double cred_aprendido_resto(const char *imdb);
// Chamado quando o dono avanca na mao. `fonteAtual` e a que mandava no cartao:
// com a fonte "aprendido" so ADIANTA o valor (apertou antes do cartao), nunca o
// atrasa — senao cada apertada depois do cartao empurraria o proximo cartao
// para mais perto do fim, episodio apos episodio.
void cred_aprender(const char *imdb, double durSeg, double posSeg, int fonteAtual);
#endif
