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
#include <stdint.h>
#include "lib/gadget/fxp_mul.h"

static uint32_t rng_state = 0x12345678u;

static int32_t rng16(void)
{
	rng_state = rng_state * 1664525u + 1013904223u;
	return (int32_t)(rng_state >> 16) - 32768;
}

static void mirror(void)
{
	static const int shrs[] = {8, 16, 19, 24};

	for (unsigned s = 0; s < sizeof(shrs) / sizeof(shrs[0]); s++)
	{
		int shr = shrs[s];

		for (int i = 0; i < 20000; i++)
		{
			int32_t v = rng16();
			int32_t num = rng16();
			int32_t ra = 0;
			int32_t rb = 0;
			int64_t ra64 = 0;
			int64_t rb64 = 0;

			assert(fxp_mul32_rem(&ra, v, num, shr)
				== -fxp_mul32_rem(&rb, -v, num, shr));
			assert(ra == -rb);
			assert(fxp_mul64_rem(&ra64, (int64_t)v * 257, (int64_t)num * 263, shr)
				== -fxp_mul64_rem(&rb64, -(int64_t)v * 257,
					(int64_t)num * 263, shr));
			assert(ra64 == -rb64);
		}
	}
}

static void conservation(void)
{
	{
		int32_t rem = 0;
		int64_t sp = 0;
		int64_t sq = 0;

		for (int i = 0; i < 4000; i++)
		{
			int32_t v = rng16();
			int32_t num = rng16();

			sq += fxp_mul32_rem(&rem, v, num, 16);
			sp += (int64_t)v * num;
		}
		assert(sq * (((int64_t)1) << 16) == sp - rem);
	}

	{
		int64_t rem = 0;
		int64_t sp = 0;
		int64_t sq = 0;

		for (int i = 0; i < 4000; i++)
		{
			int64_t v = (int64_t)rng16() * 100003;
			int64_t num = (int64_t)rng16() * 7;

			sq += fxp_mul64_rem(&rem, v, num, 19);
			sp += v * num;
		}
		assert(sq * (((int64_t)1) << 19) == sp - rem);
	}
}

static void guard_domain(void)
{
	int32_t rem = 7;

	assert(0 == fxp_mul32(1, 1, 0));
	assert(0 == fxp_mul32(1, 1, 31));
	assert(0 == fxp_mul32(1, 1, 32));
	assert(0 == fxp_mul64(1, 1, 0));
	assert(0 == fxp_mul64(1, 1, 63));
	assert(0 == fxp_mul64(1, 1, 64));
	assert(0 == fxp_mul32_rem(&rem, 1, 1, 0));
	assert(7 == rem);

	assert(0 == fxp_mul32(536870912, 1, 30));
	assert(0 == fxp_mul32(-536870912, 1, 30));
	assert(0 == fxp_mul32(536870911, 1, 30));
	assert(0 == fxp_mul64(2305843009213693952LL, 1, 62));
	assert(0 == fxp_mul64(2305843009213693952LL, -1, 62));
}

static void rem_bound(void)
{
	static const int shrs[] = {1, 8, 16, 30};

	for (unsigned s = 0; s < sizeof(shrs) / sizeof(shrs[0]); s++)
	{
		int shr = shrs[s];
		int32_t h = ((int32_t)1) << (shr - 1);
		int64_t h64 = ((int64_t)1) << (shr - 1);

		for (int r = 0; r < 50; r++)
		{
			int32_t rem = rng16() * 3;
			int64_t rem64 = (int64_t)rng16() * 1000;

			for (int i = 0; i < 400; i++)
			{
				(void)fxp_mul32_rem(&rem, rng16(), rng16(), shr);
				assert(h >= rem && -h <= rem);

				(void)fxp_mul64_rem(&rem64,
						(int64_t)rng16() * 100003,
						(int64_t)rng16() * 7, shr);
				assert(h64 >= rem64 && -h64 <= rem64);
			}
		}
	}
}

