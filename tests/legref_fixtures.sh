#!/bin/bash
# Fixtures MKV reais para tests/legref.sh e tests/legsync.sh (ffmpeg + mkvmerge).
#   bash tests/legref_fixtures.sh DIR
set -eu
D=${1:-${TMPDIR:-/tmp}/nv-legref-fx}; mkdir -p "$D"
[ -f "$D/.ok" ] && [ -f "$D/ext_traduzida_mais2000.srt" ] && [ -f "$D/filme.mkv" ] && [ -f "$D/cruz2.mkv" ] && exit 0
command -v ffmpeg >/dev/null && command -v mkvmerge >/dev/null || { echo "precisa de ffmpeg e mkvmerge"; exit 2; }
python3 - "$D" <<'PY'
import random, sys
d = sys.argv[1]
random.seed(5)
def ts(t, sep=','):
    ms = int(round(t * 1000))
    return "%02d:%02d:%02d%s%03d" % (ms // 3600000, ms // 60000 % 60, ms // 1000 % 60, sep, ms % 1000)
def ass_ts(t):
    cs = int(round(t * 100))
    return "%d:%02d:%02d.%02d" % (cs // 360000, cs // 6000 % 60, cs // 100 % 60, cs % 100)
t = 10.0; ev = []
while t < 600:
    dur = random.uniform(1.0, 3.5)
    ev.append((t, t + dur, "Line number %d says something %d" % (len(ev) + 1, random.randint(0, 99999))))
    t += dur + random.uniform(0.3, 6.0)
def srt(name, shift=0.0, pt=False):
    with open("%s/%s" % (d, name), "w") as f:
        for i, (a, b, s) in enumerate(ev):
            txt = ("Fala traduzida numero %d diferente" % (i + 1)) if pt else s
            f.write("%d\n%s --> %s\n%s\n\n" % (i + 1, ts(a + shift), ts(b + shift), txt))
srt("emb.srt"); srt("ext_mais2500.srt", 2.5, pt=True); srt("ext_menos1200.srt", -1.2)
# Traducao de verdade (como as do OpenSubtitles): outra segmentacao (falas
# vizinhas juntas, falas longas partidas em duas coladas) e bordas com folga
# de quadro, +2,0 s do video. A engine recusa este par (fail-closed); o teste
# de regressao confere que a pilula NAO diz "sincronizada" e que nada mudou.
random.seed(11)
with open("%s/ext_traduzida_mais2000.srt" % d, "w") as f:
    out = []; i = 0
    while i < len(ev):
        a, b, _ = ev[i]
        if i + 1 < len(ev) and ev[i + 1][0] - b < 1.0 and random.random() < 0.6:
            b = ev[i + 1][1]; i += 1
        if b - a > 3.0 and random.random() < 0.5:
            m = (a + b) / 2; out.append((a, m)); out.append((m, b))
        else: out.append((a, b))
        i += 1
    for k, (a, b) in enumerate(out):
        a += 2.0 + random.uniform(-0.12, 0.12); b += 2.0 + random.uniform(-0.12, 0.12)
        f.write("%d\n%s --> %s\nFala traduzida %d com outro corte\n\n" % (k + 1, ts(a), ts(b), k + 1))
with open("%s/emb.ass" % d, "w") as f:
    f.write("[Script Info]\nScriptType: v4.00+\nPlayResX: 1920\nPlayResY: 1080\n\n[V4+ Styles]\n"
            "Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding\n"
            "Style: Default,Arial,48,&H00FFFFFF,&H000000FF,&H00000000,&H00000000,0,0,0,0,100,100,0,0,1,2,0,2,10,10,10,1\n\n"
            "[Events]\nFormat: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\n")
    for a, b, s in ev:
        f.write("Dialogue: 0,%s,%s,Default,,0,0,0,,{\\i1}%s{\\i0}\n" % (ass_ts(a), ass_ts(b), s))
print(len(ev))
# FILME de 2 h (regressao "fala que ta ok e ta fora de sincronia"): a mesma
# densidade de falas de um longa (~1300), cada uma num Cluster diferente. A
# referencia embutida custa UM Range por fala; no ritmo de producao (8/s) isso
# passa de 2 minutos, e o plano automatico desistia aos 45 s.
random.seed(23)
t = 12.0; fev = []
while t < 7180:
    dur = random.uniform(1.0, 3.5)
    fev.append((t, t + dur, "Film line %d number %d" % (len(fev) + 1, random.randint(0, 99999))))
    t += dur + random.uniform(0.3, 6.0)
with open("%s/filme_emb.srt" % d, "w") as f:
    for i, (a, b, s) in enumerate(fev): f.write("%d\n%s --> %s\n%s\n\n" % (i + 1, ts(a), ts(b), s))
with open("%s/filme_ext_mais2500.srt" % d, "w") as f:
    for i, (a, b, s) in enumerate(fev):
        f.write("%d\n%s --> %s\nFala traduzida numero %d diferente\n\n" % (i + 1, ts(a + 2.5), ts(b + 2.5), i + 1))
print(len(fev))
PY
cd "$D"
# ~23 MB: os blocos de legenda ficam ESPALHADOS entre megabytes de video, como num filme.
ffmpeg -loglevel error -y -f lavfi -t 610 -i "testsrc2=s=640x360:r=10" -c:v libx264 -preset ultrafast -b:v 300k -g 50 video.mkv
# ffmpeg: SRT em ingles, sem nada mais.
ffmpeg -loglevel error -y -i video.mkv -i emb.srt -map 0 -map 1 -c copy -c:s srt -metadata:s:s:0 language=eng ff.mkv
# mkvmerge: forced (letreiro) primeiro, depois a de dialogo em ASS, depois SRT em portugues.
mkvmerge -q -o mm.mkv video.mkv --language 0:eng --forced-display-flag 0:1 --track-name 0:Signs emb.srt \
  --language 0:eng emb.ass --language 0:por ext_mais2500.srt
# Sem Cues para a legenda: o indice so aponta o video.
mkvmerge -q -o semcues.mkv video.mkv --cues 0:none --language 0:eng emb.srt
# Validacao cruzada: a primeira faixa em ingles esta +1,5 s fora; outra em
# ingles e uma em portugues concordam no tempo certo. Em cruz2 so as duas
# primeiras (sem como desempatar).
mkvmerge -q -o cruz.mkv video.mkv --language 0:eng --sync 0:1500 emb.srt --language 0:eng --track-name 0:English2 emb.srt \
  --language 0:por emb.srt
mkvmerge -q -o cruz2.mkv video.mkv --language 0:eng --sync 0:1500 emb.srt --language 0:eng --track-name 0:English2 emb.srt
# So letreiro.
mkvmerge -q -o soforced.mkv video.mkv --language 0:eng --forced-display-flag 0:1 emb.srt
ffmpeg -loglevel error -y -i video.mkv -c copy video.mp4
# MP4 com tx3g (mov_text): moov no FIM (padrao do ffmpeg, um mdat de ~22 MB
# antes), ingles + portugues; o mesmo com moov no comeco; e fragmentado.
ffmpeg -loglevel error -y -i video.mp4 -i emb.srt -i ext_mais2500.srt -map 0 -map 1 -map 2 -c copy -c:s mov_text \
  -metadata:s:s:0 language=eng -metadata:s:s:1 language=por txt.mp4
ffmpeg -loglevel error -y -i txt.mp4 -map 0 -c copy -movflags +faststart txt_fs.mp4
ffmpeg -loglevel error -y -i video.mp4 -i emb.srt -map 0 -map 1 -c copy -c:s mov_text -movflags frag_keyframe+empty_moov frag.mp4
# ~40 MB, 2 h: video minusculo, mas um Cluster a cada 5 s como num filme.
ffmpeg -loglevel error -y -f lavfi -t 7200 -i "testsrc2=s=160x90:r=2" -c:v libx264 -preset ultrafast -b:v 40k -g 10 video_filme.mkv
mkvmerge -q -o filme.mkv video_filme.mkv --language 0:eng filme_emb.srt
rm -f video_filme.mkv
touch .ok
