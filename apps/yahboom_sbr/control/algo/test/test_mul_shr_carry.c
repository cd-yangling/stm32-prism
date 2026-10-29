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
#include <stdio.h>
#include "control/algo/mul_shr.h"

static uint32_t rng_s;

static int32_t rng16(void)
{
	rng_s = rng_s * 1103515245u + 12345u;
	return (int32_t)(int16_t)(rng_s >> 16);
}

static double dabs(double d)
{
	return (0.0 > d) ? -d : d;
}

static const int shrs[] = { 1, 2, 8, 10, 16 };
#define	SHRS_N	(sizeof shrs / sizeof shrs[0])

static void test_mirror(void)
{
	for (unsigned s = 0; s < SHRS_N; s++)
	{
		int shr = shrs[s];

		for (int i = 0; i < 20000; i++)
		{
			int32_t v = rng16();
			int32_t num = rng16();
			int32_t ra = 0;
			int32_t rb = 0;
			int32_t qa = mul_shr_carry(&ra, v, num, shr);
			int32_t qb = mul_shr_carry(&rb, -v, num, shr);

			assert(qa == -qb);
			assert(ra == -rb);
		}
	}
}

static void test_identity(void)
{
	for (unsigned s = 0; s < SHRS_N; s++)
	{
		int shr = shrs[s];

		for (int r = 0; r < 50; r++)
		{
			int32_t rem = 0;

			for (int i = 0; i < 3; i++)
			{
				(void)mul_shr_carry(&rem, rng16(), rng16(), shr);
			}

			int32_t rem_init = rem;
			int64_t sp = 0;
			int64_t sq = 0;

			for (int i = 0; i < 400; i++)
			{
				int32_t v = rng16();
				int32_t num = rng16();
				int32_t q = mul_shr_carry(&rem, v, num, shr);

				sp += (int64_t)v * num;
				sq += q;
			}

			assert(sq * (((int64_t)1) << shr) == sp + rem_init - rem);

			double exact = (double)sp / (double)(((int64_t)1) << shr);

			assert(1.0 + 1e-9 >= dabs((double)sq - exact));
		}
	}
}

static void test_sub_lsb(void)
{
	{
		int32_t rem = 0;
		int64_t sq = 0;
		int nz = 0;

		for (int i = 0; i < 1280; i++)
		{
			int32_t q = mul_shr_carry(&rem, 1, 26, 8);

			assert(0 == q || 1 == q);
			if (0 != q)
			{
				nz++;
			}
			sq += q;
		}

		assert(130 == sq);
		assert(0 < nz);
		assert(0 == mul_shr(1, 26, 8));
	}

	{
		int32_t rem = 0;
		int64_t sq = 0;

		for (int i = 0; i < 1280; i++)
		{
			sq += mul_shr_carry(&rem, -1, 26, 8);
		}

		assert(-130 == sq);
	}

	{
		int32_t rem = 0;
		int64_t sq = 0;

		for (int i = 0; i < 2560; i++)
		{
			sq += mul_shr_carry(&rem, 1, 1, 8);
		}

		assert(10 == sq);
	}
}

static void test_vs_round(void)
{
	for (unsigned s = 0; s < SHRS_N; s++)
	{
		int shr = shrs[s];
		int32_t rem = 0;

		for (int i = 0; i < 50000; i++)
		{
			int32_t v = rng16();
			int32_t num = rng16();
			int32_t qc = mul_shr_carry(&rem, v, num, shr);
			int32_t qr = mul_shr(v, num, shr);
			int32_t qn = mul_shr_carry(NULL, v, num, shr);
			int32_t d = qc - qr;

			assert(qn == qr);
			assert(1 >= d && -1 <= d);
			assert((1 << (shr - 1)) >= rem && -(1 << (shr - 1)) <= rem);
		}
	}
}

static void test_edge(void)
{
	{
		int32_t rem = 0;

		assert(0 == mul_shr_carry(&rem, 0, 32767, 16));
		assert(0 == rem);
		assert(0 == mul_shr_carry(&rem, 32767, 0, 1));
		assert(0 == rem);
	}

	{
		int32_t rem = 77;

		assert(39 == mul_shr_carry(&rem, 32767, 0, 1));
		assert(-1 == rem);
	}

	{
		int32_t rem = 0;

		for (int i = 0; i < 100; i++)
		{
			(void)mul_shr_carry(&rem, 32767, 32767, 16);
		}

		assert((1 << 15) >= rem && -(1 << 15) <= rem);
	}

	{
		int32_t rem = 0;

		for (int i = 0; i < 100; i++)
		{
			(void)mul_shr_carry(&rem, -32768, 32767, 1);
		}

		assert(1 >= rem && -1 <= rem);
	}

	{
		int32_t rem = -32768;
		int32_t q = mul_shr_carry(&rem, 32767, 32767, 16);

		assert(16383 == q);
		assert(-32767 == rem);
	}

	for (unsigned s = 0; s < SHRS_N; s++)
	{
		int shr = shrs[s];
		int32_t rem = -(1 << (shr - 1));
		int64_t sp = 0;
		int64_t sq = 0;

		for (int i = 0; i < 200; i++)
		{
			int32_t v = rng16();
			int32_t num = rng16();
			int32_t q = mul_shr_carry(&rem, v, num, shr);

			sp += (int64_t)v * num;
			sq += q;
		}

		assert(sq * (((int64_t)1) << shr) == sp - (int64_t)(1 << (shr - 1)) - rem);
	}
}

static void test_rem_negative(void)
{
	int32_t rem = 0;
	int32_t q = mul_shr_carry(&rem, 5, 26, 8);

	assert(1 == q);
	assert(-126 == rem);
}

int main(void)
{
	rng_s = 20261023u;

	test_mirror();
	printf("PASS mirror\n");
	test_identity();
	printf("PASS identity\n");
	test_sub_lsb();
	printf("PASS sub_lsb\n");
	test_vs_round();
	printf("PASS vs_round\n");
	test_edge();
	printf("PASS edge\n");
	test_rem_negative();
	printf("PASS rem_negative\n");

	return 0;
}
