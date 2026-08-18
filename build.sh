#!/usr/bin/env sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
export SDL3_DIR="${SDL3_DIR:-$HOME/.local/sdl3-prefix/lib/cmake/SDL3}"
cmake -S "$ROOT" -B "$ROOT/build" -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build "$ROOT/build" --config Release --parallel
