#!/bin/sh

cd "$(dirname "$0")"
gcc -Wall -Wextra -fno-builtin -fsanitize=address,undefined -Os \
    -I../.. -I../../lib/attolibc \
    test_display.c ../../pal/display.c -o /tmp/t-display \
&& /tmp/t-display && echo "PASS test_display"
