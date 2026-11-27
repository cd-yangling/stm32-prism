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

#ifndef	__APPS_WHEELTEC_F570_DEV_SENSOR_STP23L_HEADER__
#define	__APPS_WHEELTEC_F570_DEV_SENSOR_STP23L_HEADER__

#include "dev/strm_deframer.h"

struct stp23l_rec
{
	short					dist_mm[12];
	unsigned char			quality[12];
	unsigned int			ts;
};

struct stp23l
{
	struct strm_deframer	io;
	struct stp23l_rec		rec;
};

int							stp23l_init(struct stp23l *s);
void						stp23l_feed(struct stp23l *s);
const struct stp23l_rec	*	stp23l_peek(struct stp23l *s);
void						stp23l_advance(struct stp23l *s);
#if defined(CONFIG_UART_IRQ) || defined(CONFIG_UART_DMA)
unsigned int				stp23l_rx_drop_get(const struct stp23l *s);
#endif
unsigned int				stp23l_frame_drop_get(const struct stp23l *s);

#endif	//	__APPS_WHEELTEC_F570_DEV_SENSOR_STP23L_HEADER__
