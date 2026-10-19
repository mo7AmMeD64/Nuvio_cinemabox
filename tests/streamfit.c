#include "streamfit.h"
#include <assert.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

static const uint64_t NOW = UINT64_C(1700000000000);
static int checks;
#define CHECK(x) do { assert(x); checks++; } while (0)
static StreamfitFoto foto;
static StreamfitResultado r;
static int fast[8] = {20000,20000,20000,20000,20000,20000,20000,20000};
static uint64_t bytes(double kbps) { return (uint64_t)(kbps * 1000.0 * 7200.0 / 8.0); }
static StreamfitClasse classificar(const char *url, uint64_t b, double s) {
  return streamfit_classificar(&foto,url,b,s,&r);
}

static void evidence(void) {
  streamfit_limpar(); streamfit_foto(&foto,NOW);
  CHECK(classificar("https://media.invalid/a",bytes(10000),7200)==SF_DESCONHECIDA && r.razao==SF_SEM_REDE);
  CHECK(!streamfit_diagnostico(1,"https://media.invalid/a",fast,8,NOW));
  streamfit_rede(1);
  CHECK(!streamfit_diagnostico(2,"https://media.invalid/a",fast,8,NOW));
  CHECK(!streamfit_diagnostico(1,"https://media.invalid/a",fast,4,NOW));
  CHECK(!streamfit_diagnostico(1,"https://media.invalid/a",fast,49,NOW));
  CHECK(!streamfit_diagnostico(1,"https://media.invalid/slate.mp4",fast,8,NOW));
  CHECK(!streamfit_diagnostico(1,"file:///media/a",fast,8,NOW));
  CHECK(streamfit_diagnostico(1,"https://user:secret@MEDIA.invalid:443/path?token=secret",fast,8,NOW)==8);
  CHECK(!streamfit_diagnostico(1,"https://media.invalid/a",fast,8,NOW)); // replay
  CHECK(!streamfit_diagnostico(1,"https://media.invalid/a",fast,8,NOW-1));
  streamfit_foto(&foto,NOW+1000);
  CHECK(foto.n==1 && !strcmp(foto.hosts[0].host,"media.invalid"));
  CHECK(!strstr(foto.hosts[0].host,"secret") && foto.hosts[0].otimoKbps==15000);
  CHECK(classificar("https://media.invalid/other",bytes(15000),7200)==SF_ADEQUADA);
  CHECK(r.necessarioKbps==15000 && r.otimoKbps==15000 && r.maximoKbps==18000 && r.idadeMs==1000);
  CHECK(classificar("https://media.invalid/a",bytes(15001),7200)==SF_PESADA);
  CHECK(classificar("https://cdn.invalid/a",bytes(15001),7200)==SF_DESCONHECIDA && r.razao==SF_SEM_MEDIDA);
  CHECK(classificar("https://media.invalid:8443/a",bytes(15001),7200)==SF_DESCONHECIDA);
  CHECK(classificar("https://media.invalid/a",0,7200)==SF_DESCONHECIDA && r.razao==SF_SEM_TAMANHO);
  CHECK(classificar("https://media.invalid/a",UINT64_MAX,7200)==SF_DESCONHECIDA);
  CHECK(classificar("https://media.invalid/a",bytes(5000),0)==SF_DESCONHECIDA && r.razao==SF_SEM_DURACAO);
  CHECK(classificar("https://media.invalid/a",bytes(5000),NAN)==SF_DESCONHECIDA);
  CHECK(classificar("https://media.invalid/a",bytes(5000),0.5)==SF_DESCONHECIDA);
  CHECK(classificar("https://media.invalid/a",bytes(5000),1e-300)==SF_DESCONHECIDA);
  CHECK(classificar("https://media.invalid/a",bytes(5000),INFINITY)==SF_DESCONHECIDA);
  CHECK(classificar("https://media.invalid/a",bytes(5000),86401)==SF_DESCONHECIDA);
  CHECK(classificar("https://media.invalid/downloading.mp4",bytes(5000),7200)==SF_DESCONHECIDA);
  char huge[300]; memset(huge,'a',sizeof huge);memcpy(huge,"https://",8);huge[299]=0;
  CHECK(!streamfit_diagnostico(1,huge,fast,8,NOW+1));
  streamfit_foto(&foto,NOW+STREAMFIT_IDADE_MS); CHECK(foto.n==1);
  streamfit_foto(&foto,NOW+STREAMFIT_IDADE_MS+1); CHECK(foto.n==0);
  streamfit_foto(&foto,NOW-1); CHECK(foto.n==0); // clock rollback
  int bad[8]={-1,-1,-1,10000001,20000,20000,20000,20000};
  CHECK(!streamfit_diagnostico(1,"https://bad.invalid/a",bad,8,NOW));
  int stalled[8]={0,0,20000,20000,20000,20000,20000,20000};
  CHECK(streamfit_diagnostico(1,"https://stall.invalid/a",stalled,8,NOW)==8);
  streamfit_foto(&foto,NOW); CHECK(classificar("https://stall.invalid/a",bytes(500),7200)==SF_PESADA && r.otimoKbps==0);
  StreamfitFoto frozen = foto;
  CHECK(streamfit_diagnostico(1,"https://media.invalid/a",stalled,8,NOW+1)==8);
  CHECK(streamfit_classificar(&frozen,"https://media.invalid/a",bytes(10000),7200,&r)==SF_ADEQUADA);
  streamfit_rede(2); streamfit_foto(&foto,NOW+1); CHECK(foto.n==0 && foto.rede==2);
  streamfit_rede(1); streamfit_foto(&foto,NOW+1); CHECK(foto.n==0); // returning doesn't revive old evidence
  // Freshness is per observation, not a timestamp refreshed over stale data.
  CHECK(streamfit_diagnostico(1,"https://age.invalid/a",fast,8,NOW)==8);
  int slow[5]={1000,1000,1000,1000,1000};
  CHECK(streamfit_diagnostico(1,"https://age.invalid/a",slow,5,NOW+STREAMFIT_IDADE_MS+1)==5);
  streamfit_foto(&foto,NOW+STREAMFIT_IDADE_MS+1);
  CHECK(foto.n==1 && foto.hosts[0].amostras==5 && foto.hosts[0].otimoKbps==750);
  for(int k=0;k<20;k++) CHECK(streamfit_diagnostico(1,"https://age.invalid/a",slow,5,NOW+STREAMFIT_IDADE_MS+2+k)==5);
  streamfit_foto(&foto,NOW+STREAMFIT_IDADE_MS+100); CHECK(foto.hosts[0].amostras==5);
  int maximum[STREAMFIT_AMOSTRAS_MAX];for(int i=0;i<STREAMFIT_AMOSTRAS_MAX;i++) maximum[i]=1000;
  CHECK(streamfit_diagnostico(1,"https://age.invalid/a",maximum,STREAMFIT_AMOSTRAS_MAX,NOW+STREAMFIT_IDADE_MS+99)==48);
  streamfit_foto(&foto,NOW+STREAMFIT_IDADE_MS+100); CHECK(foto.hosts[0].amostras==48);
  for(int k=0;k<40;k++) {
    char host[80];snprintf(host,sizeof host,"https://host-%d.invalid/a",k);
    CHECK(streamfit_diagnostico(1,host,fast,8,NOW+STREAMFIT_IDADE_MS+100+k)==8);
  }
  streamfit_foto(&foto,NOW+STREAMFIT_IDADE_MS+200);CHECK(foto.n==STREAMFIT_HOSTS_MAX);
  CHECK(classificar("https://age.invalid/a",bytes(5000),7200)==SF_DESCONHECIDA);
  streamfit_rede(0);streamfit_foto(&foto,NOW);CHECK(foto.n==0 && foto.rede==0);
}

