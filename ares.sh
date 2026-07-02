#!/usr/bin/env bash
# Build the ROM in the libdragon Docker container, then launch Ares on the
# host (macOS) so debugf()/ISViewer output prints to this terminal.
#
# ROM-agnostic: builds, then runs whatever single .z64 lands in the project
# root. Drop this into any n64dev2/<project>/ that follows the same layout.
#
# Run this from the HOST shell, NOT inside the container:
#   ./run.sh
set -euo pipefail

cd "$(dirname "$0")"

ARES="../64Tools/ares-v141/ares.app/Contents/MacOS/ares"

# Build inside the container (uses the asdf `libdragon` wrapper).
# libdragon make

# Find the single ROM in the project root (after building).
shopt -s nullglob
roms=(*.z64)
shopt -u nullglob

if (( ${#roms[@]} == 0 )); then
    echo "error: no .z64 found in $(pwd) after build" >&2
    exit 1
elif (( ${#roms[@]} > 1 )); then
    echo "error: multiple .z64 files found, expected one:" >&2
    printf '  %s\n' "${roms[@]}" >&2
    exit 1
fi

ROM="$(pwd)/${roms[0]}"
echo ">> launching Ares with $ROM"
exec "$ARES" "$ROM"
