"""Execute variant C panel colors across focus and page fades."""
from pathlib import Path
import subprocess
import tempfile

source = Path('src/detail.c').read_text()
start = source.index('  if (epVidroTile)\n', source.index('static void desenhaEpisodio('))
tile = source[start:source.index('  // DESFOCAR NAO ASSISTIDOS', start)]
program = r'''
#include <assert.h>
#include <math.h>
#include <stdio.h>
typedef struct { float x,y,w,h; } GfxRect;
static int draws;
static float alpha, red, green, blue;
static void gfx_cor(GfxRect r,float radius,float cr,float cg,float cb,float a) {
  (void)r; (void)radius;
  draws++; alpha=a; red=cr; green=cg; blue=cb;
}
static void draw(int epVidroTile,float ft,float a) {
  GfxRect epTile={0,360,640,200};
''' + tile + r'''
}
int main(void) {
  for(int modern=0;modern<=1;modern++)
    for(int focus=0;focus<=100;focus++)
      for(int fade=1;fade<=2;fade++) {
        float a=fade*.5f; draws=0; alpha=0;
        float ft=focus/100.0f;
        draw(modern,ft,a);
        assert(draws==modern);
        assert(fabsf(alpha-(modern?a:0))<.00001f);
        if (modern) {
          assert(fabsf(red-(31+7*ft)/255)<.00001f);
          assert(fabsf(green-(33+7*ft)/255)<.00001f);
          assert(fabsf(blue-(37+8*ft)/255)<.00001f);
        }
      }
  puts("PASS: opaque panel interpolates #1f2125 to #26282d; Standard has no panel");
}
'''
with tempfile.TemporaryDirectory(prefix='episode-tint-') as tmp:
    cfile = Path(tmp) / 'tint.c'
    binary = Path(tmp) / 'tint'
    cfile.write_text(program)
    subprocess.run(['cc', '-std=c99', '-Wall', '-Wextra', '-Werror', str(cfile),
                    '-lm', '-o', str(binary)], check=True)
    subprocess.run([str(binary)], check=True)
