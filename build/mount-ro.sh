#!/bin/sh
# mount-ro.sh <src> [dst] — read-only bind-mount src onto dst
# e.g. sudo build/mount-ro.sh ~/code/STM32CubeF1 third_party/ST/STM32Cube/STM32CubeF1
set -eu

src=${1:?usage: sudo $0 <src> [dst]}
[ -d "$src" ] || { echo "src not found: $src" >&2; exit 1; }
src=$(realpath "$src")

base=${src%/}
base=${base##*/}
dst=${2:-$base}
mkdir -p "$dst"
dst=$(realpath "$dst")

if findmnt "$dst" >/dev/null; then
    echo "already mounted: $dst"
    exit 0
fi

mount --bind "$src" "$dst"
mount -o remount,ro,bind "$dst"
echo "ro: $src -> $dst"
