# Protótipo de ajustes do Nuvio

Protótipo local para revisar navegação com controle remoto antes de implementar
no app nativo. Recorte: **Trailers, Idiomas e legendas, Desempenho desta TV**,
com 18 opções, busca e comparação com os padrões da demonstração.

## Abrir

Nesta pasta, execute:

```bash
python3 -m http.server 4179 --bind 127.0.0.1
```

Abra [o protótipo local](http://127.0.0.1:4179).
É necessário servir por HTTP porque a página usa módulos JavaScript.
Não requer build, backend ou instalação de dependências para usar.

## Experimentar

- Setas percorrem seções e opções. Direita entra na seção; esquerda volta.
- Enter representa OK: abre a folha e confirma a escolha em foco.
- Escape volta/cancela. O ✓ continua marcando o valor salvo enquanto você navega.
- Clique e Tab também funcionam; diálogos mantêm o foco dentro de seus controles.
- Busque “memória” para encontrar uma opção avançada oculta. Voltar recupera a consulta.
- Desligue o trailer do topo e abra seu ajuste de som para conferir a dependência.
- Em “Diferente do padrão”, abra um ajuste e use “Restaurar padrão”.
- Em Desempenho, experimente menos efeitos: Manter confirma; Escape, Voltar ou
  expiração após 15 segundos descartam o lote. Falha de aplicação tem prazo de
  5 segundos, separado da janela humana. Ocultar a aba também cancela o teste.

## Escopo e limites

Dados, padrões e perfil são exemplos. A arte e a fonte Inter vêm dos assets
já presentes neste repositório. Todas as requisições são ao servidor local.
As preferências confirmadas usam somente a chave de localStorage
`nuvio-ajustes-ux-demo-v1`; Reiniciar demonstração restaura apenas essa chave.
O protótipo não chama APIs, não sincroniza conta e não altera arquivos do app.

As imagens ilustram os efeitos, sem reproduzir trailers nem medir memória,
GPU, resolução real de saída ou desempenho de uma TV. Trocar o idioma da
interface muda a amostra; a interface do estudo permanece em português.
A capacidade por plataforma é uma etapa posterior. Cancelamento e recarga
foram verificados no navegador; isso não valida persistência ou comportamento
no app nativo ou no controle físico de uma TV.

## Verificações

```bash
node --test model.test.mjs
node --test browser.test.mjs
```

A suíte de navegador requer Playwright resolvível pelo Node, Google Chrome
instalado e o servidor acima ativo. Se o Playwright estiver fora deste projeto,
use `NODE_PATH` apontando para a pasta `node_modules` que o contém.
`PROTOTYPE_URL` permite trocar o endereço do servidor de testes.

Na entrega de 02/10/2026:

- 10 testes do modelo: rascunho, confirmação, cancelamento, validação, busca,
  idiomas independentes e recomendação limitada às preferências locais.
- 10 testes de navegador: navegação, salvamento/reabertura, busca com retorno,
  dependência, restauração e saídas do experimento, inclusive falha de aplicação.
- Inspeção visual em 1440×900, 1280×720 e 390×844; sem rolagem horizontal da
  página. Em telas estreitas, a ficha explicativa fica na folha de edição.

Arquivos: `index.html` e `app.css` compõem a apresentação; `app.js` cuida do
foco, diálogos e experiência; `model.mjs` guarda opções e transações em memória.
Veja também a [proposta revisada](../../ajustes-ux.md).