static void golden(void)
{
	assert(0 == fxp_mul32(1, 26, 8));
	assert(0 == fxp_mul32(1, 128, 8));
	assert(0 == fxp_mul32(0, 32767, 16));
	assert(0 == fxp_mul32(32767, 0, 1));
	assert(0 == fxp_mul32(1, 32768, 16));
	assert(0 == fxp_mul32(-1, 32768, 16));
	assert(2 == fxp_mul32(98304, 1, 16));
	assert(-2 == fxp_mul32(-98304, 1, 16));
	assert(-1 == fxp_mul32(-98303, 1, 16));
	assert(0 == fxp_mul32(-32767, 1, 16));
	assert(-1 == fxp_mul32(-32769, 1, 16));
	assert(0 == fxp_mul64(1LL << 40, 1, 41));
	assert(0 == fxp_mul64(1LL << 40, -1, 41));

	assert(2 == fxp_mul32(1, 384, 8));
	assert(2 == fxp_mul32(1, 640, 8));
	assert(4 == fxp_mul32(1, 896, 8));
	assert(0 == fxp_mul32(-1, 128, 8));
	assert(-2 == fxp_mul32(-1, 384, 8));
	assert(-2 == fxp_mul32(-1, 640, 8));
	assert(-4 == fxp_mul32(-1, 896, 8));
	assert(2 == fxp_mul64(3LL << 40, 1, 41));
	assert(-2 == fxp_mul64(3LL << 40, -1, 41));

	{
		int32_t rem = 0;
		int32_t q = fxp_mul32_rem(&rem, 1, 128, 8);

		assert(0 == q);
		assert(128 == rem);
	}

	assert(2 == fxp_mul32(1855669497, 1, 30));
	assert(2 == fxp_mul32(1610612736, 1, 30));
	assert(-2 == fxp_mul32(-1855669497, 1, 30));
	assert(32768 == fxp_mul32(2147450880, 1, 16));
	assert(1073741824 == fxp_mul32(2147483647, 1, 1));
	assert(2 == fxp_mul64(6917529027641081856LL, 1, 62));
	assert(2 == fxp_mul64(9223372036854775807LL, 1, 62));

	{
		int32_t rem = 0;
		int32_t q = fxp_mul32_rem(&rem, 1855669497, 1, 30);

		assert(2 == q);
		assert(-291814151 == rem);
		assert((int64_t)q * (((int64_t)1) << 30) + rem == 1855669497);
	}

	{
		int32_t rem = 0;
		int64_t sq = 0;

		for (int i = 0; i < 65536; i++)
		{
			sq += fxp_mul32_rem(&rem, 1, 1, 16);
		}
		assert(1 == sq);
		assert(0 == rem);
	}
}

static int64_t ref32(int32_t acc, int shr)
{
	int64_t	q = ((int64_t)acc) >> shr;
	int64_t	r = ((int64_t)acc) - q * (((int64_t)1) << shr);
	int64_t	h = ((int64_t)1) << (shr - 1);

	if ((h < r) || ((h == r) && (q & 1)))
	{
		q++;
	}
	return q;
}

static int64_t ref64(int64_t acc, int shr)
{
	__int128	q = ((__int128)acc) >> shr;
	__int128	r = ((__int128)acc) - q * (((__int128)1) << shr);
	__int128	h = ((__int128)1) << (shr - 1);

	if ((h < r) || ((h == r) && (q & 1)))
	{
		q++;
	}
	return (int64_t)q;
}

static void window_exhaustive(void)
{
	for (int shr = 1; 30 >= shr; shr++)
	{
		int64_t	h = ((int64_t)1) << (shr - 1);
		int64_t	lo = ((int64_t)INT32_MAX) + 1 - h;
		int64_t	step = 1;
		int64_t	a;

		if (20 < shr)
		{
			step = h / 65536 + 1;
		}

		for (a = lo; ((int64_t)INT32_MAX) >= a; a += step)
		{
			int32_t	v = (int32_t)a;
			int32_t	rem = 0;
			int32_t	q = fxp_mul32_rem(&rem, v, 1, shr);

			assert((int64_t)q == ref32(v, shr));
			assert((int64_t)q * (((int64_t)1) << shr) + rem == a);
			assert(h >= rem && -h <= rem);
			assert(-q == fxp_mul32(-v, 1, shr));
		}

		{
			int32_t	rem = 0;
			int32_t	q = fxp_mul32_rem(&rem, INT32_MAX, 1, shr);

			assert((int64_t)q == ref32(INT32_MAX, shr));
			assert((int64_t)q * (((int64_t)1) << shr) + rem
					== (int64_t)INT32_MAX);
		}

		assert(fxp_mul32(INT32_MIN, 1, shr) == (int32_t)ref32(INT32_MIN, shr));
	}
}

