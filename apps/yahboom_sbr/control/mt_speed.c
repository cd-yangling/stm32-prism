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
#include "tick.h"
#include "irq.h"
#include "pal/enc.h"
#include "pal/pin.h"
#include "control/mt_speed.h"

#define	MT_CYC_PER_SEC	(tick_cyc_per_us() * 1000000u)

#define	MT_STOP_TIMEOUT_CYC	(tick_cyc_per_us() * 100000u)

//	理论上最大转速下的计数率
//	商家给的最大 RPM 是 11000 RPM AB编码器 11 线
#define	MT_MAX_CNT_PER_SEC	(44u * 11000u / 60u)

//	理论上最大转速下的计数率再提升 1/4
//	算出来如果比它大就不是真速度, 判为噪声
#define	MT_VEL_MAX	((int64_t)MT_MAX_CNT_PER_SEC * 5 / 4)

struct mt_speed
{
	volatile uint32_t	cyc_last;	//	最近边沿时刻 DWT->CYCCNT
	volatile uint16_t	cnt_last;	//	最近边沿计数 TIMx->CNT

	const struct enc_dev	*dev;
	struct pin_callback	cb;

	uint32_t	cyc_prev;			//	上次采样的边沿时刻
	uint16_t	cnt_prev;			//	上次采样的边沿计数
	int16_t		vel_calc;			//	速度 单位: 计数/秒
	uint8_t		primed;
};

static struct mt_speed mt_spd_l;
static struct mt_speed mt_spd_r;

static struct mt_speed *mt_of_cb(struct pin_callback *cb)
{
	return (struct mt_speed *)(void *)((uint8_t *)cb
		- offsetof(struct mt_speed, cb));
}

static void mt_isr(const struct pin_port *pt, struct pin_callback *cb)
{
	struct mt_speed *s = mt_of_cb(cb);

	(void)pt;

	s->cyc_last = tick_get_cyc();
	s->cnt_last = enc_get_raw(s->dev);
}

static int mt_attach(struct mt_speed *s, const struct enc_ch *c,
	const struct pin_desc *pin)
{
	int ret;

	s->dev = c->dev;
	pin_callback_init(&s->cb, mt_isr, pin->pin);

	ret = pin_irq_configure(pin, PIN_INT_BOTH);

	if (0 != ret)
	{
		return ret;
	}

	return pin_add_callback(pin->port, &s->cb);
}

int mt_speed_init(const struct enc_ch *enc_l, const struct enc_ch *enc_r)
{
	int ret = mt_attach(&mt_spd_l, enc_l, enc_l->pin_a);

	if (0 != ret)
	{
		return ret;
	}

	return mt_attach(&mt_spd_r, enc_r, enc_r->pin_b);
}

//	返回速度: 带符号的编码器计数/秒 范围: [-10082, 10082]
static int16_t mt_speed_calc(struct mt_speed * s, uint32_t cyc, uint16_t cnt)
{
	if (0u == s->primed)
	{
		s->primed = 1u;
		s->cyc_prev = cyc;
		s->cnt_prev = cnt;
		s->vel_calc = 0;
	}
	else if (cyc != s->cyc_prev)
	{
		uint32_t dcyc = cyc - s->cyc_prev;
		int32_t dcnt = (int16_t)(cnt - s->cnt_prev);
		int64_t vel = (int64_t)dcnt * MT_CYC_PER_SEC / (int64_t)dcyc;

		if (MT_STOP_TIMEOUT_CYC < dcyc)
		{
			//	有新边沿, 但窗口跨了太久, 中间停过, 平均值不作数
			s->vel_calc = 0;
			s->cyc_prev = cyc;
			s->cnt_prev = cnt;
		}
		else if (MT_VEL_MAX >= vel && -MT_VEL_MAX <= vel)
		{
			s->vel_calc = (int16_t)vel;
			s->cyc_prev = cyc;
			s->cnt_prev = cnt;
		}

		//	剩下的: 可能是噪声, 也可能真超了 25%, 保持上次值, prev 不动, 窗口拉长下一次重算

	}

	//	超过了 MT_STOP_TIMEOUT_CYC 一直没新边沿, 直接速度判 0
	if (tick_expired_cyc(cyc + MT_STOP_TIMEOUT_CYC))
	{
		s->vel_calc = 0;
	}

	return s->vel_calc;
}

void mt_speed_v2(int16_t * vel_l, int16_t * vel_r)
{
	unsigned int key;
	uint32_t cyc_l, cyc_r;
	uint16_t cnt_l, cnt_r;

	key = irq_lock();
	cyc_l = mt_spd_l.cyc_last; cnt_l = mt_spd_l.cnt_last;
	cyc_r = mt_spd_r.cyc_last; cnt_r = mt_spd_r.cnt_last;
	irq_unlock(key);

	*vel_l = mt_speed_calc(&mt_spd_l, cyc_l, cnt_l);
	*vel_r = mt_speed_calc(&mt_spd_r, cyc_r, cnt_r);
}
