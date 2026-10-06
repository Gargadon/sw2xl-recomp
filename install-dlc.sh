#!/usr/bin/env bash
set -euo pipefail

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
dlc_root=${1:-'/mnt/d/Samurai Warriors 2 XL'}

if [[ ${1:-} == --dlc-root ]]; then
    [[ $# -ge 2 ]] || { echo 'Usage: install-dlc.sh [--dlc-root] PATH' >&2; exit 2; }
    dlc_root=$2
elif (( $# > 1 )); then
    echo 'Usage: install-dlc.sh [--dlc-root] PATH' >&2
    exit 2
fi

[[ -d "$dlc_root" ]] || { echo "DLC directory not found: $dlc_root" >&2; exit 1; }
dlc_root=$(cd -- "$dlc_root" && pwd)
installer="$script_dir/out/build/linux-amd64-release/sw2_install_dlc"
[[ -f "$installer" ]] || { echo 'DLC installer is missing. Run build.sh first.' >&2; exit 1; }
export LD_LIBRARY_PATH="$(dirname -- "$installer")${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
exec "$installer" "$(dirname -- "$script_dir")/Samurai Warriors 2 (USA, Europe)" "$dlc_root" "$script_dir/userdata"
