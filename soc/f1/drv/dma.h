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

#ifndef	__SOC_F1_DRV_DMA_HEADER__
#define	__SOC_F1_DRV_DMA_HEADER__

#include <stdint.h>
#include "stm32f1xx.h"

enum dma_dir
{
	DMA_DIR_PERIPH_TO_MEM,
	DMA_DIR_MEM_TO_PERIPH,
};

enum dma_width
{
	DMA_WIDTH_U8,
	DMA_WIDTH_U16,
	DMA_WIDTH_U32,
};

enum dma_mode
{
	DMA_MODE_SINGLE,
	DMA_MODE_CYCLIC,
};

enum dma_event_type
{
	DMA_EV_HALF_DONE,
	DMA_EV_DONE,
	DMA_EV_ERROR,
};

struct dma_chan
{
	DMA_TypeDef		*	ctrl;
	uint32_t			ch;
	volatile void	*	par;
	enum dma_dir		dir;
	enum dma_width		width;
	uint8_t				prio;
};

struct dma_xfer
{
	uint8_t			*	buf;
	uint32_t			len;
	enum dma_mode		mode;
	void			(*	cb)(const struct dma_chan *c, enum dma_event_type ev, void *ud);
	void			*	ud;
};

int			dma_start(const struct dma_chan *c, const struct dma_xfer *x);
int			dma_stop(const struct dma_chan *c);
uint32_t	dma_remain(const struct dma_chan *c);

#define	DMA1_CHAN_OWN(_ch, _par, _dir, _width, _prio)					\
	_Static_assert(((_ch) >= 1) && ((_ch) <= 7), "DMA1 channel 1..7");	\
	const struct dma_chan dma1_ch##_ch = {								\
		.ctrl	= DMA1,													\
		.ch		= (_ch),												\
		.par	= (_par),												\
		.dir	= (_dir),												\
		.width	= (_width),												\
		.prio	= (_prio),												\
	}

#if defined(DMA2)
#define	DMA2_CHAN_OWN(_ch, _par, _dir, _width, _prio)					\
	_Static_assert(((_ch) >= 1) && ((_ch) <= 5), "DMA2 channel 1..5");	\
	const struct dma_chan dma2_ch##_ch = {								\
		.ctrl	= DMA2,													\
		.ch		= (_ch),												\
		.par	= (_par),												\
		.dir	= (_dir),												\
		.width	= (_width),												\
		.prio	= (_prio),												\
	}
#endif

#endif	//	__SOC_F1_DRV_DMA_HEADER__
