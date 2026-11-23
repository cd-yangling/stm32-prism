#!/bin/sh
set -e
cd "$(dirname "$0")"
echo 'APP_OFFSET = 0x80000' > offset.local.mk
rm -f boot.elf boot.map
make CHIP=f405rgt6 "$@"
