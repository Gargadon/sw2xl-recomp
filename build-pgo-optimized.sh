#!/usr/bin/env bash
set -euo pipefail

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
sdk_root=$(dirname -- "$script_dir")
profile_dir="$script_dir/pgo/raw"
merged_profile="$script_dir/pgo/sw2.profdata"

shopt -s nullglob
raw_profiles=("$profile_dir"/*.profraw)
if ((${#raw_profiles[@]} == 0)); then
    echo 'No training profiles found. Run build-pgo-instrumented.sh and run-pgo-training.sh first.' >&2
    exit 1
fi

mkdir -p -- "$(dirname -- "$merged_profile")"
llvm-profdata merge -o "$merged_profile" "${raw_profiles[@]}"
[[ -f "$merged_profile" ]] || { echo "llvm-profdata did not create: $merged_profile" >&2; exit 1; }

cmake --preset linux-amd64-release -S "$script_dir" \
    "-DREXSDK_DIR=$sdk_root" -DSW2_PGO=USE -DSW2_PREPARE_GAME=ON \
    "-DSW2_PGO_PROFILE=$merged_profile"
cmake --build --preset linux-amd64-release --target samurai_warriors_2_codegen -j 6
cmake --preset linux-amd64-release -S "$script_dir"
cmake --build --preset linux-amd64-release -j 6

echo 'PGO optimized build ready. Launch it with ./run.sh'
