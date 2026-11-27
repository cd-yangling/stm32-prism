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

#include <stddef.h>
#include "lib/deframer.h"
#include "lib/gadget/util.h"
#include "dev/sensor/stp23l.h"

int stp23l_init(struct stp23l *s)
{
	return strm_deframer_init(&s->io);
}

void stp23l_feed(struct stp23l *s)
{
	strm_deframer_feed(&s->io);
}

const struct stp23l_rec *stp23l_peek(struct stp23l *s)
{
	const struct deframer_rec *fr = strm_deframer_peek(&s->io);
	unsigned int n;

	if (NULL == fr)
	{
		return NULL;
	}

	for (n = 0u; 12u > n; n++)
	{
		s->rec.dist_mm[n] = (short)get_le16(&fr->buf[10u + n * 15u]);
		s->rec.quality[n] = fr->buf[10u + n * 15u + 8u];
	}

	s->rec.ts = fr->ts;

	return &s->rec;
}

void stp23l_advance(struct stp23l *s)
{
	strm_deframer_advance(&s->io);
}

#if defined(CONFIG_UART_IRQ) || defined(CONFIG_UART_DMA)
unsigned int stp23l_rx_drop_get(const struct stp23l *s)
{
	return strm_deframer_rx_drop_get(&s->io);
}
#endif

unsigned int stp23l_frame_drop_get(const struct stp23l *s)
{
	return strm_deframer_frame_drop_get(&s->io);
}
