/*                            _ooOoo_
                            o8888888o
                             88" . "88
                             (| -_- |)
                              O\ = /O
                          ____/`---'\____
                        .   ' \\| |// `.
                         / \\||| : |||// \
                       / _||||| -:- |||||- \
                         | | \\\ - /// | |
                       | \_| ''\---/'' | |
                        \ .-\__ `-` ___/-. /
                     ___`. .' /--.--\ `. . __
                  ."" '< `.___\_<|>_/___.' >'"".
                 | | : `- \`.;`\ _ /`;.`/ - ` : | |
                   \ \ `-. \_ __\ /__ _/ .-` / /
           ======`-.____`-.___\_____/___.-`____.-'======
                              `=---='

           .............................................
                    佛祖保佑             永无BUG
            佛曰:
                    写字楼里写字间,写字间里程序员;
                    程序人员写程序,又拿程序换酒钱.
                    酒醒只在网上坐,酒醉还来网下眠;
                    酒醉酒醒日复日,网上网下年复年.
                    但愿老死电脑间,不愿鞠躬老板前;
                    奔驰宝马贵者趣,公交自行程序员.
                    别人笑我忒疯癫,我笑自己命太贱;
                    不见满街漂亮妹,哪个归得程序员?
*/

/* Copyright (C) 2026 YangLing */

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "lib/gadget/hexdump.h"
#include "lib/gadget/split.h"
#include "lib/gadget/fixtoa.h"

static char ob[256];
static unsigned int oi;
static void collect(char c)
{
	ob[oi++] = c;
	ob[oi] = 0;
}

static void expect(const char *want)
{
	if (strcmp(ob, want) != 0)
	{
		assert(!"printf output mismatch");
	}
	oi = 0;
	ob[0] = 0;
}

