Home initial publication investigation, compared with `v1.7.4`.

The rule `(cat_n() == 0) || parcialNaTela` and the sequential CW/social gate
are the same in v1.7.4 and the integration checkout. It is not evidence that
Glass added a new network timeout. A verified difference from #233 is that
fixed CW/social rows no longer consume the catalogue quota; the same setting
can therefore request two additional catalogues. Their latency must not delay
showing a row which is already ready.

A populated catalogue containing only CW/social was treated as a complete Home
and switched to silent publication. In a deterministic local fixture, Cat A
was ready at 1 ms but did not enter the published catalogue until 354 ms,
after Cat B's controlled 350 ms response. The cold assertion failed before
the patch (`/tmp/nuvio-home-progressiva-before.log`). This reproduces the
publication wait; these numbers are not a physical-TV network benchmark.

`cat_home_apenas_fixas()` now takes the decision under the catalogue publication
mutex. Empty and exclusively CW/social catalogues can grow progressively.
Any other row, including a collection with an empty base, remains warm. Trakt
list/collection markers and unassigned items remain warm too, so an early CW
batch cannot erase a ready watchlist. A warm catalogue still publishes once at
completion and keeps its existing rows/items/keys while HTTP is in flight.
The existing partial-cycle rule remains, as do source/generation checks,
snapshot/cache ownership, order, quota, timeouts and three catalogue workers.
This patch does not parallelize CW/social or change their network policy.

The new fixture asserts the actual Cat A item and row through mutex-protected
catalogue copy APIs at the public milestone, not just its log label. It also
asserts ready cached items, a ready Trakt watchlist and a base-less collection
at the first pending milestone. All five scenarios pass with ASan/UBSan:

| Scenario | Ready | Published | Behavior |
| --- | ---: | ---: | --- |
| Cold, CW/social only | 13 ms | 13 ms | Cat A published before slow Cat B |
| Warm, three catalogue rows | 13 ms | 355 ms | Ready rows kept during refresh |
| Ready Trakt list only | 11 ms | 361 ms | Watchlist kept while fresh CW/social arrives |
| Collection row, empty base | 13 ms | 359 ms | Existing collection kept during refresh |
| Account/generation changes mid-request | 73 ms | 73 ms | Old cycle abandoned, new owner rebuilt |

Final evidence: `/tmp/nuvio-home-progressiva-final-asan.log`.

```
SANITIZE=1 bash tests/home_progressiva.sh
SANITIZE=1 bash tests/montagem_cedo.sh
SANITIZE=1 bash tests/montagem_estrutura.sh
SANITIZE=1 bash tests/snapshot_falha.sh
SANITIZE=1 bash tests/homejanelas.sh
```

The three existing discovery/snapshot suites passed before and after the
change. Their fixture updates supply the newer `recomenda_geracao` API;
`montagem_cedo` also exposes controlled fixture hooks and a CW failure stub.
`homejanelas` models only complete-package/empty states and supplies the new
getter double. No physical-device gain, packaging or installation is claimed.
