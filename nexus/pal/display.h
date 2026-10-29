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

#ifndef	__NEXUS_PAL_DISPLAY_HEADER__
#define	__NEXUS_PAL_DISPLAY_HEADER__

#include <stdint.h>

enum display_fmt
{
	DISPLAY_FMT_MONO_VTILED = 1,	//	单色竖排页式
};

struct display_caps
{
	uint16_t x_res;					//	像素宽
	uint16_t y_res;					//	像素高
	enum display_fmt fmt;			//	帧格式
};

struct display_area
{
	uint16_t x;						//	区域左上列
	uint16_t y;						//	区域左上行,页对齐(8 倍数)
	uint16_t w;						//	区域列数
	uint16_t h;						//	区域行数,页对齐(8 倍数)
	uint16_t pitch;					//	buf 行距字节,>=w;buf 为区域自身数据,自 (x,y) 起
};

struct display;

struct display_ops
{
	int (*init)(const struct display *d);
	int (*write)(const struct display *d, const struct display_area *a, const uint8_t *buf);
	int (*blanking_on)(const struct display *d);	//	熄屏,显存保留
	int (*blanking_off)(const struct display *d);	//	恢复显示
};

struct display
{
	const struct display_ops *ops;
	const void *ctx;
	const struct display_caps *caps;
	const char *name;
};

int display_init(const struct display *d);
int display_write(const struct display *d, const struct display_area *a, const uint8_t *buf);
int display_blanking_on(const struct display *d);
int display_blanking_off(const struct display *d);
uint32_t display_frame_bytes(const struct display_caps *caps);

#endif	//	__NEXUS_PAL_DISPLAY_HEADER__
