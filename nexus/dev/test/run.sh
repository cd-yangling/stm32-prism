#!/bin/sh

cd "$(dirname "$0")"
gcc -Wall -Wextra -fno-builtin -fsanitize=address,undefined -Os \
    -I../.. -I../../lib/attolibc \
    test_ssd1306.c ../../dev/ssd1306.c ../../pal/i2c.c ../../pal/display.c \
    -o /tmp/t-ssd1306 \
&& /tmp/t-ssd1306 && echo "PASS test_ssd1306"
