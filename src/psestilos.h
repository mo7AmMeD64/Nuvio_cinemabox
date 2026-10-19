// FUNDOS NOVOS DA ESCOLHA DE PERFIL (2.0), desenhados atras dos avatares:
//
//   FILMES    parede inclinada de cartazes; as colunas correm em sentidos
//             opostos e os cartazes sao os do perfil EM FOCO (psparede.h).
//             Trocar o foco troca a parede num cross-fade.
//   LUZ       preto, uma luz difusa na cor do perfil que segue o foco, outra
//             do vizinho no alto, uma linha de horizonte e grao de filme.
//   PROJETOR  sala de cinema antes da sessao: feixe que desce do alto com
//             poeira dentro, pelicula correndo nas bordas, luz que pisca de
//             leve e um risco de filme de vez em quando.
//
// Regras herdadas do mural: nada escurece o centro (o dono tirou o overlay em
// volta dos perfis, 21/09/2026); o pedido de textura e no update e o desenho so
// le GLuint; animacoes reduzidas = o primeiro quadro, parado.
#ifndef NV_PSESTILOS_H
#define NV_PSESTILOS_H

// Indices de V_PS_FUNDO (ajustes.c).
#define PS_FUNDO_FILMES    0
#define PS_FUNDO_LISTRAS   1
#define PS_FUNDO_ARTE      2
#define PS_FUNDO_LUZ       3
#define PS_FUNDO_PROJETOR  4

#define PSEST_PAREDE_MAX 12

typedef struct {
  float luz[6];        // cor do perfil em foco (rgb) e do vizinho (rgb)
  float xFoco;         // centro do avatar em foco, em x de tela
  int   perfil;        // profile_index do foco (0 = nenhum)
  int   semParede;     // perfil com PIN: a parede propria nao aparece
  // Reserva quando o perfil nao tem parede: o mural do catalogo.
  const char *const *mural;
  int   muralN;
  int   reduzida;
  int   parado;        // PIN por cima: o fundo para de andar
} PSCena;

void psestilos_iniciar(void);
void psestilos_atualizar(float dt, const PSCena *c, int modo);
void psestilos_desenhar(const PSCena *c, int modo, float alfa);

#endif
