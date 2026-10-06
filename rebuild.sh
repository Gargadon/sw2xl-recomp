#!/usr/bin/env bash
set -euo pipefail

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
for file in \
    "$script_dir/out/build/linux-amd64-release/CMakeCache.txt" \
    "$script_dir/generated/default/sources.cmake" \
    "$script_dir/generated/default/dll_targets.cmake" \
    "$script_dir/generated/sw2xl_us/sources.cmake"; do
    [[ -f "$file" ]] || { echo "Initial build is missing: $file. Run build.sh first." >&2; exit 1; }
done

cd -- "$script_dir"
cmake --preset linux-amd64-release -DSW2_PREPARE_GAME=OFF
cmake --build --preset linux-amd64-release -j 6
