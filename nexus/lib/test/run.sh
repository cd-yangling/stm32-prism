#!/bin/sh

cd "$(dirname "$0")"
for t in test_lib test_fifo
do
	gcc -Wall -Wextra -fno-builtin -fsanitize=address,undefined -Os \
	    -I../.. -I../../lib/attolibc \
	    $t.c ../../lib/attolibc/*.c ../../lib/gadget/*.c -o /tmp/t-$t \
	&& /tmp/t-$t && echo "PASS $t" || exit 1
done
