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
#include <errno.h>
#include "pal/display.h"
#include "subsys/gfx.h"
#include "subsys/font_8x16.h"

#define	GFX_FB_BYTES	1024u

static const struct display *gfx_disp;
static uint8_t fb[GFX_FB_BYTES];
static int fb_w;
static int fb_h;
static uint32_t fb_n;

int gfx_attach(const struct display *d)
{
	uint32_t i;

	fb_n = display_frame_bytes(d->caps);
	if ((DISPLAY_FMT_MONO_VTILED != d->caps->fmt) ||
	    (0 != (d->caps->y_res & 7u)) ||
	    (fb_n > sizeof(fb)) || (0u == fb_n))
	{
		return -EINVAL;
	}
	gfx_disp = d;
	fb_w = d->caps->x_res;
	fb_h = d->caps->y_res;
	for (i = 0; i < fb_n; i++)
	{
		fb[i] = 0u;
	}
	return 0;
}

void gfx_px(int x, int y, int on)
{
	uint8_t mask;

	if (0 > x || fb_w <= x || 0 > y || fb_h <= y)
	{
		return;
	}
	mask = (uint8_t)(1u << (y & 7));
	if (on)
	{
		fb[(y / 8) * fb_w + x] |= mask;
	}
	else
	{
		fb[(y / 8) * fb_w + x] &= (uint8_t)~mask;
	}
}

void gfx_text(int x, int y, const char *s)
{
	const struct font *f = &font_8x16;
	int pages = f->h / 8;
	int i;

	y &= ~7;
	for (i = 0; '\0' != s[i]; i++)
	{
		int cx = x + i * f->w;
		uint8_t ch = (uint8_t)s[i];
		const uint8_t *g;
		int p;
		int c;

		if (cx + f->w > fb_w)
		{
			break;
		}
		if (ch < f->first || ch > f->last)
		{
			ch = '?';
		}
		g = f->data + (ch - f->first) * f->h;
		for (p = 0; p < pages; p++)
		{
			for (c = 0; c < f->w; c++)
			{
				fb[(y / 8 + p) * fb_w + cx + c] = g[p * f->w + c];
			}
		}
	}
}

void gfx_fill(int on)
{
	uint32_t i;
	uint8_t v = on ? 0xFFu : 0x00u;

	for (i = 0; i < fb_n; i++)
	{
		fb[i] = v;
	}
}

int gfx_flush(int x, int y, int w, int h)
{
	struct display_area a;

	if (NULL == gfx_disp)
	{
		return -ENODEV;
	}
	if (0 > x)
	{
		w += x;
		x = 0;
	}
	if (0 > y)
	{
		h += y;
		y = 0;
	}
	if (fb_w - x < w)
	{
		w = fb_w - x;
	}
	if (fb_h - y < h)
	{
		h = fb_h - y;
	}
	if (0 >= w || 0 >= h)
	{
		return 0;
	}
	a.x = (uint16_t)x;
	a.y = (uint16_t)(y & ~7);
	a.w = (uint16_t)w;
	a.h = (uint16_t)(((y + h + 7) & ~7) - a.y);
	a.pitch = (uint16_t)fb_w;
	return display_write(gfx_disp, &a, &fb[(a.y / 8) * fb_w + a.x]);
}
