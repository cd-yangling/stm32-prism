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
#ifndef	__APPS_WHEELTEC_F570_DEV_STRM_DEFRAMER_HEADER__
#define	__APPS_WHEELTEC_F570_DEV_STRM_DEFRAMER_HEADER__

#include "lib/deframer.h"
#if defined(CONFIG_UART_IRQ) || defined(CONFIG_UART_DMA)
#include "lib/gadget/fifo.h"

#define	STRM_DEFRAMER_QUE_RX_LEN	256u
#endif

#ifdef CONFIG_UART_DMA
#define	STRM_DEFRAMER_DMA_RX_LEN	64u
#endif

struct uart_dev;

enum strm_deframer_mode
{
	STRM_DEFRAMER_POLL,
	STRM_DEFRAMER_IRQ,
	STRM_DEFRAMER_ASYNC,
};

struct strm_deframer
{
	const struct uart_dev		*	dev;
	const struct deframer_cfg	*	dfr_cfg;
	enum strm_deframer_mode			mode;
	struct deframer					dfr;
#if defined(CONFIG_UART_IRQ) || defined(CONFIG_UART_DMA)
	struct fifo						rxq;
	unsigned char					rxq_buf[STRM_DEFRAMER_QUE_RX_LEN];
	volatile unsigned int			rx_drop;
#endif
#ifdef CONFIG_UART_DMA
	unsigned char					dma_buf[STRM_DEFRAMER_DMA_RX_LEN];
#endif
};

int								strm_deframer_init(struct strm_deframer *s);
void							strm_deframer_feed(struct strm_deframer *s);
const struct deframer_rec	*	strm_deframer_peek(const struct strm_deframer *s);
void							strm_deframer_advance(struct strm_deframer *s);
#if defined(CONFIG_UART_IRQ) || defined(CONFIG_UART_DMA)
unsigned int					strm_deframer_rx_drop_get(const struct strm_deframer *s);
#endif
unsigned int					strm_deframer_frame_drop_get(const struct strm_deframer *s);

#endif	//	__APPS_WHEELTEC_F570_DEV_STRM_DEFRAMER_HEADER__
