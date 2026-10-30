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
#include <stddef.h>
#include "pal/i2c.h"
#include "pal/display.h"
#include "dev/ssd1306.h"

#define	WIRE_MAX	4096

static uint8_t wire[WIRE_MAX];
static uint16_t wire_n;
static uint8_t seen_addr7;

static int mock_init(const struct i2c_bus *b, const struct i2c_pins *p)
{
	(void)b;
	(void)p;
	return 0;
}

static int mock_xfer(const struct i2c_bus *b, uint8_t addr7, const struct i2c_msg *m, int n)
{
	int i;
	uint16_t j;

	(void)b;
	seen_addr7 = addr7;
	for (i = 0; i < n; i++)
	{
		for (j = 0; j < m[i].len && wire_n < WIRE_MAX; j++)
		{
			wire[wire_n++] = m[i].buf[j];
		}
	}
	return 0;
}

static void mock_config(const struct i2c_bus *b, const uint32_t spd_hz)
{
	(void)b;
	(void)spd_hz;
}

static const struct i2c_ops mock_i2c_ops =
{
	.init = mock_init,
	.xfer = mock_xfer,
	.config = mock_config,
};

static const struct i2c_bus mock_bus =
{
	.ops = &mock_i2c_ops,
	.ctx = NULL,
	.name = "mocki2c",
};

static const struct i2c_dev mock_dev =
{
	.bus = &mock_bus,
	.addr7 = 0x3C,
	.spd_hz = 400 * 1000,
	.name = "ssd1306",
};

static const struct display_caps caps64 =
{
	.x_res = 128,
	.y_res = 64,
	.fmt = DISPLAY_FMT_MONO_VTILED,
};

static const struct display disp64 =
{
	.ops = &ssd1306_ops,
	.ctx = &mock_dev,
	.caps = &caps64,
	.name = "oled",
};

static const struct display_caps caps32 =
{
	.x_res = 128,
	.y_res = 32,
	.fmt = DISPLAY_FMT_MONO_VTILED,
};

static const struct display disp32 =
{
	.ops = &ssd1306_ops,
	.ctx = &mock_dev,
	.caps = &caps32,
	.name = "oled32",
};

static uint8_t src[9 * 128];

static void src_fill(void)
{
	uint16_t p;
	uint16_t c;

	for (p = 0; p < 9; p++)
	{
		for (c = 0; c < 128; c++)
		{
			src[p * 128 + c] = (uint8_t)(p * 0x10u + c);
		}
	}
}

int main(void)
{
	uint16_t i;

	src_fill();

	wire_n = 0;
	assert(0 == display_init(&disp64));
	assert(0x3C == seen_addr7);
	assert(27 == wire_n);
	assert(0x00 == wire[0]);
	assert(0xA8 == wire[4] && 0x3F == wire[5]);
	assert(0x20 == wire[11] && 0x00 == wire[12]);
	assert(0xDA == wire[15] && 0x12 == wire[16]);
	assert(0xAF == wire[26]);

	wire_n = 0;
	assert(0 == display_init(&disp32));
	assert(27 == wire_n);
	assert(0xA8 == wire[4] && 0x1F == wire[5]);
	assert(0xDA == wire[15] && 0x02 == wire[16]);

	wire_n = 0;
	{
		struct display_area a = { 10, 16, 20, 16, 128 };

		assert(0 == display_write(&disp64, &a, src));
	}
	assert(0x00 == wire[0]);
	assert(0x21 == wire[1] && 10 == wire[2] && 29 == wire[3]);
	assert(0x22 == wire[4] && 2 == wire[5] && 3 == wire[6]);
	assert(0x40 == wire[7]);
	for (i = 0; i < 20; i++)
	{
		assert(src[i] == wire[8 + i]);
	}
	assert(0x40 == wire[28]);
	for (i = 0; i < 20; i++)
	{
		assert(src[128 + i] == wire[29 + i]);
	}
	assert(49 == wire_n);

	wire_n = 0;
	{
		struct display_area a = { 0, 0, 128, 64, 128 };

		assert(0 == display_write(&disp64, &a, src));
	}
	assert(0x22 == wire[4] && 0 == wire[5] && 7 == wire[6]);
	return 0;
}
