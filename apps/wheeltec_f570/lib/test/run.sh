#!/bin/sh

cd "$(dirname "$0")"
gcc -Wall -Wextra -fno-builtin -fsanitize=address,undefined -Os \
    -I../.. -I../../../../nexus -I../../../../nexus/lib/attolibc \
    test_deframer.c ../deframer.c \
    ../../../../nexus/lib/attolibc/*.c -o /tmp/t-test_deframer \
&& /tmp/t-test_deframer && echo "PASS test_deframer" || exit 1
