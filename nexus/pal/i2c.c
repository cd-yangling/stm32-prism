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

#include <stdint.h>
#include "pal/i2c.h"

int i2c_bus_init(const struct i2c_bus *b, const struct i2c_pins *pins)
{
	return b->ops->init(b, pins);
}

struct i2c_msg
{
	uint8_t *buf;
	uint16_t len;
#define	I2C_MSG_READ			((1) << 0)
	uint16_t flags;
};

static int i2c_xfer(const struct i2c_dev *d, const struct i2c_msg *m, int n)
{
	int i, ret = 0;

	d->bus->ops->config(d->bus, d->spd_hz);

	for (i = 0; !ret && i < n; i++)
	{
		d->bus->ops->start(d->bus);

		ret = d->bus->ops->xfer(d->bus, d->addr7,
				m[i].flags & I2C_MSG_READ, m[i].buf, m[i].len);
	}

	d->bus->ops->stop(d->bus);

	return ret;
}

int i2c_write(const struct i2c_dev *d, const uint8_t *buf, uint16_t len)
{
	const struct i2c_msg msg = 
	{
		.buf = (uint8_t *)buf, .len = len, .flags = 0,
	};

	return i2c_xfer(d, &msg, 1);
}

int i2c_read(const struct i2c_dev *d, uint8_t *buf, uint16_t len)
{
	const struct i2c_msg msg = 
	{
		.buf = buf, .len = len, .flags = I2C_MSG_READ,
	};

	return i2c_xfer(d, &msg, 1);
}

int i2c_mem_read(const struct i2c_dev *d, uint8_t *mb, uint16_t ml, uint8_t *rb, uint16_t rl)
{
	const struct i2c_msg msg[2] =
	{
		{ .buf = mb, .len = ml, .flags = 0},
		{ .buf = rb, .len = rl, .flags = I2C_MSG_READ},
	};

	return i2c_xfer(d, msg, 2);
}

int i2c_probe(const struct i2c_dev *d)
{
	const struct i2c_msg msg = { 0 };

	return i2c_xfer(d, &msg, 1);
}

