N01 introduces an additive blocking API for existing worker threads. The names
`RedePedido`/`RedeResposta` follow the plugins2 contract; the implementation is
new and does not replace legacy wrappers, Discord TLS, range recovery, their
timeouts, or their connection reuse. No plugins/P2P modules were imported.

`rede_pedir` owns an isolated native curl handle and callback context. Response
bodies default to 8 MiB and cannot exceed 32 MiB; headers default to 16 KiB and
cannot exceed 64 KiB across informational responses and redirects. Length and
decoded gzip output are checked before growing buffers. Limit, deadline,
generation, cancellation, OOM and transport failures discard the body. HTTP
4xx/5xx are successful HTTP responses with status/body. The caller must free a
response with `rede_resposta_limpar` before reusing it. Response headers and
the final URL are private; only `host` is redacted for display/logging.

TLS verification is mandatory in this new API. An application-owned CA bundle
can be supplied. Redirects are manual, limited to five, restricted to HTTP(S),
and refuse HTTPS-to-HTTP. All caller headers are permanently dropped after a
change in scheme/host/port, including unknown custom API keys. Bodies and
non-GET/HEAD methods are refused across origins unless 303 or POST 301/302 first
converts them to GET. Header injection, userinfo URLs, Host and caller body
framing headers are rejected. Same-origin redirects retain headers.

`RedeGrupo` owns a generation; advance it on account/profile/title changes.
Each `RedeJob` captures one generation and has independent cancellation.
References keep groups/jobs alive throughout the request. The request checks
again before returning. A consumer that queues a result must also check
`rede_job_estado` at publication: no synchronous network function can retract
a result after a later profile change. This is cancellation/lifetime support,
not a new scheduler or an automatic connection pool.

One deadline covers all hops, with no hidden retries. Curl progress interrupts
active transfer/idle-body waits. A native curl built with a **blocking DNS
resolver** plus NOSIGNAL can remain inside DNS beyond the deadline and cannot
be synchronously interrupted by this API. No hard DNS cancellation claim is
made. `rede_pedido_capacidades` describes compiled support, not an appliance
benchmark. TPK4 state uses pthread keys and mutexes, without compiler TLS.

`bytes_fio` describes the final hop's transferred payload before decompression;
`n_corpo` describes decoded bytes. Intervals have real elapsed milliseconds,
including measured zero-byte idle intervals; delayed progress is not split
into fabricated one-second samples. A final interval shorter than one second
is marked incomplete. No body means no interval/TTFB estimate. StreamFit must
require an acceptable final HTTP status, `erro == REDE_OK`, the correct
resource/host/network generation, and complete intervals before using them.
Request `ms` includes connection/redirect time; `corpo_ms` excludes that wait.
The API is not wired to playback or StreamFit by this patch. New isolated
handles do not yet share a connection cache between calls; latency gains have
not been measured on any physical TV.

WGT's existing synchronous XHR cannot enforce receive-time caps, deadlines or
abort while blocked. The strict API returns `REDE_INDISPONIVEL` and capabilities
zero **without issuing XHR**. Legacy wrappers continue to work. Their
`rede_baixar_st_retry` now reads Retry-After seconds/HTTP-date when exposed by
CORS; hidden/missing/invalid values stay zero. This is not an async WGT fetch
adapter or proof of behavior on physical Samsung hardware.

Verification:

```
bash tests/rede_pedido.sh
NV_SANITIZERS=1 bash tests/rede_pedido.sh
NV_SANITIZERS=1 NV_TPK40_TEST=1 bash tests/rede_pedido.sh
NV_TSAN=1 bash tests/rede_pedido.sh
```

The fixture uses three local HTTP/TLS origins and a temporary self-signed CA,
never contacts external providers, and never prints headers or credentials.
Tests cover verified TLS, redirects/methods/secrets, complete/truncated bodies,
binary NUL, known/chunked/gzip limits, pre-allocation/OOM/overflow, total
deadline, cancellation/generation during an idle response, lifetime after the
owner releases a group, late-publication fencing, concurrent/nested requests,
wire intervals including a quiet window, and Retry-After from real native HTTP
and extracted production WGT JavaScript. Cold concurrent startup also exercises
acquire/release publication of curl initialization. The TPK40 run is host configuration
and symbol evidence, not an ARM package or physical-device benchmark.

Legacy focused regressions: `rede_buffer.sh`, `rede_reuso.sh`,
`rede_parada.sh`, `rede_sonda.sh` (also WGT header/final-URL probe contract).