static void order(void) {
  int seq[8]={0,1,2,3,4,5,-1,99}, tmp[8];
  unsigned char c[6]={SF_PESADA,SF_ADEQUADA,SF_DESCONHECIDA,SF_PESADA,SF_ADEQUADA,SF_PESADA};
  streamfit_particionar(seq,8,c,6,tmp);
  int expected[8]={1,2,4,-1,99,0,3,5};
  CHECK(!memcmp(seq,expected,sizeof seq));
  unsigned seed=0x17a8;
  for(int run=0;run<400;run++) {
    int ord[256], saved[256], work[256], len=1+run%256;
    unsigned char classes[256];
    for(int i=0;i<len;i++) {seed=seed*1664525+1013904223;classes[i]=(unsigned char)(seed%3);ord[i]=i;}
    for(int i=len-1;i>0;i--) {seed=seed*1664525+1013904223;int j=(int)(seed%(unsigned)(i+1)), t=ord[i];ord[i]=ord[j];ord[j]=t;}
    memcpy(saved,ord,(size_t)len*sizeof *ord);
    streamfit_particionar(ord,len,classes,len,work);
    int k=0;
    for(int heavy=0;heavy<2;heavy++) for(int i=0;i<len;i++)
      if((classes[saved[i]]==SF_PESADA)==heavy) CHECK(ord[k++]==saved[i]);
    CHECK(k==len);
  }
}

static void *writer(void *unused) {
  (void)unused;
  for(int i=0;i<2000;i++) streamfit_diagnostico(7,"https://thread.invalid/a",fast,8,NOW+(unsigned)i);
  return NULL;
}
static void concurrent(void) {
  pthread_t t;
  streamfit_rede(7);CHECK(!pthread_create(&t,NULL,writer,NULL));
  for(int i=0;i<2000;i++) {StreamfitFoto s;streamfit_foto(&s,NOW+10000);CHECK(s.n>=0 && s.n<=STREAMFIT_HOSTS_MAX);}
  CHECK(!pthread_join(t,NULL));
  streamfit_foto(&foto,NOW+10000);CHECK(foto.n==1 && foto.hosts[0].amostras==8);
}

int main(void) { evidence();order();concurrent();printf("streamfit: PASS (%d deterministic checks, no network)\n",checks);return 0; }
