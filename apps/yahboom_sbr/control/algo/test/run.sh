#!/bin/sh

cd "$(dirname "$0")"
for t in test_mul_shr_carry
do
	gcc -Wall -Wextra -fsanitize=address,undefined -Os \
	    -I../../.. \
	    $t.c -o /tmp/t-$t \
	&& /tmp/t-$t && echo "PASS $t" || exit 1
done
