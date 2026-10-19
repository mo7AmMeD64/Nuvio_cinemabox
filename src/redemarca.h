#ifndef NV_REDEMARCA_H
#define NV_REDEMARCA_H
#include <stdint.h>
/* Process-local, opaque generation from a trustworthy default-network
 * observer. No observer or uncertain connectivity = 0; never infer from
 * HTTP success, network health, SSID or the LAN address. */
uint64_t redemarca_atual(void);
/* Adapter sequence advances even on disconnect. Delayed notifications from
 * an older sequence cannot resurrect a network. No persistence or logging. */
void redemarca_observar(uint64_t sequencia, int conhecida);
#endif