static void tie_lattice(void)
{
	for (int shr = 1; 30 >= shr; shr++)
	{
		int64_t	mod = ((int64_t)1) << shr;
		int64_t	h = mod >> 1;
		int64_t	span = ((int64_t)1) << (32 - shr);
		int64_t	stride = (14 > shr) ? (((int64_t)1) << (14 - shr)) : 1;
		int64_t	k;

		for (k = -span; span >= k; k += stride)
		{
			int64_t	tie = k * mod + h;

			if (((int64_t)INT32_MIN) > tie || ((int64_t)INT32_MAX) < tie)
			{
				continue;
			}

			{
				int32_t	v = (int32_t)tie;
				int32_t	rem = 0;
				int32_t	q = fxp_mul32_rem(&rem, v, 1, shr);

				assert((int64_t)q == ref32(v, shr));
				assert((int64_t)q * mod + rem == tie);
				assert(h >= rem && -h <= rem);
				assert(-q == fxp_mul32(-v, 1, shr));
			}
		}
	}
}

static void window64(void)
{
	uint32_t	rs = 20261110u;

	for (int shr = 1; 62 >= shr; shr++)
	{
		int64_t	mod = ((int64_t)1) << shr;
		int64_t	h = mod >> 1;

		for (int i = 0; 256 > i; i++)
		{
			int64_t	hi;
			int64_t	rnd;
			int64_t	a;
			int64_t	rem = 0;
			int64_t	q;

			rs = rs * 1664525u + 1013904223u;
			hi = (int64_t)(rs >> 1);
			rs = rs * 1664525u + 1013904223u;
			rnd = (hi << 31) | (int64_t)(rs >> 1);
			a = ((int64_t)INT64_MAX) - rnd % h;

			q = fxp_mul64_rem(&rem, a, 1, shr);
			assert(q == ref64(a, shr));
			assert(((__int128)q) * mod + rem == a);
			assert(h >= rem && -h <= rem);
			assert(-q == fxp_mul64(-a, 1, shr));
		}

		{
			int64_t	rem = 0;
			int64_t	q = fxp_mul64_rem(&rem, INT64_MAX, 1, shr);

			assert(q == ref64(INT64_MAX, shr));
			assert(((__int128)q) * mod + rem == (__int128)INT64_MAX);
		}

		assert(fxp_mul64(INT64_MIN, 1, shr) == ref64(INT64_MIN, shr));
	}
}

static void corner_product(void)
{
	static const int32_t	cs[10] =
	{
		INT32_MIN, INT32_MIN + 1, -3, -2, -1, 0, 1, 2, INT32_MAX - 1, INT32_MAX
	};

	for (int shr = 1; 30 >= shr; shr++)
	{
		int64_t	mod = ((int64_t)1) << shr;
		int64_t	h = mod >> 1;
		int64_t	rr[6] = { -h, -h + 1, 0, 1, h - 1, h };

		for (int a = 0; 10 > a; a++)
		{
			for (int b = 0; 10 > b; b++)
			{
				int64_t	p = (int64_t)cs[a] * cs[b];

				if (((int64_t)INT32_MIN) > p || ((int64_t)INT32_MAX) < p)
				{
					continue;
				}

				for (int c = 0; 6 > c; c++)
				{
					int64_t	acc = p + rr[c];

					if (((int64_t)INT32_MIN) > acc
							|| ((int64_t)INT32_MAX) < acc)
					{
						continue;
					}

					{
						int32_t	rem = (int32_t)rr[c];
						int32_t	q = fxp_mul32_rem(&rem, cs[a], cs[b], shr);

						assert((int64_t)q == ref32((int32_t)acc, shr));
						assert((int64_t)q * mod + rem == acc);
					}
				}
			}
		}
	}
}

static void corner_product64(void)
{
	static const int64_t	cs[10] =
	{
		INT64_MIN, INT64_MIN + 1, -3, -2, -1, 0, 1, 2, INT64_MAX - 1, INT64_MAX
	};

	for (int shr = 1; 62 >= shr; shr++)
	{
		__int128	mod = ((__int128)1) << shr;
		__int128	h = mod >> 1;
		__int128	rr[6] = { -h, -h + 1, 0, 1, h - 1, h };

		for (int a = 0; 10 > a; a++)
		{
			for (int b = 0; 10 > b; b++)
			{
				__int128	p = (__int128)cs[a] * cs[b];

				if (((__int128)INT64_MIN) > p || ((__int128)INT64_MAX) < p)
				{
					continue;
				}

				for (int c = 0; 6 > c; c++)
				{
					__int128	acc = p + rr[c];

					if (((__int128)INT64_MIN) > acc
							|| ((__int128)INT64_MAX) < acc)
					{
						continue;
					}

					{
						int64_t	rem = (int64_t)rr[c];
						int64_t	q = fxp_mul64_rem(&rem, cs[a], cs[b], shr);

						assert(q == ref64((int64_t)acc, shr));
						assert((__int128)q * mod + rem == acc);
					}
				}
			}
		}
	}
}

