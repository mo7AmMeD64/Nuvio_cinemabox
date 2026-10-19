// SONDA DE MPEG-TS (#158). Na LG C4 do pasha o .ts do Xtream chega (buffer
// 8-36 s), o uMS reserva VDEC/ADEC e NUNCA manda videoInfo — nem esperando
// 45 s (registro 14195) — enquanto outro app na mesma TV toca o mesmo canal.
// O uMS nao diz o que recebeu; esta sonda le o comeco do fluxo e diz: e TS
// mesmo? que codecs a PMT declara? o video e H.264 High 10 / HEVC Main 10?
// esta embaralhado?
//
// Funcao pura, sem rede e sem SDL (tests/ts_sonda.sh compila no Mac).
#ifndef NV_TS_SONDA_H
#define NV_TS_SONDA_H

#include <stdio.h>
#include <string.h>

typedef struct {
  int pacotes;        // pacotes de 188 com sync 0x47 alinhado
  int deslocamento;   // onde o primeiro sync alinhado foi achado (-1 = nao e TS)
  int embaralhados;   // pacotes com transport_scrambling_control != 0
  int pmtPid;         // -1 = PAT nao vista
  int nEs;
  struct { int pid, tipo, desc; } es[8];  // desc: 0x6A AC3, 0x7A EAC3, 0x7B DTS (tipo 0x06)
  int videoPid;       // primeiro ES de video, -1 = nenhum
  int perfil, nivel;  // do SPS (H.264: profile_idc/level_idc; HEVC: general_profile_idc/level), -1
  int hevc;
  int pcrPid;          // da PMT; -1 = nao vista
  int pcrVistos;       // pacotes com PCR no campo de adaptacao
  int pesInicios[8];   // inicios de PES (00 00 01) por ES da PMT
} TsSonda;

static inline const char *ts_tipo_nome(int t, int desc) {
  switch (t) {
    case 0x01: return "MPEG-1 video";
    case 0x02: return "MPEG-2 video";
    case 0x03: return "MPEG-1 audio";
    case 0x04: return "MPEG-2 audio";
    case 0x0F: return "AAC";
    case 0x11: return "AAC-LATM";
    case 0x1B: return "H.264";
    case 0x24: return "HEVC";
    case 0x81: return "AC3";
    case 0x87: return "EAC3";
    case 0x06: return desc == 0x6A ? "AC3" : desc == 0x7A ? "EAC3" : desc == 0x7B ? "DTS"
                    : desc == 0x59 ? "legenda DVB" : desc == 0x56 ? "teletexto" : "privado";
    default:   return "?";
  }
}
static inline int ts_tipo_video(int t) { return t == 0x01 || t == 0x02 || t == 0x1B || t == 0x24; }

// Le o SPS no payload acumulado do PID de video.
static inline void ts_ler_sps(TsSonda *s, const unsigned char *p, int n) {
  int i;
  for (i = 0; i + 6 < n; i++) {
    if (p[i] || p[i + 1] || p[i + 2] != 1) continue;
    if (!s->hevc && (p[i + 3] & 0x1F) == 7) {         // H.264 SPS
      s->perfil = p[i + 4]; s->nivel = p[i + 6]; return;
    }
    if (s->hevc && ((p[i + 3] >> 1) & 0x3F) == 33 && i + 17 < n) {  // HEVC SPS
      // nal(2) + vps_id/max_sub_layers/nesting(1) + profile_space|tier|profile_idc(1)
      // + compat(4) + flags(6) + general_level_idc(1)
      s->perfil = p[i + 6] & 0x1F; s->nivel = p[i + 17]; return;
    }
  }
}

