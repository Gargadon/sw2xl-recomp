#!/usr/bin/env bash
set -euo pipefail

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
sdk_root=$(dirname -- "$script_dir")

cmake --preset linux-amd64-release -S "$script_dir" \
    "-DREXSDK_DIR=$sdk_root" -DSW2_PGO=AUTO
cmake --build --preset linux-amd64-release -j 6
