#!/bin/sh
set -e
cd "$(dirname "$0")"
echo 'APP_OFFSET = 0x8000' > offset.local.mk
rm -f boot.elf boot.map
make CHIP=f103c8t6 "$@"
