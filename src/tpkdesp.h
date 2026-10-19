// DESPEDIDA DO .tpk (Samsung nativo): o que o host .NET avisa no log vira o
// despedida.txt que dados_despedida_ler le no arranque seguinte, como o
// NuvioActivity faz no Android.
//
// Sem isto TODA saida pela TV — tecla Exit, desligar, Home com o app morto em
// segundo plano — contava como "anterior caiu": o host chama OnTerminate,
// solta o player, esconde as janelas e arma o _exit sem o main() do app
// chegar a avisos_encerrar. Medido no D1 (1.7.0, 6 h): das 23 quedas do
// tizen-tpk com log da sessao, 19 terminam com "saida: principal escondida",
// XF86Exit ou XF86PowerOff; 3 sao queda de verdade (free() invalid pointer).
//
// "oculto" e escrito quando a janela some, o app pausa ou termina, ou chega
// Exit/PowerOff; e apagado quando a janela volta, o app retoma ou chega outra
// tecla (prova de que seguia em uso). "fim" (saida limpa) nunca e sobrescrito.
#ifndef NV_TPKDESP_H
#define NV_TPKDESP_H

// Caminho do arquivo (dados/despedida.txt). Sem chamar, nada e gravado.
void tpkdesp_iniciar(const char *dirDados);
// Linha que o host mandou para nv_tpk_log (sem o prefixo "[host] ").
void tpkdesp_linha(const char *linha);
// Nome da tecla do NUI (Key.KeyPressedName), so ao apertar.
void tpkdesp_tecla(const char *nome);
// 1 = linha/tecla que esconde, -1 = que mostra de volta, 0 = neutra.
int tpkdesp_classificar_linha(const char *linha);
int tpkdesp_classificar_tecla(const char *nome);

#endif
