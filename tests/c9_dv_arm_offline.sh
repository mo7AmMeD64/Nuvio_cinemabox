#!/bin/bash
# Build only, using the cached SDK; never contact the TV or pull an image.
set -euo pipefail
cd "$(dirname "$0")/.."
dir=$(mktemp -d "${TMPDIR:-/tmp}/nuvio-c9-dv-arm.XXXXXX")
trap 'rm -rf "$dir"' EXIT
printf '%s\n' '#!/bin/bash' \
  'if [ "${1:-}" = run ]; then shift; exec docker run --network none --pull never "$@"; fi' \
  'exec docker "$@"' > "$dir/docker-offline"
chmod +x "$dir/docker-offline"
CARGO_NET_OFFLINE=true NUVIO_CONTAINER_RUNTIME="$dir/docker-offline" bash tools/arm.sh --build
