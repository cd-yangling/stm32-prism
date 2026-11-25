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
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include "lib/gadget/barrier.h"
#include "lib/deframer.h"

static int deframer_cfg_check(const struct deframer_cfg *c)
{
	if ((NULL == c->head) || (0u == c->head_len))
	{
		return -EINVAL;
	}

	if (c->frame_len > DEFRAMER_FRAME_MAX)
	{
		return -EINVAL;
	}

	if (c->head_len > c->frame_len)
	{
		return -EINVAL;
	}

	if (c->ck_off >= c->ck_pos)
	{
		return -EINVAL;
	}

	if (c->ck_pos < c->head_len)
	{
		return -EINVAL;
	}

	if (c->ck_pos >= c->frame_len)
	{
		return -EINVAL;
	}

	if ((0u != c->has_tail) && (c->ck_pos + 1u >= c->frame_len))
	{
		return -EINVAL;
	}

	if ((DEFRAMER_CK_XOR != c->ck_kind) && (DEFRAMER_CK_SUM8 != c->ck_kind))
	{
		return -EINVAL;
	}

	return 0;
}

static void deframer_ck_add(struct deframer *f, unsigned char b)
{
	if (DEFRAMER_CK_XOR == f->cfg->ck_kind)
	{
		f->acc ^= b;
	}
	else
	{
		f->acc = (f->acc + b) & 0xFFu;
	}
}

static void deframer_resync(struct deframer *f)
{
	const struct deframer_cfg *c = f->cfg;
	unsigned short n = f->pos + 1u;
	unsigned short i;
	unsigned short k;

	memmove(f->asm_buf, f->asm_buf + 1u, n - 1u);

	n--;

	k = 0u;

	while ((k < n) && (k < c->head_len) && (f->asm_buf[k] == c->head[k]))
	{
		k++;
	}

	f->pos = k;
	f->acc = c->ck_init;

	for (i = c->ck_off; (i < k) && (i < n); i++)
	{
		deframer_ck_add(f, f->asm_buf[i]);
	}
}

static int deframer_emit(struct deframer *f)
{
	const struct deframer_cfg *c = f->cfg;
	int result = 0u;

	if (f->rin - f->rout >= DEFRAMER_READY_LEN)
	{
		f->fail++;
	}
	else
	{
		struct deframer_rec *s = &f->ready[f->rin % DEFRAMER_READY_LEN];

		memcpy(s->buf, f->asm_buf, c->frame_len);

		s->len = c->frame_len;
		s->ts = f->cur_ts;
		barrier();
		f->rin++;
		result = 1u;
	}

	f->pos = 0u;
	f->acc = c->ck_init;

	return result;
}

static int deframer_byte(struct deframer *f, unsigned char b)
{
	const struct deframer_cfg *c = f->cfg;
	unsigned short pos = f->pos;

	f->asm_buf[pos] = b;

	if ((pos >= c->ck_off) && (pos < c->ck_pos))
	{
		deframer_ck_add(f, b);
	}

	if (pos < c->head_len)
	{
		if (c->head[pos] == b)
		{
			f->pos = pos + 1u;
		}
		else
		{
			deframer_resync(f);
		}

		return 0;
	}

	if (pos == c->ck_pos)
	{
		if (b != (f->acc & 0xFFu))
		{
			f->fail++;
			deframer_resync(f);
			return 0;
		}
	}
	else if ((0u != c->has_tail) && (pos + 1u == c->frame_len))
	{
		if (c->tail_val != b)
		{
			f->fail++;
			deframer_resync(f);
			return 0;
		}
	}

	f->pos = pos + 1u;

	if (f->pos < c->frame_len)
	{
		return 0;
	}

	return deframer_emit(f);
}

int deframer_init(struct deframer *f, const struct deframer_cfg *cfg)
{
	int r = deframer_cfg_check(cfg);

	if (0 != r)
	{
		return r;
	}

	f->cfg = cfg;
	f->pos = 0u;
	f->acc = cfg->ck_init;
	f->fail = 0u;
	f->cur_ts = 0u;
	f->rin = 0u;
	f->rout = 0u;

	return 0;
}

int deframer_feed(struct deframer *f, const unsigned char *data,
		unsigned short len, unsigned int ts)
{
	int n = 0;
	unsigned short i;

	f->cur_ts = ts;

	for (i = 0u; i < len; i++)
	{
		n += deframer_byte(f, data[i]);
	}

	return n;
}

const struct deframer_rec *deframer_peek(const struct deframer *f)
{
	if (f->rout == f->rin)
	{
		return NULL;
	}

	return &f->ready[f->rout % DEFRAMER_READY_LEN];
}

void deframer_advance(struct deframer *f)
{
	barrier();
	f->rout++;
}

unsigned int deframer_fail_get(const struct deframer *f)
{
	return f->fail;
}
