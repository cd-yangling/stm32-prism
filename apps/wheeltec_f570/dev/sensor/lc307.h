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

#ifndef	__APPS_WHEELTEC_F570_DEV_SENSOR_LC307_HEADER__
#define	__APPS_WHEELTEC_F570_DEV_SENSOR_LC307_HEADER__

#include "dev/strm_deframer.h"

struct lc307_rec
{
	short					flowx;
	short					flowy;
	unsigned short			dt_us;
	unsigned char			valid;
	unsigned char			ver;
	unsigned int			ts;
};

struct lc307
{
	struct strm_deframer	io;
	struct lc307_rec		rec;
};

int							lc307_init(struct lc307 *s);
void						lc307_feed(struct lc307 *s);
const struct lc307_rec	*	lc307_peek(struct lc307 *s);
void						lc307_advance(struct lc307 *s);
float						lc307_flow_rad(short flowv, unsigned short dt_us);
#if defined(CONFIG_UART_IRQ) || defined(CONFIG_UART_DMA)
unsigned int				lc307_rx_drop_get(const struct lc307 *s);
#endif
unsigned int				lc307_frame_drop_get(const struct lc307 *s);

#endif	//	__APPS_WHEELTEC_F570_DEV_SENSOR_LC307_HEADER__
