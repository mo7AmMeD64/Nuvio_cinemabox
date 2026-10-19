# Device-local preferences and account sync

The existing `somenteDesteAparelho` classification now guards incoming account blobs as well as outgoing settings and profile snapshots. A remote RAM/image-quality/GPU configuration therefore cannot overwrite this TV's local values. Shared choices such as hero visibility still follow the selected profile. `tests/ajustes_perfil.sh` covers local preservation, shared updates, profile switching and logout.

The installation identifier remains the existing `dados_cliente_id` stored in `cliente.txt`; no second device identity or account migration was introduced. This identifier prevents sync echo and is not an authentication secret. Its persistence failure handling has not been changed by this patch.

Memory diagnostics separately track the candidate, measured/applied budget and persisted automatic budget, including rollback and manual overrides. A 300 MB candidate does not mean 300 MB was retained, nor does it describe total process RAM. The diagnostic persistence and real-cache regressions are recorded in the execution ledger.

Validated here: local tests. Still required: the same account on Android and Samsung/LG with distinct local budgets, including sync and restart. No cross-device physical result is claimed.
