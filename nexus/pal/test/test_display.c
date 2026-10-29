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

static struct display_area got;
static int calls;
static int ret_inject;

static int mock_init(const struct display *d)
{
	(void)d;
	return 0;
}

static int mock_write(const struct display *d, const struct display_area *a, const uint8_t *buf)
{
	(void)d;
	(void)buf;
	got = *a;
	calls++;
	return ret_inject;
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

static uint8_t fb[1024];

static int write_try(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t pitch)
{
	struct display_area a = { x, y, w, h, pitch };

	calls = 0;
	return display_write(&mock_disp, &a, fb);
}

int main(void)
{
	assert(0 == write_try(0, 0, 128, 64, 128));
	assert(1 == calls && 0 == got.x && 128 == got.w && 64 == got.h && 128 == got.pitch);
	assert(0 == write_try(8, 16, 16, 16, 128));
	assert(8 == got.x && 16 == got.y && 16 == got.w && 128 == got.pitch);
	assert(0 == write_try(0, 0, 8, 8, 200));

	assert(-EINVAL == write_try(0, 0, 129, 64, 129));
	assert(-EINVAL == write_try(0, 0, 128, 65, 128));
	assert(-EINVAL == write_try(120, 0, 9, 8, 9));
	assert(-EINVAL == write_try(0, 60, 128, 8, 128));
	assert(-EINVAL == write_try(65535, 0, 1, 8, 1));
	assert(-EINVAL == write_try(0, 65528, 8, 8, 8));
	assert(-EINVAL == write_try(0, 4, 8, 8, 8));
	assert(-EINVAL == write_try(0, 0, 8, 12, 8));
	assert(-EINVAL == write_try(0, 0, 8, 8, 7));
	assert(-EINVAL == write_try(0, 0, 0, 8, 8));
	assert(-EINVAL == write_try(0, 0, 8, 0, 8));
	assert(0 == calls);

	{
		struct display_area a = { 0, 0, 8, 8, 8 };

		assert(-EINVAL == display_write(&mock_disp, NULL, fb));
		assert(-EINVAL == display_write(&mock_disp, &a, NULL));
	}

	{
		ret_inject = -5;
		assert(-5 == write_try(0, 0, 8, 8, 8));
		ret_inject = 0;
	}

	assert(0 == display_init(&mock_disp));
	assert(0 == display_blanking_on(&mock_disp));
	assert(0 == display_blanking_off(&mock_disp));
	assert(1024u == display_frame_bytes(&mock_caps));
	return 0;
}
