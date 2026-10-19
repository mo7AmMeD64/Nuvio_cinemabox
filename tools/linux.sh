#!/bin/bash
# Native Linux UI preview. Video uses the same stubs as the macOS build.
#   bash tools/linux.sh           # build and run
#   bash tools/linux.sh --build   # build only
#   bash tools/linux.sh --preview # browse UI without an account
set -euo pipefail
cd "$(dirname "$0")/.."

BUILD_ONLY=0
while [ "$#" -gt 0 ]; do
  case "$1" in
    --build) BUILD_ONLY=1; shift ;;
    --preview) export NUVIO_UI_PREVIEW=1; shift ;;
    *) break ;;
  esac
done
CC="${CC:-cc}"
DEPS=(sdl2 SDL2_image SDL2_ttf glesv2 egl zlib)
if ! pkg-config --exists "${DEPS[@]}"; then
  echo "linux.sh: install SDL2, SDL2_image, SDL2_ttf, GLES2, EGL and zlib development packages (see README.md)." >&2
  exit 2
fi

ENVF=$(mktemp)
trap 'rm -f "$ENVF"' EXIT
bash tools/env.sh --env-file "$ENVF"
DEFINES=(-DNV_LINUX_DESKTOP)
while IFS='=' read -r name value; do
  # Pass each definition as one argument, including literal C string quotes.
  value=${value//\\/\\\\}
  value=${value//\"/\\\"}
  DEFINES+=("-D$name=\"$value\"")
done < "$ENVF"
read -r -a DEP_CFLAGS <<< "$(pkg-config --cflags "${DEPS[@]}")"
read -r -a DEP_LIBS <<< "$(pkg-config --libs "${DEPS[@]}")"
mkdir -p build/linux
"$CC" src/*.c src/dts/*.c -Isrc -o build/linux/nuvio-native-legacy -O1 -g \
  "${DEFINES[@]}" "${DEP_CFLAGS[@]}" "${DEP_LIBS[@]}" -ldl -pthread -lm
echo "Linux UI build: $PWD/build/linux/nuvio-native-legacy"
if [ "$BUILD_ONLY" = 1 ]; then exit 0; fi

DEFAULT_DATA="${XDG_DATA_HOME:-$HOME/.local/share}/nuvio-native-legacy-linux"
if [ "${NUVIO_UI_PREVIEW:-0}" = 1 ]; then
  DEFAULT_DATA="${DEFAULT_DATA}-preview"
fi
export NUVIO_DADOS="${NUVIO_DADOS:-$DEFAULT_DATA}"
mkdir -p "$NUVIO_DADOS"
rm -f "$ENVF"
trap - EXIT
exec build/linux/nuvio-native-legacy "$PWD/deploy/app/art" "$@"
