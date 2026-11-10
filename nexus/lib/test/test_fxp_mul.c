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

	assert(1 == fxp_mul32(536870912, 1, 30));
	assert(-1 == fxp_mul32(-536870912, 1, 30));
	assert(0 == fxp_mul32(536870911, 1, 30));
	assert(1 == fxp_mul64(2305843009213693952LL, 1, 62));
	assert(-1 == fxp_mul64(2305843009213693952LL, -1, 62));
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
	assert(1 == fxp_mul32(1, 128, 8));
	assert(0 == fxp_mul32(0, 32767, 16));
	assert(0 == fxp_mul32(32767, 0, 1));
	assert(1 == fxp_mul32(1, 32768, 16));
	assert(-1 == fxp_mul32(-1, 32768, 16));
	assert(2 == fxp_mul32(98304, 1, 16));
	assert(-2 == fxp_mul32(-98304, 1, 16));
	assert(-1 == fxp_mul32(-98303, 1, 16));
	assert(0 == fxp_mul32(-32767, 1, 16));
	assert(-1 == fxp_mul32(-32769, 1, 16));
	assert(1 == fxp_mul64(1LL << 40, 1, 41));
	assert(-1 == fxp_mul64(1LL << 40, -1, 41));

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

int main(void)
{
	mirror();
	conservation();
	guard_domain();
	rem_bound();
	golden();
	return 0;
}
