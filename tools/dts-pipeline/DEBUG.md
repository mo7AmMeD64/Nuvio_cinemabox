# DTS Debug package

**Nuvio Legacy DTS Debug** uses the same native-first fallback as the normal app,
with additional diagnostics. Its ID is `space.nuvio.native.legacy.dtsdebug`;
it has a separate app/service identity and data directory, and no self-updater.
It can be installed alongside the normal app.

Build the SDK using the [main build recipe](../../README.md#building), then:

```sh
NUVIO_CONTAINER_RUNTIME=podman \
NUVIO_BUILD_PLATFORM=linux/amd64 \
NUVIO_ARES_PACKAGE=/absolute/path/to/ares-package \
bash tools/build-dts-debug.sh
```

For ARM64 use `NUVIO_BUILD_PLATFORM=linux/arm64`. Docker can replace Podman.
The helper uses the normal `nuvio-webos-sdk` image; `NUVIO_SDK_IMAGE` selects
another image (`NUVIO_DTS_DEBUG_SDK_IMAGE` overrides it for debug builds).
The CLI must be executable; the Node-based LG CLI also needs Node on `PATH`.
The helper reads `local.properties`; `NUVIO_PROPERTIES` selects another file.

The helper builds in a temporary tree, verifies identity, adapters and notices,
and preserves production artifacts. It does not install or contact a TV.
Outputs in `build/dts-debug/` are the IPK, `.ipk.sha256` and `.ipk.json` manifest.
Use `--output build/dts-debug/new-build` to preserve another build; overwriting
a same-named package with different bytes is refused.

Install the IPK with the usual webOS tooling and launch its separate app entry.
Reproduce playback and collect the app log with model, firmware, chosen track
and audio output. `worker-rate` and `[dts-transport]` report conversion/buffering
and network timing. Host checks: `bash tests/dts_debug.sh` and
`python3 tests/dts_debug_build.py`. See the [DTS guide](../../docs/features/dts/README.md)
for limits and licenses.
