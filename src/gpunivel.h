// NIVEL DE GPU do .tpk da Samsung: o que a GPU da TV aguenta desenhar.
//
// POR QUE EXISTE (registro D1 8825, Tizen 5.0 de 2019, 1108 MB, "Mali-TDVX |
// OpenGL ES 3.2 r11p0", 1920x1080): tela vazia a 60 fps; com a home cheia,
// 22-29 fps e ~45 trancos por janela. O pior quadro reparte assim:
//   [quadro] pior=68ms | ev=0 bomb=0 upd=0.2 clr=62.0 des=5.1 swap=0.8
// A CPU submete o quadro em 2-5 ms e o tempo vai todo no glClear: e o primeiro
// toque GL no buffer seguinte, onde o driver espera a GPU liberar um buffer da
// janela. HIPOTESE, nao prova: a GPU nao termina o quadro anterior a tempo
// (preenchimento/ALU por pixel). O mesmo app na Tizen 6 roda a 60 fps.
//
// THREE EFFECT LEVELS, independent of the interface resolution:
//   0 = como sempre (1080p nativo, efeitos cheios).
//   1 = EFEITOS LEVES (gfx_definir_efeitos_leves): dither Bayer barato no lugar do highp dos
//       degrades e sem os realces decorativos (brilho no alto do card, luz de
//       canto, a luz do chao da escolha de perfil). One Kawase blur pass
//       instead of two, retaining frosted glass. Native resolution.
//   2 = EFEITOS MINIMOS (gfx_definir_efeitos_minimos): nivel 1 + sem a luz de
//       tela cheia do tema imersivo e sem sombra/halo. Existing flat-glass
//       rule remains. Tried if level 1 is still GPU-bound below 45 fps.
//   720p is selected separately: any effects level, never automatic.
//   App upscaling uses SGSR 1 or sharp bilinear; weak GPUs use linear.
//
// COMO O NIVEL E ESCOLHIDO (.tpk, Android e LG webOS):
//   Live FPS reports govern eligible Home/Settings continuously, revalidating
//   saved levels: three reports below panel refresh minus 5 FPS reduce one
//   effects tier; ten near refresh restore one, with failed-retry hysteresis.
//   Loading/mixed scenes and >1-second stalls do not count. The per-frame
//   comparison below remains the fallback until the first eligible report.
//   - -DNV_TPK_NIVEL_FORCADO=N (tools/tpk.sh com NV_TPK_NIVEL=N): fixo, sem
//     medir. E o canario de comparacao; a release nao usa.
//   - senao ADAPTATIVO: in full Home or Settings, measures FPS and how much of the
//     quadro e ESPERA (clr+swap) contra CPU (ev+bomb+upd+des). FPS < 45 com a
//     espera dominando = GPU presa: desce UM nivel, espera assentar, mede de
//     novo. So conserva a reducao com ganho >=15% E >=5 fps; caso contrario
//     returns to the previous level and blocks retries only for this workload
//     in this session. Scene/layout/glass/color changes start a fresh baseline.
//     Compara antes de aceitar FPS bom ou CPU dominante no candidato.
//     gpu-nivel.txt mantem versao=1/nivel para leitores antigos e acrescenta
//     avaliacao=1, bloqueado e fps0/1/2. Nivel legado >0 e comparado uma vez
//     com o 0 na mesma sessao. Candidato so e gravado depois da comparacao.
//     Chave diferente (firmware novo, outra TV) = recomeca do 0.
//   - Sem nada gravado: comeca no 0, inclusive em GPU fraca conhecida.
//     Tres descartes >1 s na tentativa, mesmo intercalados, com Home cheia e
//     espera de GPU dominante: GPU fraca fica no 2 so nesta sessao, sem gravar.
//     Janela completa ou perda dessas condicoes zera a contagem.
//   - Changing the measured scene, losing Home artwork, or suspending cancels
//     the candidate and returns
//     ao nivel anterior; a proxima comparacao mede uma referencia nova e
//     preserva a reavaliacao pendente do legado ate concluir a comparacao.
//   - a chave ganha "4k" com superficie acima de 1080p: o nivel aprendido em
//     4K (4x os pixels) nao vale para 1080p, e vice-versa.
//   - NUNCA vira 720p sozinho (pedido do dono, 06/10: "720p e o pior cenario,
//     e o que fica mais feio"): o maximo automatico e o 2, em 1080p.
//   - nos demais alvos: nivel 0 por padrao (.wgt nao muda). No Mac,
//     NUVIO_GPU_NIVEL=N no ambiente forca o nivel, para ver o resultado.
//
// GLES3 / EXTENSOES: o host cria contexto GLES 2.0 (Program.cs Version20,
// Program40.cs TVGLApplication), mas o driver pode entregar um contexto maior
// (a Mali do 8825 responde "OpenGL ES 3.2"). So se usa glInvalidateFramebuffer
// se GL_VERSION disser "OpenGL ES 3" E eglGetProcAddress devolver a funcao;
// senao glDiscardFramebufferEXT se GL_EXT_discard_framebuffer estiver na
// lista; senao nada (o caminho GLES2 de sempre). Onde: antes de sobrescrever
// um alvo inteiro (ampliacao do nivel 2, passadas do desfoque) e, no fim do
// quadro, profundidade/stencil da janela quando ela os tem.
#ifndef NV_GPUNIVEL_H
#define NV_GPUNIVEL_H

