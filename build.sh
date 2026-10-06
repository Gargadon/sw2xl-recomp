#!/usr/bin/env bash
set -euo pipefail

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
sdk_root=$(dirname -- "$script_dir")

cmake --preset linux-amd64-release -S "$script_dir" \
    "-DREXSDK_DIR=$sdk_root" -DSW2_PGO=AUTO -DSW2_PREPARE_GAME=ON
cmake --build --preset linux-amd64-release --target samurai_warriors_2_codegen -j 6
cmake --preset linux-amd64-release -S "$script_dir"
cmake --build --preset linux-amd64-release -j 6
