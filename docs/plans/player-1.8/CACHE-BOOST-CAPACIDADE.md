# F07 — cache de seek em disco e volume até 200%: capacidade por backend

2026-10-04, branch `f07-cache-boost`. Complementa a seção "Cache de seek e
volume" do [README](README.md) e a linha F07 do
[PLANO](../../releases/1.8.0/PLANO.md).

## Matriz

| Backend | Cache de seek em disco | Volume acima de 100% | O que a UI mostra |
|---|---|---|---|
| Android (Media3) | **Implementado**: `SimpleCache` + `CacheDataSource` por sessão | **Implementado**: `AudioProcessor` de ganho no `DefaultAudioSink` | Opção 256/512/1024 MB; linha de volume 0–200% |
| LG webOS (uMS) | Não há: o app só entrega a URL ao pipeline | Não há: o app não vê PCM; `setVolume` do uMS vai só até 100 | "Não disponível nesta TV" nas duas |
| Samsung .wgt (AVPlay) | Não há (ver abaixo) | Não há | "Não disponível nesta TV" nas duas |
| Samsung .tpk (AVPlay nativo) | Não há | Não há | "Não disponível nesta TV" nas duas |

## Android: o que foi feito

- **Cache** (`CacheMidia.kt`, `CacheSessao.kt`): `CacheDataSource` fica ACIMA da
  cadeia existente (`DefaultDataSource` → cache → `ParaleloDataSource.Factory`
  com cabeçalhos de autenticação, Range, redirecionamentos e as 4 conexões).
  Lacunas vão ao upstream com o mesmo `DataSpec` (posição/tamanho = Range);
  trechos já baixados voltam do disco. Acertos de cache não passam pelo
  `ParaleloDataSource`, então o medidor passivo do StreamFit continua contando
  só bytes de socket.
- Só VOD HTTP progressivo. HLS/DASH (`.m3u8`/`.mpd` ou o que o
  `ParaleloDataSource.serve` recusa) ficam fora (`NAO_SE_APLICA`); canais ao
  vivo nunca armam o cache (player.c manda 0). Trailers não armam: tocam sem
  cache e a 100%.
- Uma pasta por sessão em `<cacheDir>/nv-seek/<boot>/<sessão>`, sem banco de
  dados (índice dentro da pasta), LRU no limite efetivo. Fechar o player ou
  trocar a fonte libera e apaga a pasta fora do fio principal (1,5 s depois,
  para os loaders cancelados terminarem). No arranque, toda pasta de outro
  `boot` (processo que morreu) é apagada; links simbólicos não são seguidos.
- `statvfs` (`File.usableSpace`) antes de abrir: o maior tamanho ≤ pedido que
  deixa 512 MB livres; espaço desconhecido = desligado. Não coube nada →
  `POUCO_ESPACO` e aviso curto na ilha.
- ENOSPC/erro de escrita no meio: o sink protegido (`CacheSessao.Escrita`,
  com `CacheDataSink` sem buffer interno) desliga o cache da sessão, comita só
  o prefixo confirmado, e o vídeo continua (`DISCO_CHEIO` + aviso uma vez).
- Diagnóstico: "Cache de seek" com usado/limite (ou o motivo) no painel Grande
  do medidor de desempenho, atualizado a cada 2 s.
- **Volume** (`GanhoAudioProcessor.kt`, `GanhoMath.kt`): 0–100% é o
  `ExoPlayer.volume`; 100–200% é ganho real no PCM (200% = +6,02 dB), com
  limitador suave (tanh acima de −1 dBFS), linear abaixo dele, intocado a
  100%. Passthrough/bitstream não passa por processadores: a linha
  fica limitada a 100% com o motivo; o passthrough nunca é desligado em
  silêncio. Escolhido em vez de `LoudnessEnhancer` porque vale para toda saída
  PCM, inclusive o decodificador FFmpeg, não depende de efeito de sessão da
  TV e a conta é testável.
- Convivência com F06 (sincronia por áudio): o sink é
  `AudioSyncSink(DefaultAudioSink[GanhoAudioProcessor])`. A escuta do F06 lê
  `handleBuffer` na ENTRADA do sink (PCM decodificado com o tempo de mídia,
  antes de qualquer processamento), então a sincronia analisa o sinal sem
  reforço nem limitador; o ganho do F07 roda na cadeia de processadores, no
  que vai para a saída. A detecção PCM × bitstream é uma só: o formato de
  entrada que o `AudioSyncSink.configure` relata por `nativeAudioEstado`
  alimenta o `audsync` e também o estado do reforço (`cacheboost`). Offload de
  PCM não é ligado neste player; se um dia for, o reforço precisa considerar.
- Volume é da sessão: volta a 100% a cada título aberto; troca de fonte e
  reconexão reaplicam o valor da sessão.

## LG e Samsung: por que não

- **Buffer interno do AVPlay não é cache de disco.** `setBufferingParam`
  controla quanto o player segura em memória antes/depois de pausas; não há
  API para o app guardar os bytes em arquivo nem para o player ler de um
  arquivo parcial durante o download. Não prova cache de seek.
- **uMS (webOS)** recebe a URL e baixa por conta própria; o app não vê os
  bytes de mídia nem o PCM. Oferecer cache exigiria um proxy local (o app
  servindo `http://127.0.0.1` com Range a partir de um arquivo próprio), que
  não está provado em nenhuma das TVs (permissão de socket local no .wgt não
  existe; LG/TPK precisariam de prova de throughput e de Range sob o pipeline).
- **Ganho**: sem acesso ao PCM decodificado, só restaria subir o volume físico
  da TV, o que o plano proíbe ("não elevar o volume físico da TV para simular
  ganho do player").
- Portanto as duas opções aparecem como "Não disponível nesta TV" nesses
  backends (decisão de compilação: `NV_ANDROID`), em vez de um controle que
  não faz nada.

## O que precisa de TV (não provado nesta fatia)

- Build do APK e reprodução real: seek dentro do trecho em cache mais rápido
  que fora dele, consumo de disco/CPU, comportamento com remux 4K no heap de
  192 MB da TCL (o cache é disco; o teto do buffer em RAM não mudou).
- ENOSPC real num armazenamento cheio; limpeza após matar o app.
- Volume acima de 100% audível e sem clipping em áudio PCM real, e o estado de
  passthrough com receptor AC3/E-AC3 (o relato vem do formato de entrada do sink, via F06).
