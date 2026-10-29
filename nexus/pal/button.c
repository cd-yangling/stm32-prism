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

#include <stddef.h>
#include "pal/button.h"
#include "pal/pin.h"
#include "subsys/input.h"
#include "tick.h"

struct button_dat
{
	struct pin_callback	cb;
	volatile uint8_t	pending;
	uint8_t				watching;
	int8_t				last_raw;
	int8_t				stable;
	uint32_t			since;
};

_Static_assert(sizeof(struct button_dat) == BUTTON_DAT_SIZE, "BUTTON_DAT_SIZE out of sync with struct button_dat");

#define	dat_of_cb(cb_p) \
	((struct button_dat *)(void *)((uint8_t *)(cb_p) - offsetof(struct button_dat, cb)))

static void button_edge(const struct pin_port *pt, struct pin_callback *cb)
{
	struct button_dat *d = dat_of_cb(cb);

	(void)pt;
	d->pending = 1;
}

void button_init(const struct button_dev *b)
{
	struct button_dat *d = b->dat;

	pin_configure(b->pin);
	d->pending = 0;
	d->watching = 0;
	d->stable = pin_get(b->pin);
	pin_callback_init(&d->cb, button_edge, b->pin->pin);
	pin_add_callback(b->pin->port, &d->cb);
	pin_irq_configure(b->pin, PIN_INT_BOTH);
}

void button_poll(const struct button_dev *b)
{
	struct button_dat *d = b->dat;
	int raw;

	if (!d->watching && !d->pending)
	{
		return;
	}

	if (!d->watching)
	{
		d->watching = 1;
		d->pending = 0;
		d->last_raw = pin_get(b->pin);
		d->since = tick_get();
	}

	raw = pin_get(b->pin);

	if (raw != d->last_raw)
	{
		d->last_raw = raw;
		d->since = tick_get();
		return;
	}

	if ((tick_get() - d->since) < (uint32_t)b->debounce_ms)
	{
		return;
	}

	d->watching = 0;

	if (raw != d->stable)
	{
		d->stable = raw;
		input_report(b, INPUT_EV_KEY, b->code, raw, 1);
	}
}

int button_pressed(const struct button_dev *b)
{
	return b->dat->stable;
}
