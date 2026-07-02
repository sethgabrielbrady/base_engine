#!/usr/bin/env bash
# Upload the built ROM to the SummerCart64 via sc64deployer, then drop into
# its debug terminal (so libdragon debugf()/stdout prints to this terminal).
#
# ROM-agnostic: uploads whatever single .z64 sits in the project root. Drop
# this into any n64dev2/<project>/ that follows the same layout.
#
# Run from the HOST shell:
#   ./deploy.sh
set -euo pipefail

cd "$(dirname "$0")"

SC64="../64Tools/scdeployer/sc64deployer"

# Find the single ROM in the project root.
shopt -s nullglob
roms=(*.z64)
shopt -u nullglob

if (( ${#roms[@]} == 0 )); then
    echo "error: no .z64 found in $(pwd)" >&2
    echo "       build it first (e.g. 'libdragon make' or './run.sh')." >&2
    exit 1
elif (( ${#roms[@]} > 1 )); then
    echo "error: multiple .z64 files found, expected one:" >&2
    printf '  %s\n' "${roms[@]}" >&2
    exit 1
fi

ROM="$(pwd)/${roms[0]}"

echo ">> uploading $ROM"
"$SC64" upload "$ROM"

echo ">> upload OK, starting debug terminal (ctrl-c to exit)"
exec "$SC64" debug