static void tie_inject(void)
{
	static const int64_t	ks[8] = { 0, 1, 2, 3, 7, 1000, 65535, 1 << 20 };

	for (int shr = 1; 30 >= shr; shr++)
	{
		int64_t	mod = ((int64_t)1) << shr;
		int64_t	h = mod >> 1;

		for (int i = 0; 8 > i; i++)
		{
			int64_t	k = ks[i];

			if ((int64_t)INT32_MAX / mod < k)
			{
				continue;
			}

			for (int s = 0; 2 > s; s++)
			{
				int64_t	r0 = s ? h : -h;
				int64_t	acc = k * mod + r0;

				if (((int64_t)INT32_MIN) > acc || ((int64_t)INT32_MAX) < acc)
				{
					continue;
				}

				{
					int32_t	rem = (int32_t)r0;
					int32_t	q = fxp_mul32_rem(&rem, (int32_t)k,
							(int32_t)mod, shr);

					assert((int64_t)q == ref32((int32_t)acc, shr));
					assert((int64_t)q * mod + rem == acc);
				}
			}
		}
	}
}

static void push_limit(void)
{
	for (int i = 0; 3 > i; i++)
	{
		int	shr = (0 == i) ? 1 : ((1 == i) ? 16 : 30);
		int64_t	mod = ((int64_t)1) << shr;
		int64_t	h = mod >> 1;
		int64_t	p = ((int64_t)INT32_MAX) - h - 1;
		int32_t	rem_a = 0;
		int32_t	rem_b = 0;
		int32_t	v = (int32_t)p;

		for (int b = 0; 4096 > b; b++)
		{
			int64_t	acc_a = p + rem_a;
			int64_t	acc_b = -p + rem_b;
			int32_t	qa = fxp_mul32_rem(&rem_a, v, 1, shr);
			int32_t	qb = fxp_mul32_rem(&rem_b, -v, 1, shr);

			assert((int64_t)qa == ref32((int32_t)acc_a, shr));
			assert((int64_t)qb == ref32((int32_t)acc_b, shr));
			assert((int64_t)qa * mod + rem_a == acc_a);
			assert((int64_t)qb * mod + rem_b == acc_b);
			assert(h >= rem_a && -h <= rem_a);
			assert(qa == -qb);
			assert(rem_a == -rem_b);
		}
	}
}

static void null_consistency(void)
{
	static const int	shrs[4] = { 1, 8, 16, 30 };

	assert(0 == fxp_mul32_rem(NULL, 1, 1, 0));
	assert(0 == fxp_mul32_rem(NULL, 1, 1, 31));
	assert(0 == fxp_mul64_rem(NULL, 1, 1, 0));
	assert(0 == fxp_mul64_rem(NULL, 1, 1, 63));

	for (int s = 0; 4 > s; s++)
	{
		int	shr = shrs[s];

		for (int i = 0; 20000 > i; i++)
		{
			int32_t	v = rng16();
			int32_t	num = rng16();
			int32_t	zero = 0;
			int32_t	q = fxp_mul32(v, num, shr);

			assert(q == fxp_mul32_rem(NULL, v, num, shr));
			assert(q == fxp_mul32_rem(&zero, v, num, shr));
		}
	}

	for (int i = 0; 20000 > i; i++)
	{
		int64_t	v = (int64_t)rng16() * 100003;
		int64_t	num = (int64_t)rng16() * 7;
		int64_t	zero = 0;
		int64_t	q = fxp_mul64(v, num, 19);

		assert(q == fxp_mul64_rem(NULL, v, num, 19));
		assert(q == fxp_mul64_rem(&zero, v, num, 19));
	}
}

int main(void)
{
	mirror();
	conservation();
	guard_domain();
	rem_bound();
	golden();
	window_exhaustive();
	tie_lattice();
	window64();
	corner_product();
	corner_product64();
	tie_inject();
	push_limit();
	null_consistency();
	return 0;
}