int main(void)
{
	/* ---------- printf ---------- */
	printf_init(collect);

	printf("hello");                  expect("hello");
	printf("%d", 42);                 expect("42");
	printf("%d", -42);                expect("-42");
	printf("%05d", -42);              expect("-0042");
	printf("%u", 4000000000u);        expect("4000000000");
	printf("%x", 0xdeadbeefu);        expect("deadbeef");
	printf("%p", (void *)0x20000000u);expect("0x20000000");
	printf("%s|%c|%%", "ab", 'Z');    expect("ab|Z|%");
	printf("[%8s]", "ab");            expect("[      ab]");
	printf("[%-8s]", "ab");           expect("[ab      ]");
	printf("[%08d]", 42);             expect("[00000042]");
	printf("a\nb");                   expect("a\r\nb");
	printf("%d %d %d", 1, 2, 3);      expect("1 2 3");
	printf("a%qb", 1);                expect("a%qb");

	char b[16];
	int n;

	n = snprintf(b, sizeof b, "%d-%s", 7, "x");
	assert(3 == n && 0 == strcmp(b, "7-x"));
	n = snprintf(b, 4, "%s", "abcdef");
	assert(6 == n && 0 == strcmp(b, "abc"));

	printf("%");                    expect("%");
	n = snprintf(b, sizeof b, "%99999999999d", 42);
	assert(9999 == n);
	assert(' ' == b[14] && '\0' == b[15]);

	/* ---------- sscanf ---------- */
	{
		int d; unsigned int u; char s[16]; char c;

		assert(1 == sscanf("42", "%d", &d) && 42 == d);
		assert(1 == sscanf("-42", "%d", &d) && -42 == d);
		assert(2 == sscanf("  99 red", "%d %s", &d, s)
		       && 99 == d && 0 == strcmp(s, "red"));
		assert(1 == sscanf("0x1F", "%x", &u) && 31 == u);
		assert(2 == sscanf("blink 300", "%15s %d", s, &d)
		       && 0 == strcmp(s, "blink") && 300 == d);
		assert(0 == sscanf("abc", "%d", &d));
		assert(1 == sscanf("a", "%c", &c) && 'a' == c);
		assert(1 == sscanf("x1", "x%d", &d) && 1 == d);
		assert(1 == sscanf("12345678901234567890123", "%15s", s)
		       && 0 == strcmp(s, "123456789012345"));
		assert(1 == sscanf("65535", "%u", &u) && 65535 == u);
		assert(1 == sscanf("blink abc", "%15s %d", s, &d));
	}

	/* ---------- string ---------- */
	{
		char b[32];

		assert(0 == strlen("") && 3 == strlen("abc"));
		assert(0 == strcmp("abc", "abc"));
		assert(strcmp("abc", "abd") < 0);
		assert(strcmp("abd", "abc") > 0);
		assert(0 == strcmp("", "") && strcmp("", "a") < 0);
		assert(strcmp("\x80", "a") > 0);
		assert(strncmp("\x80", "a", 1) > 0);
		assert(0 == strncmp("abcdef", "abcxyz", 3));
		assert(strncmp("abcdef", "abcxyz", 6) < 0);
		assert(0 == strncmp("abc", "abc", 10));
		assert(0 == strncmp("abc", "", 0));

		strcpy(b, "hello");
		assert('h' == b[0] && '\0' == b[5]);
		assert(b == strcpy(b, "x"));
		{
			char e[6];

			strcpy(e, "hello");
			assert('\0' == e[5]);
		}
		strcpy(b, "ab");
		strcat(b, "cd");
		assert(0 == strcmp(b, "abcd"));
		assert(b == strcat(b, ""));

		{
			const char *s = "hello";

			assert(s + 2 == strchr(s, 'l'));
			assert(s + 4 == strchr(s, 'o'));
			assert(NULL == strchr(s, 'z'));
			assert(s + 5 == strchr(s, '\0'));
			assert(s + 3 == strrchr(s, 'l'));
			assert(s + 5 == strrchr(s, '\0'));
			assert(s == strrchr(s, 'h'));
		}
		{
			const char *h = "abc abc";

			assert(h == strstr(h, "abc"));
			assert(h + 3 == strstr(h, " abc"));
			assert(h == strstr(h, ""));
			assert(NULL == strstr(h, "abcd"));
			assert(NULL == strstr("", "a"));
			assert("aaab" + 1 == strstr("aaab", "aab"));
		}
		{
			char t[] = "boot -t 60; app";

			assert(t == strtok(t, " ") && 0 == strcmp(t, "boot"));
			assert(0 == strcmp(strtok(NULL, " "), "-t"));
			assert(0 == strcmp(strtok(NULL, " "), "60;"));
			assert(0 == strcmp(strtok(NULL, " "), "app"));
			assert(NULL == strtok(NULL, " "));
			assert(NULL == strtok(NULL, " "));
			{
				char t2[] = "  , , ";

				assert(NULL == strtok(t2, " ,"));
			}
		}
	}

	/* ---------- mem ---------- */
	{
		char a[16], b[16];

		memset(a, 'x', sizeof a);
		assert('x' == a[0] && 'x' == a[15]);
		assert(a == memset(a, 0, 0));
		memset(a, 0x141, 2);
		assert('A' == a[0] && 'A' == a[1]);
		memset(a, 'x', sizeof a);
		memset(a, 0, 0);
		assert('x' == a[0]);
		memcpy(b, "123456789", 10);
		assert('\0' == b[9] && '1' == b[0]);
		assert(0 == memcmp(b, "123456789", 10));
		assert(0 == memcmp("abc", "abd", 2));
		assert(memcmp("abc", "abd", 3) < 0);
		assert(0 == memcmp("abc", "abc", 0));
		assert(memcmp("\x80", "\x01", 1) > 0);
		assert(a == memcpy(a, b, 0));

		{
			static unsigned char A[256], C[256], D[256], R[256], S[256];
			int n, od, os, i, diff;

			for (n = 0; n <= 64; n++)
			{
				for (od = 0; od < 4; od++)
				{
					for (os = 0; os < 4; os++)
					{
						for (i = 0; i < 256; i++)
						{
							A[i] = C[i] = (unsigned char)(i * 7 + 3);
							D[i] = R[i] = S[i] = 0xCC;
						}
						memcpy(D + od, A + os, n);
						memset(R + od, 0x5A, n);
						for (i = 0; i < 256; i++)
						{
							unsigned char want_m = (i >= od && i < od + n) ? A[i - od + os] : 0xCC;
							unsigned char want_s = (i >= od && i < od + n) ? 0x5A : 0xCC;

							assert(want_m == D[i]);
							assert(want_s == R[i]);
						}
					}
				}
			}

			for (n = 0; n <= 70; n++)
			{
				for (od = 0; od < 4; od++)
				{
					for (os = 0; os < 4; os++)
					{
						for (i = 0; i < 256; i++)
						{
							A[i] = C[i] = (unsigned char)(i * 7 + 3);
						}
						assert(0 == memcmp(A + od, C + od, n));
					}
				}
			}

			for (n = 1; n <= 70; n++)
			{
				for (od = 0; od < 4; od++)
				{
					for (i = 0; i < 256; i++)
					{
						A[i] = C[i] = (unsigned char)(i * 7 + 3);
						S[i] = A[i];
					}
					S[od + n - 1] = (unsigned char)(A[od + n - 1] ^ 0x80);
					{
						int want = (int)A[od + n - 1] - (int)S[od + n - 1];
						int got = memcmp(A + od, S + od, n);
						int sg = (got > 0) - (got < 0);
						int sw = (want > 0) - (want < 0);

						assert(sg == sw);
					}
					for (diff = 0; diff < n; diff++)
					{
						for (i = 0; i < 256; i++)
						{
							A[i] = (unsigned char)(i * 7 + 3);
							S[i] = A[i];
						}
						S[diff] = (unsigned char)(A[diff] ^ 0x80);
						{
							int want = (int)A[diff] - (int)S[diff];
							int got = memcmp(A, S, n);
							int sg = (got > 0) - (got < 0);
							int sw = (want > 0) - (want < 0);

							assert(sg == sw);
						}
					}
					if (n > 1)
					{
						int os2 = (od + 1) % 4;
						int got = memcmp(A + od, S + os2, n - 1);
						int ref = 0;

						for (i = 0; i < n - 1; i++)
						{
							if (A[od + i] != S[os2 + i])
							{
								ref = (int)A[od + i] - (int)S[os2 + i];
								break;
							}
						}
						{
							int sg = (got > 0) - (got < 0);
							int sw = (ref > 0) - (ref < 0);

							assert(sg == sw);
						}
					}
				}
			}
		}
		/* ---- memmove:重叠两方向 + 边界 ---- */
		{
			char m[32];

			strcpy(m, "0123456789ABCDE");
			memmove(m + 2, m, 10);
			assert('0' == m[2] && '9' == m[11]);
			assert(0 == strncmp(m + 2, "0123456789", 10));

			strcpy(m, "0123456789ABCDE");
			memmove(m, m + 2, 10);
			assert(0 == strncmp(m, "23456789AB", 10));

			strcpy(m, "0123456789");
			memmove(m + 3, m, 0);
			assert(0 == strcmp(m, "0123456789"));

			{
				char u[24] = "abcdefghijklmnopqrst";

				memmove(u + 5, u + 1, 12);
				assert(0 == strncmp(u + 5, "bcdefghijklm", 12));
			}
		}
	}

	/* ---------- 转换(strtol/strtoul/atoi) ---------- */
	{
		char *end;
		long v;

		v = strtol("42", &end, 10);
		assert(42 == v && '\0' == *end);
		v = strtol("  -7x", &end, 10);
		assert(-7 == v && 'x' == *end && end == "  -7x" + 4);
		v = strtol("0x1F", &end, 16);
		assert(31 == v && '\0' == *end);
		v = strtol("0x1F", &end, 0);
		assert(31 == v && '\0' == *end);
		v = strtol("017", &end, 0);
		assert(15 == v && '\0' == *end);
		v = strtol("101", &end, 2);
		assert(5 == v && '\0' == *end);
		v = strtol("z", &end, 36);
		assert(35 == v && '\0' == *end);
		v = strtol("zz", &end, 36);
		assert(35 * 36 + 35 == v && '\0' == *end);
		v = strtol("", &end, 10);
		assert(0 == v && end == (char *)"" && '\0' == *end);
		v = strtol("  ", &end, 10);
		assert(0 == v && end == (char *)"  ");
		v = strtol("hello", &end, 10);
		assert(0 == v && end == (char *)"hello");

		v = strtol("99999999999999999999", &end, 10);
		assert(LONG_MAX == v && '\0' == *end);
		v = strtol("-99999999999999999999", &end, 10);
		assert(LONG_MIN == v && '\0' == *end);
		v = strtol("18446744073709551620", &end, 10);
		assert(LONG_MAX == v && '\0' == *end);
		v = strtol("1000000000000000000", &end, 16);
		assert(LONG_MAX == v && '\0' == *end);

		{
			unsigned long uv = strtoul("-1", &end, 10);

			assert(ULONG_MAX == uv && '\0' == *end);
			uv = strtoul("0xFFFFFFFFFFFFFFFF", &end, 16);
			assert(ULONG_MAX == uv && '\0' == *end);
			uv = strtoul("3000000000", &end, 10);
			assert(3000000000ul == uv && '\0' == *end);
			uv = strtoul("99999999999999999999999", &end, 10);
			assert(ULONG_MAX == uv && '\0' == *end);
		}

		assert(42 == atoi("42"));
		assert(-13 == atoi(" -13"));
		assert(0 == atoi("abc"));
		assert(12 == atoi("12abc"));
	}

	/* ---------- split_argv ---------- */
	{
		char line[64];
		char *av[8];
		int argc;

		strcpy(line, "ee w 0x50 hello");
		argc = split_argv(line, av, 8);
		assert(4 == argc);
		assert(0 == strcmp(av[0], "ee"));
		assert(0 == strcmp(av[1], "w"));
		assert(0 == strcmp(av[2], "0x50"));
		assert(0 == strcmp(av[3], "hello"));

		strcpy(line, "   leading and   trailing   ");
		argc = split_argv(line, av, 8);
		assert(3 == argc);
		assert(0 == strcmp(av[0], "leading"));
		assert(0 == strcmp(av[1], "and"));
		assert(0 == strcmp(av[2], "trailing"));

		strcpy(line, "a\tb\r\nc");
		argc = split_argv(line, av, 8);
		assert(3 == argc);
		assert(0 == strcmp(av[0], "a") && 0 == strcmp(av[1], "b"));
		assert(0 == strcmp(av[2], "c"));

		strcpy(line, "one two three four five");
		argc = split_argv(line, av, 3);
		assert(3 == argc && 0 == strcmp(av[2], "three"));

		strcpy(line, "");
		argc = split_argv(line, av, 8);
		assert(0 == argc);

		strcpy(line, "   ");
		argc = split_argv(line, av, 8);
		assert(0 == argc);

		strcpy(line, "solo");
		argc = split_argv(line, av, 8);
		assert(1 == argc && 0 == strcmp(av[0], "solo"));

		strcpy(line, "a b");
		argc = split_argv(line, av, 0);
		assert(0 == argc);
	}

	/* ---------- hexdump_sn ---------- */
	{
		char ob[128];

		unsigned char full[16] = {'a','b','c','d','e','f','g','h',
		                          'I','J','K','L','M','N','O','P'};

		hexdump_sn(ob, sizeof ob, full, 16, 0x00000010u);
		assert(0 == strcmp(ob,
		    "00000010  61 62 63 64 65 66 67 68  49 4a 4b 4c 4d 4e 4f 50"
		    "  |abcdefghIJKLMNOP|"));

		/* 不可打印 -> '.';0x7f 与 0x20 边界 */
		unsigned char mix[4] = {0x00, 0x1F, 0x20, 0x7F};

		hexdump_sn(ob, sizeof ob, mix, 4, 0);
		assert(0 == strcmp(ob,
		    "00000000  00 1f 20 7f"
		    "                                       |.. .|"));

		unsigned char eight[8] = {1,2,3,4,5,6,7,8};

		hexdump_sn(ob, sizeof ob, eight, 8, 0);
		assert(0 == strcmp(ob,
		    "00000000  01 02 03 04 05 06 07 08"
		    "                           |........|"));

		/* len=0:空串 */
		hexdump_sn(ob, sizeof ob, mix, 0, 0x12345678u);
		assert('\0' == ob[0]);

		/* 首地址偏移满宽 8 位十六进制 */
		unsigned char one[1] = {0xFF};

		hexdump_sn(ob, sizeof ob, one, 1, 0xDEADBE00u);
		assert(0 == strcmp(ob,
		    "deadbe00  ff"
		    "                                                |.|"));
	}

	/* ---------- fixtoa ---------- */
	{
		char b[16];

		assert(0 == fixtoa(b, sizeof b, 4500, 2) && 0 == strcmp(b, "45.00"));
		assert(0 == fixtoa(b, sizeof b, 0, 2) && 0 == strcmp(b, "0.00"));
		/* 整数部分为 0 的负数, 负号不能丢 */
		assert(0 == fixtoa(b, sizeof b, -50, 2) && 0 == strcmp(b, "-0.50"));
		assert(0 == fixtoa(b, sizeof b, -5, 2) && 0 == strcmp(b, "-0.05"));
		assert(0 == fixtoa(b, sizeof b, 5, 2) && 0 == strcmp(b, "0.05"));
		assert(0 == fixtoa(b, sizeof b, -1, 2) && 0 == strcmp(b, "-0.01"));
		assert(0 == fixtoa(b, sizeof b, -17999, 2) && 0 == strcmp(b, "-179.99"));
		assert(0 == fixtoa(b, sizeof b, 123456, 3) && 0 == strcmp(b, "123.456"));
		assert(0 == fixtoa(b, sizeof b, 1, 6) && 0 == strcmp(b, "0.000001"));
		assert(0 == fixtoa(b, sizeof b, 999, 2) && 0 == strcmp(b, "9.99"));
		assert(0 == fixtoa(b, sizeof b, 1001, 2) && 0 == strcmp(b, "10.01"));

		/* decimals=0: 纯整数, 无小数点 */
		assert(0 == fixtoa(b, sizeof b, 45, 0) && 0 == strcmp(b, "45"));
		assert(0 == fixtoa(b, sizeof b, -45, 0) && 0 == strcmp(b, "-45"));
		assert(0 == fixtoa(b, sizeof b, 0, 0) && 0 == strcmp(b, "0"));

		/* int32 边界: INT32_MIN 取负不溢出 */
		assert(0 == fixtoa(b, sizeof b, INT32_MAX, 3)
		       && 0 == strcmp(b, "2147483.647"));
		assert(0 == fixtoa(b, sizeof b, INT32_MIN, 2)
		       && 0 == strcmp(b, "-21474836.48"));

		/* decimals 越界 -> -EINVAL 且置空串 */
		b[0] = 'x';
		assert(-EINVAL == fixtoa(b, sizeof b, 1, 7) && '\0' == b[0]);
		b[0] = 'x';
		assert(-EINVAL == fixtoa(b, sizeof b, 1, -1) && '\0' == b[0]);

		/* buf 不足 -> -ENOSPC 且置空串; outsz=0 一个字节都不写 */
		b[0] = 'x';
		assert(-ENOSPC == fixtoa(b, 4, 4500, 2) && '\0' == b[0]);
		assert(-ENOSPC == fixtoa(NULL, 0, 4500, 2));
	}

	return 0;
}
