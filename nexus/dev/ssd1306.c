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
#include <errno.h>
#include "pal/i2c.h"
#include "pal/display.h"
#include "dev/ssd1306.h"

static const uint8_t init_seq[] =
{
	0xAEu,             // display off
	0xD5u, 0x80u,      // clock divide / oscillator = 0x80
	0xA8u, 0x3Fu,      // multiplex = 63(128x64 缺省,init 按 caps 覆盖)
	0xD3u, 0x00u,      // display offset = 0
	0x40u,             // display start line = 0
	0x8Du, 0x14u,      // charge pump ON
	0x20u, 0x00u,      // memory addressing = 水平(区域写窗口配套)
	0xA1u,             // segment remap:左右正立
	0xC8u,             // COM scan 反向:上下正立
	0xDAu, 0x12u,      // COM pins(128x64 缺省,init 按 caps 覆盖)
	0x81u, 0xCFu,      // contrast(charge pump 供电推荐值)
	0xD9u, 0xF1u,      // pre-charge(charge pump 供电推荐值)
	0xDBu, 0x40u,      // VCOMH deselect level
	0x2Eu,             // deactivate scroll
	0xA4u,             // 输出跟随 GDDRAM(0xA5 才是全亮测试)
	0xA6u,             // normal 显示(非反相)
	0xAFu,             // display on
};

#define	SEQ_MUX_VAL		4u
#define	SEQ_COMPINS_VAL	15u

typedef char init_seq_is_26_bytes[(26 == sizeof(init_seq)) ? 1 : -1];

static int wr(const struct i2c_dev *d, uint8_t ctrl, const uint8_t *buf, uint16_t n)
{
	uint8_t block[1 + 128];
	uint16_t i;

	block[0] = ctrl;
	for (i = 0; i < n; i++)
	{
		block[1 + i] = buf[i];
	}
	return i2c_write(d, block, (uint16_t)(1u + n));
}

static int ssd1306_init(const struct display *d)
{
	uint8_t seq[sizeof(init_seq)];
	uint16_t i;

	if ((128u < d->caps->x_res) || (16u > d->caps->y_res) || (64u < d->caps->y_res))
	{
		return -EINVAL;
	}
	for (i = 0; i < (uint16_t)sizeof(seq); i++)
	{
		seq[i] = init_seq[i];
	}
	seq[SEQ_MUX_VAL] = (uint8_t)(d->caps->y_res - 1u);
	seq[SEQ_COMPINS_VAL] = (32u == d->caps->y_res) ? 0x02u : 0x12u;
	return wr(d->ctx, 0x00u, seq, (uint16_t)sizeof(seq));
}

static int ssd1306_write(const struct display *d, const struct display_area *a, const uint8_t *buf)
{
	const struct i2c_dev *dev = d->ctx;
	uint8_t cmd[6];
	uint8_t page;
	uint8_t pages;
	int ret;

	cmd[0] = 0x21u;
	cmd[1] = (uint8_t)a->x;
	cmd[2] = (uint8_t)(a->x + a->w - 1u);
	cmd[3] = 0x22u;
	cmd[4] = (uint8_t)(a->y / 8u);
	cmd[5] = (uint8_t)((a->y + a->h) / 8u - 1u);
	ret = wr(dev, 0x00u, cmd, 6u);
	if (0 != ret)
	{
		return ret;
	}
	pages = (uint8_t)(a->h / 8u);
	for (page = 0; page < pages; page++)
	{
		ret = wr(dev, 0x40u, &buf[(uint32_t)page * a->pitch], a->w);
		if (0 != ret)
		{
			return ret;
		}
	}
	return 0;
}

static int ssd1306_blanking_on(const struct display *d)
{
	uint8_t c = 0xAEu;

	return wr(d->ctx, 0x00u, &c, 1u);
}

static int ssd1306_blanking_off(const struct display *d)
{
	uint8_t c = 0xAFu;

	return wr(d->ctx, 0x00u, &c, 1u);
}

const struct display_ops ssd1306_ops =
{
	.init         = ssd1306_init,
	.write        = ssd1306_write,
	.blanking_on  = ssd1306_blanking_on,
	.blanking_off = ssd1306_blanking_off,
};
