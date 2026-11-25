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
#include <stddef.h>
#include <string.h>
#include "lib/deframer.h"

static const unsigned char lc_head[] = {0xFE, 0x0A};

static const struct deframer_cfg lc_cfg =
{
	.head		= lc_head,
	.head_len	= 2u,
	.frame_len	= 14u,
	.ck_kind	= DEFRAMER_CK_XOR,
	.ck_init	= 0u,
	.ck_off		= 2u,
	.ck_pos		= 12u,
	.has_tail	= 1u,
	.tail_val	= 0x55u,
};

static const unsigned char stp_head[] =
{
	0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x02, 0x00, 0x00, 0xB8, 0x00,
};

static const struct deframer_cfg stp_cfg =
{
	.head		= stp_head,
	.head_len	= 10u,
	.frame_len	= 195u,
	.ck_kind	= DEFRAMER_CK_SUM8,
	.ck_init	= 0u,
	.ck_off		= 4u,
	.ck_pos		= 194u,
	.has_tail	= 0u,
	.tail_val	= 0u,
};

static void mk_lc307(unsigned char *p, short fx, short fy,
		unsigned short ts_us, unsigned short dist,
		unsigned char valid, unsigned char ver)
{
	unsigned char x = 0u;
	int i;

	p[0] = 0xFE;
	p[1] = 0x0A;
	p[2] = (unsigned char)(fx & 0xFF);
	p[3] = (unsigned char)((fx >> 8) & 0xFF);
	p[4] = (unsigned char)(fy & 0xFF);
	p[5] = (unsigned char)((fy >> 8) & 0xFF);
	p[6] = (unsigned char)(ts_us & 0xFF);
	p[7] = (unsigned char)((ts_us >> 8) & 0xFF);
	p[8] = (unsigned char)(dist & 0xFF);
	p[9] = (unsigned char)((dist >> 8) & 0xFF);
	p[10] = valid;
	p[11] = ver;

	for (i = 2; i <= 11; i++)
	{
		x ^= p[i];
	}

	p[12] = x;
	p[13] = 0x55;
}

static void mk_stp23l(unsigned char *p, short dist0)
{
	unsigned int sum = 0xBAu;
	int i;

	memcpy(p, stp_head, 10);

	for (i = 10; i < 194; i++)
	{
		p[i] = (unsigned char)(i * 7u + 3u);
	}

	for (i = 0; i < 12; i++)
	{
		short d = (short)(dist0 + i * 37);
		int o = 10 + i * 15;

		p[o] = (unsigned char)(d & 0xFF);
		p[o + 1] = (unsigned char)((d >> 8) & 0xFF);
	}

	p[190] = 0x11;
	p[191] = 0x22;
	p[192] = 0x33;
	p[193] = 0x44;

	for (i = 10; i < 194; i++)
	{
		sum += p[i];
	}

	p[194] = (unsigned char)(sum & 0xFFu);
}

static unsigned int rng = 0xC0FFEEu;

static unsigned int rnd(void)
{
	rng = rng * 1664525u + 1013904223u;
	return rng >> 8;
}

static void drain(struct deframer *f, const unsigned char *ref,
		unsigned short len, unsigned int ts)
{
	const struct deframer_rec *r = deframer_peek(f);

	assert(NULL != r);
	assert(len == r->len);
	assert(ts == r->ts);
	assert(0 == memcmp(ref, r->buf, len));
	deframer_advance(f);
	assert(NULL == deframer_peek(f));
}