static inline void ts_sondar(const unsigned char *b, long n, TsSonda *s) {
  static unsigned char pes[65536];
  int nPes = 0;
  long i, ini = -1;
  memset(s, 0, sizeof *s);
  s->deslocamento = -1; s->pmtPid = -1; s->videoPid = -1; s->perfil = s->nivel = -1; s->pcrPid = -1;
  if (!b || n < 188 * 3) return;
  for (i = 0; i + 188 * 2 < n && i < 188 * 4; i++)
    if (b[i] == 0x47 && b[i + 188] == 0x47 && b[i + 376] == 0x47) { ini = i; break; }
  if (ini < 0) return;
  s->deslocamento = (int)ini;
  for (i = ini; i + 188 <= n; i += 188) {
    const unsigned char *k = b + i;
    int pid, pusi, af, off;
    if (k[0] != 0x47) break;
    s->pacotes++;
    pid = ((k[1] & 0x1F) << 8) | k[2];
    pusi = (k[1] >> 6) & 1;
    if ((k[3] >> 6) & 3) s->embaralhados++;
    af = (k[3] >> 4) & 3;
    off = 4;
    if ((af == 2 || af == 3) && k[4] > 0 && (k[5] & 0x10)) s->pcrVistos++;
    if (pusi && af != 2) {
      int o = af == 3 ? 5 + k[4] : 4, e;
      if (o + 3 < 188 && !k[o] && !k[o + 1] && k[o + 2] == 1)
        for (e = 0; e < s->nEs; e++) if (s->es[e].pid == pid) s->pesInicios[e]++;
    }
    if (af == 2 || af == 0) continue;
    if (af == 3) off += 1 + k[4];
    if (off >= 188) continue;
    if (pid == 0 && pusi && s->pmtPid < 0) {
      int p = off + 1 + k[off];                       // pointer_field
      int sl, j;
      if (p + 8 >= 188 || k[p] != 0x00) continue;
      sl = ((k[p + 1] & 0x0F) << 8) | k[p + 2];
      for (j = p + 8; j + 4 <= p + 3 + sl - 4 && j + 4 <= 188; j += 4) {
        int prog = (k[j] << 8) | k[j + 1];
        if (prog) { s->pmtPid = ((k[j + 2] & 0x1F) << 8) | k[j + 3]; break; }
      }
    } else if (pid == s->pmtPid && pusi && !s->nEs) {
      int p = off + 1 + k[off];
      int sl, fim, pil, j;
      if (p + 12 >= 188 || k[p] != 0x02) continue;
      sl = ((k[p + 1] & 0x0F) << 8) | k[p + 2];
      fim = p + 3 + sl - 4; if (fim > 188) fim = 188;
      pil = ((k[p + 10] & 0x0F) << 8) | k[p + 11];
      s->pcrPid = ((k[p + 8] & 0x1F) << 8) | k[p + 9];
      for (j = p + 12 + pil; j + 5 <= fim && s->nEs < 8; ) {
        int t = k[j], epid = ((k[j + 1] & 0x1F) << 8) | k[j + 2];
        int il = ((k[j + 3] & 0x0F) << 8) | k[j + 4], d = j + 5, desc = 0;
        while (d + 2 <= j + 5 + il && d + 2 <= 188) {
          if (!desc && (k[d] == 0x6A || k[d] == 0x7A || k[d] == 0x7B || k[d] == 0x59 || k[d] == 0x56))
            desc = k[d];
          d += 2 + k[d + 1];
        }
        s->es[s->nEs].pid = epid; s->es[s->nEs].tipo = t; s->es[s->nEs].desc = desc; s->nEs++;
        if (s->videoPid < 0 && ts_tipo_video(t)) { s->videoPid = epid; s->hevc = t == 0x24; }
        j += 5 + il;
      }
    } else if (pid == s->videoPid && s->perfil < 0 && nPes < (int)sizeof pes - 188) {
      memcpy(pes + nPes, k + off, (size_t)(188 - off)); nPes += 188 - off;
    }
  }
  if (nPes) ts_ler_sps(s, pes, nPes);
}

// Uma linha para o registro.
static inline void ts_resumo(const TsSonda *s, char *d, size_t n) {
  size_t u;
  int i;
  if (s->deslocamento < 0) { snprintf(d, n, "nao e MPEG-TS (sem sync 0x47 alinhado)"); return; }
  u = (size_t)snprintf(d, n, "%d pacotes, embaralhados=%d, pmt=%s", s->pacotes, s->embaralhados,
                       s->pmtPid < 0 ? "sem PAT" : s->nEs ? "ok" : "nao vista");
  for (i = 0; i < s->nEs && u < n; i++)
    u += (size_t)snprintf(d + u, n - u, " | pid %d 0x%02x %s pes=%d", s->es[i].pid, s->es[i].tipo,
                          ts_tipo_nome(s->es[i].tipo, s->es[i].desc), s->pesInicios[i]);
  if (u < n && s->nEs)
    u += (size_t)snprintf(d + u, n - u, " | pcr pid %d (%d vistos)", s->pcrPid, s->pcrVistos);
  if (u < n && s->videoPid >= 0)
    snprintf(d + u, n - u, " | sps perfil=%d nivel=%d%s", s->perfil, s->nivel,
             s->perfil < 0 ? " (SPS nao achado)" :
             (!s->hevc && s->perfil == 110) ? " (High 10: 10 bits)" :
             (!s->hevc && s->perfil == 122) ? " (High 4:2:2)" :
             (s->hevc && s->perfil == 2) ? " (Main 10)" : "");
}

#endif
