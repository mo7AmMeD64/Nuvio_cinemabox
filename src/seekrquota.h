// Installation-wide Seekr lookup budget. No profile, account or key identity
// participates in the ledger. Clearing app data/reinstalling can erase it;
// stronger enforcement requires a registered-device backend.
#ifndef NV_SEEKRQUOTA_H
#define NV_SEEKRQUOTA_H

#define SEEKR_QUOTA_DIA 50
enum {
  SEEKR_QUOTA_OK = 1,
  SEEKR_QUOTA_LIMITE = 0,
  SEEKR_QUOTA_ARMAZENAMENTO = -1,
  SEEKR_QUOTA_RELOGIO = -2
};
typedef struct {
  int usadas, restantes, limite;
  int relogioAtrasado, persistente;
  long long reinicioUtc; // Unix seconds; convert to local time in the UI.
} SeekrQuotaUso;

// Worker only. Reserve durably BEFORE each /sprites HTTP dispatch. A failed
// or cancelled request still consumes its reservation; VTT/JPEG/validate do not.
// utc is explicit to permit deterministic rollover/rollback tests.
int seekrquota_reservar(long long utc);
// Cheap snapshot after the first read. A rollback never opens an older window.
void seekrquota_uso(long long utc, SeekrQuotaUso *uso);

#endif
