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
#include <errno.h>
#include "pal/display.h"
#include "subsys/gfx.h"

static struct display_area got;
static const uint8_t *got_buf;
static int calls;

static int mock_init(const struct display *d)
{
	(void)d;
	return 0;
}

static int mock_write(const struct display *d, const struct display_area *a, const uint8_t *buf)
{
	(void)d;
	got = *a;
	got_buf = buf;
	calls++;
	return 0;
}

static int mock_blank(const struct display *d)
{
	(void)d;
	return 0;
}

static const struct display_ops mock_ops =
{
	.init         = mock_init,
	.write        = mock_write,
	.blanking_on  = mock_blank,
	.blanking_off = mock_blank,
};

static const struct display_caps mock_caps =
{
	.x_res = 128,
	.y_res = 64,
	.fmt = DISPLAY_FMT_MONO_VTILED,
};

static const struct display mock_disp =
{
	.ops = &mock_ops,
	.ctx = NULL,
	.caps = &mock_caps,
	.name = "mock",
};

int main(void)
{
	assert(-ENODEV == gfx_flush(0, 0, 8, 8));
	assert(0 == gfx_attach(&mock_disp));
	calls = 0;

	gfx_fill(0);
	gfx_px(10, 9, 1);
	assert(0 == gfx_flush(9, 8, 3, 2));
	assert(1 == calls);
	assert(9 == got.x && 8 == got.y && 3 == got.w && 8 == got.h);
	assert(128 == got.pitch);
	assert(0x02 == got_buf[0 * 128 + 1]);

	gfx_fill(0);
	gfx_px(0, 0, 1);
	assert(0 == gfx_flush(0, 0, 1, 1));
	assert(0 == got.y && 8 == got.h);
	assert(0x01 == got_buf[0]);

	gfx_fill(0);
	gfx_px(0, 16, 1);
	assert(0 == gfx_flush(0, 16, 128, 16));
	assert(0 == got.x && 16 == got.y && 16 == got.h);
	assert(0x01 == got_buf[0]);

	assert(0 == gfx_flush(120, 60, 100, 100));
	assert(120 == got.x && 56 == got.y && 8 == got.w && 8 == got.h);

	assert(0 == gfx_flush(200, 0, 5, 8));
	assert(0 == gfx_flush(-8, 0, 4, 8));
	assert(4 == calls);

	assert(0 == gfx_flush(-5, -5, 10, 10));
	assert(0 == got.x && 0 == got.y && 5 == got.w && 8 == got.h);
	return 0;
}
