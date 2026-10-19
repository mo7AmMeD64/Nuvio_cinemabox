#include <assert.h>
#include "../src/badges.c"
int main(void){
  uint64_t m=badges_detectar("Movie.2160p.WEB-DL.DV.Atmos.5.1.HEVC.NFLX");
  assert(m&bit("r-4k"));assert(m&bit("a-atmos-dv"));assert(!(m&bit("v-dv")));
  assert(m&bit("co-x265"));assert(m&bit("p-netflix"));assert(m&bit("c-51"));
  assert(!badges_detectar("Adventure.S01E01.mkv"));
  m=badges_detectar("1080p HDR10+ DTS-HD MA 7.1 BluRay");
  assert(m&bit("v-hdr10plus"));assert(!(m&bit("v-hdr10")));assert(m&bit("a-dtshdma"));assert(!(m&bit("a-dts")));
  assert(!badges_provedor("unknown"));assert(badges_provedor("Apple TV+")==bit("p-appletv"));
  assert(!(badges_detectar("Movie 1080p 4k")&bit("r-4k")));
  m=badges_detectar("Movie 2160p HLG HEVC");
  assert(m&bit("v-hlg"));assert(!(m&bit("v-hdr")));
  assert(badges_detectar("Movie 2160p HDR")&bit("v-hdr"));
  assert(badges_detectar("Movie.2160p.DolbyVision.mkv")&bit("v-dv"));
  assert(badges_detectar("Movie.2160p.DV.DTS-HD.MA.mkv")&bit("v-dv"));
  assert(badges_provedor("Netflix HLG")==bit("p-netflix"));
  // #198: cor por grupo, a do pacote inicial da wiki do Nuvio (badges.c).
  { float r,g,b;
    assert(badges_cor_selo("r-4k",&r,&g,&b) && r==1.0f && (int)(g*255+.5f)==0xBE && (int)(b*255+.5f)==1);
    assert(badges_cor_selo("q-webdl",&r,&g,&b) && (int)(g*255+.5f)==0xC0);
    assert(badges_cor_selo("v-dv",&r,&g,&b) && (int)(g*255+.5f)==0x6B);
    assert(badges_cor_selo("a-atmos-dv",&r,&g,&b) && (int)(g*255+.5f)==0x6B);   // grupo de video
    assert(badges_cor_selo("a-atmos",&r,&g,&b) && (int)(b*255+.5f)==0xD1);
    assert(badges_cor_selo("c-51",&r,&g,&b) && (int)(g*255+.5f)==0xD7);
    assert(badges_cor_selo("co-x265",&r,&g,&b) && (int)(r*255+.5f)==0x9C);
    assert(badges_cor_selo("p-netflix",&r,&g,&b) && (int)(r*255+.5f)==0xE5);
    assert(badges_cor_selo("p-crave",&r,&g,&b) && (int)(r*255+.5f)==0x85);
    assert(!badges_cor_selo("x-nada",&r,&g,&b)); }
  puts("badges: PASS (metadata, combined Dolby, hierarchy, unknown omitted, group colors)");
}