// Depois do contexto GL, ANTES de gfx_iniciar e de tex_iniciar: le GL_*,
// decide o nivel inicial e marca a GPU fraca no perfil (perfiltv.h).
// `w`,`h` = drawable da janela.
void gpun_iniciar(int w, int h);
// Query with the current GL context; Mali-4xx/no fragment highp use bilinear.
int gpun_upscale_suportado(void);
// Output choice, independent of drawable size and internal resolution/effects.
// Pass panel eligibility + opt-in from ajustes_ampliar_app(); this gate rejects
// unsupported GPUs (Mali-4xx/no-highp), even on an eligible 4K panel.
void gpun_ampliar_app(int app);
// Preview resize: keep the selected GPU level and rebuild its internal target.
// Called on the drawing thread before beginning the next frame; other builds ignore it.
void gpun_redimensionar(int w, int h);

// Uma linha no log: [perfil] tpk mem=... gpu=... -> tex=... (so no .tpk).
void gpun_log_perfil(long memMB, int texMb, int fios, int heroi);

int  gpun_nivel(void);
// Efeitos reduzidos pelo automatico, para a ajuda dos Ajustes.
int  gpun_efeitos_automaticos(void);
// Troca o nivel na hora, sem medir nem gravar (captura de teste).
void gpun_definir_nivel(int n);
// Effects preference, all platforms: 0 automatic, 1 full, 2 light, 3 minimal.
// Stored preference values are positional; new values must be appended.
void gpun_preferencia(int p);

// Explicit 720p resolution: internal 1280x720 target, independent of effects.
// Does not disable adaptation or override a manual effects preference.
void gpun_forcar_720(void);

// 4K that did not hold (resolucao.h): from the next frame on, the interface is
// drawn into an internal 1920x1080 target and scaled to the 4K surface in one
// pass; the adaptive level measures again at 1080p. No-op without a surface
// above 1080p or with 720p forced. Session-only; main.c remembers it on disk.
void gpun_alvo_1080(void);
int  gpun_alvo_1080_ativo(void);

// Laco de quadro: _inicio ANTES do glClear da tela (liga o alvo interno no
// explicit 720p or 4K fallback), _fim after drawing and before swap.
void gpun_quadro_inicio(void);
void gpun_quadro_fim(void);

// Previous frame phases. `cena` is a nonzero stable workload identity for
// full Home/Settings (0 = ineligible); changed identity cancels comparison.
// `cheia` requires artwork only on Home; Settings is eligible without it.
void gpun_medir(double dtms, double espera, double cpu, int cena, int cheia);
// Live FPS/[gpu-modos] report (~3 seconds), same scene/artwork eligibility.
// Three reports below refresh minus 5 FPS shed one effects tier; ten above
// refresh minus 2 FPS restore one. Reports with >1-second stalls are ignored.
// Saved levels are provisional; failed restorations cannot repeat per workload.
// Once eligible, this reporter governs instead of the legacy frame comparison.
void gpun_relatar(double fps, double pior, double periodo, int cena, int cheia);

// Descarta a COR do alvo ligado agora, que vai ser sobrescrito inteiro.
// `padrao` = 1 se o ligado e a janela (FBO 0). No-op sem ES3/extensao.
void gpun_descartar_cor(int padrao);
#endif
