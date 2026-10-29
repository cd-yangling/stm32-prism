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

#include "fifo.h"
#include <errno.h>
#include <string.h>

#define barrier() __asm volatile ("" ::: "memory")

static unsigned int rounddown_pow_of_two(unsigned int n)
{
	if (0 == n)
	{
		return 0;
	}

	return 1u << (31u - (unsigned int)__builtin_clz(n));
}

int fifo_init(struct fifo *q, void *buf, unsigned int len)
{
	unsigned int size;

	size = rounddown_pow_of_two(len);
	if (size < 2u)
	{
		return -EINVAL;
	}

	q->data = (unsigned char *)buf;
	q->mask = size - 1u;
	q->in = 0u;
	q->out = 0u;
	return 0;
}

unsigned int fifo_size(const struct fifo *q)
{
	return q->mask + 1u;
}

unsigned int fifo_used(const struct fifo *q)
{
	return q->in - q->out;
}

unsigned int fifo_avail(const struct fifo *q)
{
	return (q->mask + 1u) - (q->in - q->out);
}

int fifo_is_empty(const struct fifo *q)
{
	return q->in == q->out;
}

int fifo_is_full(const struct fifo *q)
{
	return fifo_size(q) == fifo_used(q);
}

static void fifo_copy_in(struct fifo *q, const void *src, unsigned int len)
{
	unsigned int off = q->in & q->mask;
	unsigned int l = q->mask + 1u - off;

	if (l > len)
	{
		l = len;
	}

	memcpy(q->data + off, src, l);
	memcpy(q->data, (const unsigned char *)src + l, len - l);
}

static void fifo_copy_out(struct fifo *q, void *dst, unsigned int len)
{
	unsigned int off = q->out & q->mask;
	unsigned int l = q->mask + 1u - off;

	if (l > len)
	{
		l = len;
	}

	memcpy(dst, q->data + off, l);
	memcpy((unsigned char *)dst + l, q->data, len - l);
}

unsigned int fifo_enqueue(struct fifo *q, const void *src, unsigned int len)
{
	unsigned int avail = fifo_avail(q);

	if (len > avail)
	{
		len = avail;
	}

	fifo_copy_in(q, src, len);
	barrier();		//	编译器屏障
	q->in += len;
	return len;
}

unsigned int fifo_dequeue(struct fifo *q, void *dst, unsigned int len)
{
	unsigned int used = fifo_used(q);

	if (len > used)
	{
		len = used;	// 有多少取多少
	}

	fifo_copy_out(q, dst, len);
	barrier();		//	编译器屏障
	q->out += len;
	return len;
}

int fifo_putc(struct fifo *q, unsigned char c)
{
	if (fifo_is_full(q))
	{
		return 0;
	}

	q->data[q->in & q->mask] = c;
	barrier();
	q->in++;
	return 1;
}

int fifo_getc(struct fifo *q)
{
	if (fifo_is_empty(q))
	{
		return -EAGAIN;
	}

	{
		int c = q->data[q->out & q->mask];

		barrier();
		q->out++;
		return c;
	}
}
