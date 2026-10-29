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
#include "lib/gadget/fifo.h"

int main(void)
{
	/* ---------- 用例 1:init 边界 + 容量取整 ---------- */
	{
		struct fifo f;
		unsigned char buf[100];

		assert(0 == fifo_init(&f, buf, 100));
		assert(64 == fifo_size(&f));
		assert(0 == fifo_used(&f));
		assert(64 == fifo_avail(&f));
		assert(fifo_is_empty(&f));
		assert(!fifo_is_full(&f));

		assert(0 == fifo_init(&f, buf, 3));
		assert(2 == fifo_size(&f));
		assert(0 == fifo_init(&f, buf, 2));
		assert(2 == fifo_size(&f));

		assert(-EINVAL == fifo_init(&f, buf, 1));
		assert(-EINVAL == fifo_init(&f, buf, 0));

		assert(0 == fifo_init(&f, buf, 256));
		assert(256 == fifo_size(&f));
	}

	/* ---------- 用例 2:满容量(满 = size) ---------- */
	{
		struct fifo f;
		unsigned char buf[16];
		int i;

		assert(0 == fifo_init(&f, buf, sizeof buf));
		for (i = 0; i < 16; i++)
		{
			assert(1 == fifo_putc(&f, (unsigned char)i));
		}
		assert(fifo_is_full(&f));
		assert(0 == fifo_avail(&f));
		assert(0 == fifo_putc(&f, 0xAA));

		for (i = 0; i < 16; i++)
		{
			assert(i == fifo_getc(&f));
		}
		assert(fifo_is_empty(&f));
		assert(16 == fifo_avail(&f));
		assert(-EAGAIN == fifo_getc(&f));
	}

	/* ---------- 用例 3:全错位 全长度(回绕全排) ----------*/
	{
		struct fifo f;
		unsigned char buf[8];
		unsigned char out[8];
		int start, len, i;

		for (start = 0; start < 8; start++)
		{
			for (len = 0; len <= 8; len++)
			{
				unsigned char ref[8];

				assert(0 == fifo_init(&f, buf, sizeof buf));
				for (i = 0; i < start; i++)
				{
					assert(1 == fifo_putc(&f, 0xE0));
				}
				assert((unsigned int)start == fifo_dequeue(&f, out, sizeof out));
				assert(fifo_is_empty(&f));

				for (i = 0; i < 8; i++)
				{
					ref[i] = (unsigned char)(i * 7 + 3);
				}
				assert((unsigned int)len == fifo_enqueue(&f, ref, len));
				assert((unsigned int)len == fifo_used(&f));
				assert((8u - (unsigned int)len) == fifo_avail(&f));
				assert((unsigned int)len == fifo_dequeue(&f, out, len));
				for (i = 0; i < len; i++)
				{
					assert(ref[i] == out[i]);
				}
				assert(fifo_is_empty(&f));
			}
		}
	}

	/* ---------- 用例 4:部分拷贝 ---------- */
	{
		struct fifo f;
		unsigned char buf[8];
		unsigned char out[8];
		unsigned char src[16];
		int i;

		assert(0 == fifo_init(&f, buf, sizeof buf));
		for (i = 0; i < 16; i++)
		{
			src[i] = (unsigned char)(0xA0 + i);
		}

		assert(8 == fifo_enqueue(&f, src, 16));
		assert(8 == fifo_used(&f));
		assert(fifo_is_full(&f));
		assert(0 == fifo_enqueue(&f, src, 1));

		assert(3 == fifo_dequeue(&f, out, 3));
		assert(out[0] == 0xA0 && out[2] == 0xA2);
		assert(5 == fifo_used(&f));
		assert(3 == fifo_enqueue(&f, src + 5, 16));

		assert(8 == fifo_dequeue(&f, out, 100));
		assert(fifo_is_empty(&f));
		assert(0 == fifo_dequeue(&f, out, 100));

		assert(8 == fifo_enqueue(&f, src, 8));
		assert(0 == fifo_enqueue(&f, src, 0));
		assert(0 == fifo_dequeue(&f, out, 0));
		assert(8 == fifo_used(&f));
	}

	/* ---------- 用例 5:putc/getc 与流式混用 + 0xFF 完整性 ----------*/
	{
		struct fifo f;
		unsigned char buf[8];
		unsigned char out[8];
		int i;

		assert(0 == fifo_init(&f, buf, sizeof buf));
		assert(1 == fifo_putc(&f, 0xFF));
		assert(3 == fifo_enqueue(&f, "\x01\x02\x03", 3));
		assert(4 == fifo_used(&f));
		assert(0xFF == fifo_getc(&f));
		assert(3 == fifo_dequeue(&f, out, 3));
		assert(out[0] == 1 && out[2] == 3);
		assert(-EAGAIN == fifo_getc(&f));

		for (i = 0; i < 8; i++)
		{
			assert(1 == fifo_putc(&f, 0xFF));
		}
		assert(0 == fifo_putc(&f, 0xFF));
		assert(8 == fifo_dequeue(&f, out, 8));
		for (i = 0; i < 8; i++)
		{
			assert(0xFF == out[i]);
		}
	}

	/* ---------- 用例 6:索引 32 位回绕 ----------*/
	{
		struct fifo f;
		unsigned char buf[16];
		unsigned int i;

		assert(0 == fifo_init(&f, buf, sizeof buf));
		f.in = 0xFFFFFFF0u;
		f.out = 0xFFFFFFF0u;
		assert(fifo_is_empty(&f));
		assert(16 == fifo_avail(&f));

		for (i = 0; i < 16; i++)
		{
			assert(1 == fifo_putc(&f, (unsigned char)(0xF0 + i)));
		}
		assert(fifo_is_full(&f));
		for (i = 0; i < 16; i++)
		{
			assert((int)(unsigned char)(0xF0 + i) == fifo_getc(&f));
		}
		assert(fifo_is_empty(&f));

		f.in = 0xFFFFFFFFu;
		f.out = 0xFFFFFFF1u;
		assert(14 == fifo_used(&f));
		assert(2 == fifo_avail(&f));
		assert(1 == fifo_putc(&f, 0x5A));
		assert(0x5A == buf[15]);
	}

	/* ---------- 用例 7:随机压力 ----*/
	{
		struct fifo f;
		unsigned char buf[64];
		unsigned char wr[70], rd[70];
		unsigned char ref[128];
		unsigned int ref_in = 0, ref_out = 0;
		unsigned int rng = 0x12345678u;
		unsigned int round;

		assert(0 == fifo_init(&f, buf, sizeof buf));
		for (round = 0; round < 5000u; round++)
		{
			unsigned int len;
			unsigned int i;

			rng = rng * 1664525u + 1013904223u;
			len = (rng >> 24) % 70u;

			for (i = 0; i < len; i++)
			{
				wr[i] = (unsigned char)(rng ^ round ^ i);
			}
			{
				unsigned int expect = 64u - (ref_in - ref_out);

				if (len < expect)
				{
					expect = len;
				}
				assert(expect == fifo_enqueue(&f, wr, len));
				for (i = 0; i < expect; i++)
				{
					ref[(ref_in + i) & 127u] = wr[i];
				}
				ref_in += expect;
			}

			rng = rng * 1664525u + 1013904223u;
			len = (rng >> 24) % 70u;
			{
				unsigned int n = fifo_dequeue(&f, rd, len);
				unsigned int used = ref_in - ref_out;

				if (n > used)
				{
					n = used;
				}
				for (i = 0; i < n; i++)
				{
					assert(ref[ref_out & 127u] == rd[i]);
					ref_out++;
				}
				assert(fifo_used(&f) == ref_in - ref_out);
			}
		}
	}

	return 0;
}