int main(void)
{
	{
		struct deframer f;
		struct deframer_cfg c = lc_cfg;

		assert(0 == deframer_init(&f, &lc_cfg));

		c.head = NULL;
		assert(-EINVAL == deframer_init(&f, &c));
		c = lc_cfg;
		c.head_len = 0u;
		assert(-EINVAL == deframer_init(&f, &c));
		c = lc_cfg;
		c.frame_len = DEFRAMER_FRAME_MAX + 1u;
		assert(-EINVAL == deframer_init(&f, &c));
		c = lc_cfg;
		c.head_len = 15u;
		assert(-EINVAL == deframer_init(&f, &c));
		c = lc_cfg;
		c.ck_off = 12u;
		assert(-EINVAL == deframer_init(&f, &c));
		c = lc_cfg;
		c.head_len = 13u;
		assert(-EINVAL == deframer_init(&f, &c));
		c = lc_cfg;
		c.ck_pos = 14u;
		assert(-EINVAL == deframer_init(&f, &c));
		c = lc_cfg;
		c.ck_pos = 13u;
		assert(-EINVAL == deframer_init(&f, &c));
		c = lc_cfg;
		c.ck_kind = 7u;
		assert(-EINVAL == deframer_init(&f, &c));
	}

	{
		struct deframer f;
		unsigned char fr[14];
		int i;

		mk_lc307(fr, -321, 654, 20000, 999, 0xF5, 0x12);
		assert(0 == deframer_init(&f, &lc_cfg));

		assert(1 == deframer_feed(&f, fr, 14u, 100u));
		drain(&f, fr, 14u, 100u);

		for (i = 0; i < 13; i++)
		{
			assert(0 == deframer_feed(&f, &fr[i], 1u, 200u));
		}

		assert(1 == deframer_feed(&f, &fr[13], 1u, 200u));
		drain(&f, fr, 14u, 200u);

		assert(0 == deframer_feed(&f, fr, 6u, 300u));
		assert(1 == deframer_feed(&f, fr + 6, 8u, 301u));
		drain(&f, fr, 14u, 301u);

		assert(0u == deframer_fail_get(&f));
	}

	{
		struct deframer f;
		unsigned char fr[14];
		unsigned char bad[14];
		unsigned char junk[24];
		int i;

		mk_lc307(fr, 100, -100, 19999, 999, 0xF5, 0x01);
		assert(0 == deframer_init(&f, &lc_cfg));

		for (i = 0; i < 24; i++)
		{
			junk[i] = (unsigned char)(0x21 + (i % 0x5D));
		}

		assert(0 == deframer_feed(&f, junk, 24u, 1u));
		assert(1 == deframer_feed(&f, fr, 14u, 2u));
		drain(&f, fr, 14u, 2u);

		memcpy(bad, fr, 14);
		bad[12] ^= 0xFFu;
		assert(0 == deframer_feed(&f, bad, 14u, 3u));
		assert(1u == deframer_fail_get(&f));
		assert(1 == deframer_feed(&f, fr, 14u, 4u));
		drain(&f, fr, 14u, 4u);

		memcpy(bad, fr, 14);
		bad[13] = 0x54;
		assert(0 == deframer_feed(&f, bad, 14u, 5u));
		assert(2u == deframer_fail_get(&f));
		assert(1 == deframer_feed(&f, fr, 14u, 6u));
		drain(&f, fr, 14u, 6u);

		{
			unsigned char fake[6] = {0x00, 0xFE, 0x0A, 0x71, 0xFE, 0x0A};

			assert(0 == deframer_feed(&f, fake, 6u, 7u));
			assert(0 == deframer_feed(&f, fr, 14u, 8u));
			assert(3u == deframer_fail_get(&f));
			assert(1 == deframer_feed(&f, fr, 14u, 9u));
			drain(&f, fr, 14u, 9u);
		}

		{
			unsigned char two[28];
			unsigned char g[14];

			mk_lc307(g, 1, 2, 20, 999, 0xF5, 0x02);

			memcpy(two, fr, 14);
			memcpy(two + 14, g, 14);

			assert(2 == deframer_feed(&f, two, 28u, 9u));
			{
				const struct deframer_rec *r = deframer_peek(&f);

				assert(NULL != r);
				assert(14u == r->len);
				assert(0 == memcmp(fr, r->buf, 14u));
				deframer_advance(&f);
			}
			{
				const struct deframer_rec *r = deframer_peek(&f);

				assert(NULL != r);
				assert(14u == r->len);
				assert(0 == memcmp(g, r->buf, 14u));
				deframer_advance(&f);
			}
			assert(NULL == deframer_peek(&f));
		}
	}

	{
		struct deframer f;
		unsigned char fr[195];
		unsigned char bad[195];
		unsigned char aa[20];
		int i;

		mk_stp23l(fr, 1500);
		assert(0 == deframer_init(&f, &stp_cfg));

		assert(1 == deframer_feed(&f, fr, 195u, 10u));
		drain(&f, fr, 195u, 10u);

		for (i = 0; i < 194; i++)
		{
			assert(0 == deframer_feed(&f, &fr[i], 1u, 11u));
		}

		assert(1 == deframer_feed(&f, &fr[194], 1u, 11u));
		drain(&f, fr, 195u, 11u);

		assert(0u == deframer_fail_get(&f));

		for (i = 0; i < 20; i++)
		{
			aa[i] = 0xAA;
		}

		assert(0 == deframer_feed(&f, aa, 20u, 12u));
		assert(1 == deframer_feed(&f, fr, 195u, 13u));
		drain(&f, fr, 195u, 13u);

		for (i = 0; i < 195; i++)
		{
			bad[i] = fr[i];
		}
		bad[100] ^= 0x01u;
		assert(0 == deframer_feed(&f, bad, 195u, 14u));
		assert(1u == deframer_fail_get(&f));
		assert(1 == deframer_feed(&f, fr, 195u, 15u));
		drain(&f, fr, 195u, 15u);
	}

	{
		struct deframer f;
		unsigned char fr[14];
		int i;

		mk_lc307(fr, 7, 8, 20000, 999, 0xF5, 0x03);
		assert(0 == deframer_init(&f, &lc_cfg));

		for (i = 0; i < 4; i++)
		{
			assert(1 == deframer_feed(&f, fr, 14u, 1u + (unsigned int)i));
		}

		assert(0 == deframer_feed(&f, fr, 14u, 5u));
		assert(1u == deframer_fail_get(&f));

		for (i = 0; i < 4; i++)
		{
			const struct deframer_rec *r = deframer_peek(&f);

			assert(NULL != r);
			assert(14u == r->len);
			assert(0 == memcmp(fr, r->buf, 14u));
			deframer_advance(&f);
		}

		assert(NULL == deframer_peek(&f));
	}

	{
		struct deframer f;
		unsigned char stream[50u * 22u];
		unsigned char ref[50u][14u];
		unsigned int total = 0u;
		unsigned int seen = 0u;
		unsigned int k;

		assert(0 == deframer_init(&f, &lc_cfg));

		for (k = 0u; k < 50u; k++)
		{
			unsigned int nj = rnd() % 9u;
			unsigned int i;

			mk_lc307(ref[k], (short)(k * 11u - 30u),
					-(short)k, (unsigned short)(20000u + k),
					999u, 0xF5, (unsigned char)(k & 0xFFu));

			memcpy(&stream[total], ref[k], 14u);

			total += 14u;

			for (i = 0u; i < nj; i++)
			{
				stream[total + i] =
						(unsigned char)(0x21u + (rnd() % 0x5Du));
			}

			total += nj;
		}

		{
			unsigned int off = 0u;
			const struct deframer_rec *r;

			while (off < total)
			{
				unsigned int cut = 1u + (rnd() % 20u);

				if (off + cut > total)
				{
					cut = total - off;
				}

				(void)deframer_feed(&f, &stream[off], (unsigned short)cut,
						1000u + off);
				off += cut;

				while (NULL != (r = deframer_peek(&f)))
				{
					assert(14u == r->len);
					assert(0 == memcmp(ref[seen], r->buf, 14u));
					deframer_advance(&f);
					seen++;
				}
			}
		}

		{
			const struct deframer_rec *r;

			while (NULL != (r = deframer_peek(&f)))
			{
				assert(14u == r->len);
				assert(0 == memcmp(ref[seen], r->buf, 14u));
				deframer_advance(&f);
				seen++;
			}
		}

		assert(50u == seen);
	}

	{
		static const unsigned char ov_head[] =
		{
			0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x00,
		};
		const struct deframer_cfg ov_cfg =
		{
			.head		= ov_head,
			.head_len	= 6u,
			.frame_len	= 11u,
			.ck_kind	= DEFRAMER_CK_SUM8,
			.ck_init	= 0u,
			.ck_off		= 4u,
			.ck_pos		= 10u,
			.has_tail	= 0u,
			.tail_val	= 0u,
		};
		struct deframer f;
		unsigned char fr[11];
		unsigned char junk[8];
		unsigned int sum = 0u;
		int i;

		memcpy(fr, ov_head, 6);

		fr[6] = 0x31;
		fr[7] = 0x42;
		fr[8] = 0x53;
		fr[9] = 0x64;

		for (i = 4; i < 10; i++)
		{
			sum += fr[i];
		}

		fr[10] = (unsigned char)(sum & 0xFFu);

		memset(junk, 0xAA, 8);

		assert(0 == deframer_init(&f, &ov_cfg));
		assert(0 == deframer_feed(&f, junk, 8u, 1u));
		assert(1 == deframer_feed(&f, fr, 11u, 2u));
		drain(&f, fr, 11u, 2u);
	}

	return 0;
}
