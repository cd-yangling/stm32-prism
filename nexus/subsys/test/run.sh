#!/bin/sh

cd "$(dirname "$0")"
gcc -Wall -Wextra -fno-builtin -fsanitize=address,undefined -Os \
    -I../.. -I../../lib/attolibc \
    test_gfx.c ../../subsys/gfx.c ../../subsys/font_8x16.c \
    ../../pal/display.c -o /tmp/t-gfx \
&& /tmp/t-gfx && echo "PASS test_gfx"
