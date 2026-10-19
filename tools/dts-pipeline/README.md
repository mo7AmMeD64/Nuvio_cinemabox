# Starfish packet adapters

`src/dts/adapter/` bridges `nuvio_dts_adapter_v2` to LG firmware via
`dlopen`/`dlsym`. Missing symbols leave fallback unavailable. No firmware library
is bundled. `tools/arm.sh` builds/stages both C++ string ABIs automatically:
`dts-starfish-webos3.so` (legacy) and `dts-starfish-webos4.so` (modern).

Standalone build: `bash tools/build-dts-pipeline.sh --output build/dts-pipeline`.
Defaults: Podman, `localhost/nuvio-webos-sdk:latest`; override with
`NUVIO_CONTAINER_RUNTIME`/`NUVIO_DTS_SDK_IMAGE`. Runtime lookup uses `lib/`
beside the executable; `NUVIO_DTS_ADAPTER_DIR` supports isolated fixtures.

The worker serializes controls, feeds preroll after Load, and plays once both
streams and loadCompleted are ready. Seeks recreate the pipeline. Feed copies
packets: `Ok` releases them, `BufferFull` retries, and `Pending`/unknown replies
fail while retaining bytes through destruction. Ownership follows
[Kodi's reference](https://github.com/xbmc/xbmc/blob/c94ca95c20ce273cc76bda6e2ab9cadaa8504b20/xbmc/cores/VideoPlayer/MediaPipelineWebOS.cpp)
and requires physical-firmware verification.

`bash tests/dts_pipeline.sh` tests both ABIs against a fake native library.
See the [DTS guide](../../docs/features/dts/README.md) for limits and licenses,
and [DTS Debug builds](DEBUG.md) for a separate test app.
