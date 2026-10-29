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
#include <stddef.h>
#include "pal/spi.h"
#include "pal/pin.h"

int spi_bus_init(const struct spi_bus *b, const struct spi_pins *pins)
{
	return b->ops->init(b, pins);
}

struct spi_msg
{
	const uint8_t *tx;
	uint8_t *rx;
	uint16_t len;
};

static int spi_txn(const struct spi_dev *d, const struct spi_msg *m, int n)
{
	int i, ret = 0;

	d->bus->ops->config(d->bus, d);

	pin_set(d->cs, 1);

	for (i = 0; !ret && i < n; i++)
	{
		ret = d->bus->ops->xfer(d->bus, m[i].tx, m[i].rx, m[i].len);
	}

	pin_set(d->cs, 0);

	return ret;
}

int spi_write(const struct spi_dev *d, const uint8_t *buf, uint16_t len)
{
	const struct spi_msg msg =
	{
		.tx = buf, .rx = NULL, .len = len,
	};

	return spi_txn(d, &msg, 1);
}

int spi_read(const struct spi_dev *d, uint8_t *buf, uint16_t len)
{
	const struct spi_msg msg =
	{
		.tx = NULL, .rx = buf, .len = len,
	};

	return spi_txn(d, &msg, 1);
}

int spi_write_then_read(const struct spi_dev *d, const uint8_t *wb, uint16_t wl, uint8_t *rb, uint16_t rl)
{
	const struct spi_msg msg[2] =
	{
		{ .tx = wb, .rx = NULL, .len = wl },
		{ .tx = NULL, .rx = rb, .len = rl },
	};

	return spi_txn(d, msg, 2);
}

int spi_write_then_write(const struct spi_dev *d, const uint8_t *wb, uint16_t wl, const uint8_t *data, uint16_t dl)
{
	const struct spi_msg msg[2] =
	{
		{ .tx = wb, .rx = NULL, .len = wl },
		{ .tx = data, .rx = NULL, .len = dl },
	};

	return spi_txn(d, msg, 2);
}
