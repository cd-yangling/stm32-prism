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

#if defined(CONFIG_I2C1) || defined(CONFIG_I2C2)
#include <stdint.h>
#include <errno.h>
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_i2c.h"
#include "stm32f1xx_ll_gpio.h"
#include "pal/i2c.h"
#include "pal/pin.h"
#include "tick.h"
#include "irq.h"

#define	I2C_DEFAULT_SPD_HZ	(100 * 1000)	//	100KHz
#define I2C_DEFAULT_TMO_MS	(100u)			//	100ms
#define	I2C_TMO_CRITICAL_MS	(35u)

struct i2c_dat
{
	uint32_t			cur_spd_hz;
};

struct i2c_ctx
{
	I2C_TypeDef		*	regs;
	struct i2c_dat	*	data;
};

static void f1_i2c_start(const struct i2c_bus *b)
{
	const struct i2c_ctx *c = b->ctx;

	LL_I2C_DisableBitPOS(c->regs);
	LL_I2C_GenerateStartCondition(c->regs);
}

static void f1_i2c_stop(const struct i2c_bus *b)
{
	const struct i2c_ctx *c = b->ctx;

	LL_I2C_GenerateStopCondition(c->regs);
}

static inline int f1_I2Ce_detect(const struct i2c_ctx *c, uint32_t t0)
{
	if (LL_I2C_IsActiveFlag_AF(c->regs))
	{
		LL_I2C_ClearFlag_AF(c->regs);
		return -ENXIO;
	}
	else if (LL_I2C_IsActiveFlag_BERR(c->regs) || LL_I2C_IsActiveFlag_ARLO(c->regs))
	{
		LL_I2C_ClearFlag_BERR(c->regs);
		LL_I2C_ClearFlag_ARLO(c->regs);
		return -EIO;
	}
	else
	{
		if ((tick_get() - t0) >= I2C_DEFAULT_TMO_MS)
			return -ETIMEDOUT;
		else
			return 0;
	}
}

static int f1_wait_sb(const struct i2c_ctx *c, uint32_t t0)
{
	int ret = 0;

	//	EV5 等待 SB = 1
	//
	//	SB: 起始位(主模式) (Start bit (Master mode))
	//		0: 未发送起始条件
	//		1: 起始条件已发送
	//	- 当发送出起始条件时该位被置'1'.
	//	- 软件读取SR1寄存器后, 数据寄存器的操作将清除该位,当PE=0时, 硬件清除该位.
	//
	//	SB: Start bit (Master mode)
	//		0: No Start condition
	//		1: Start condition generated.
	//	- Set when a Start condition generated.
	//	- Cleared by software by reading the SR1 register followed by writing the DR register, or by hardware when PE=0

	while (((!LL_I2C_IsActiveFlag_SB(c->regs))) && (0 == (ret = f1_I2Ce_detect(c, t0)))) { }

	return ret;
}

static int f1_wait_addr(const struct i2c_ctx *c, uint32_t t0)
{
	int ret = 0;

	//	EV6 等待 ADDR = 1
	//
	//	ADDR:地址已被发送(主模式)/地址匹配(从模式) (Address sent (master mode)/matched (slave mode))
	//	在软件读取SR1寄存器后, SR2寄存器的读操作将清除该位, 或当PE=0时, 由硬件清除该位.
	//	地址已被发送(主模式)
	//		0: 地址发送没有结束
	//		1: 地址发送结束
	//	- 7位地址模式时, 当收到地址的ACK后该位被置'1'
	//	注:在收到NACK后, ADDR位不会被置位

	//	ADDR: Address sent (master mode)/matched (slave mode)
	//	This bit is cleared by software reading SR1 register followed reading SR2, or by hardware when PE=0.
	//	Address sent (Master)
	//		0: No end of address transmission
	//		1: End of address transmission
	//	- For 7-bit addressing, the bit is set after the ACK of the byte.
	//	Note: ADDR is not set after a NACK reception
	while (((!LL_I2C_IsActiveFlag_ADDR(c->regs))) && (0 == (ret = f1_I2Ce_detect(c, t0)))) { }

	return ret;
}

static int f1_wait_txe(const struct i2c_ctx *c, uint32_t t0)
{
	int ret = 0;

	//	EV8_1 等待 TxE = 1
	//	TxE: 数据寄存器为空(发送时) (Data register empty (transmitters))
	//		0: 数据寄存器非空
	//		1: 数据寄存器空
	//	- 在发送数据时, 数据寄存器为空时该位被置'1', 在发送地址阶段不设置该位
	//	- 软件写数据到DR寄存器可清除该位; 或在发生一个起始或停止条件后,  或当PE=0时由硬件自动清除
	//	如果收到一个NACK, 或下一个要发送的字节是PEC(PEC=1), 该位不被置位
	//	注: 在写入第1个要发送的数据后, 或设置了BTF时写入数据, 都不能清除TxE位, 这是因为数据寄存器仍然为空

	//	TxE: Data register empty (transmitters)
	//		0: Data register not empty
	//		1: Data register empty
	//	- Set when DR is empty in transmission. TxE is not set during address phase.
	//	- Cleared by software writing to the DR register or by hardware after a start or a stop condition or when PE=0.
	//	TxE is not set if either a NACK is received, or if next byte to be transmitted is PEC (PEC=1)
	//	Note: TxE is not cleared by writing the first data being transmitted, or by writing data when
	//		BTF is set, as in both cases the data register is still empty.
	while (((!LL_I2C_IsActiveFlag_TXE(c->regs))) && (0 == (ret = f1_I2Ce_detect(c, t0)))) { }

	return ret;
}

static int f1_wait_txe_and_btf(const struct i2c_ctx *c, uint32_t t0)
{
	int ret = 0;

	//	EV8_2 等待 TxE = 1 && BTF = 1
	//	TxE: 数据寄存器为空(发送时) (Data register empty (transmitters))
	//		0: 数据寄存器非空
	//		1: 数据寄存器空
	//	- 在发送数据时,  数据寄存器为空时该位被置'1', 在发送地址阶段不设置该位
	//	- 软件写数据到DR寄存器可清除该位; 或在发生一个起始或停止条件后, 或当PE=0时由硬件自动清除
	//	如果收到一个NACK, 或下一个要发送的字节是PEC(PEC=1), 该位不被置位
	//	注: 在写入第1个要发送的数据后, 或设置了BTF时写入数据, 都不能清除TxE位, 这是因为数据寄存器仍然为空

	//	TxE: Data register empty (transmitters)
	//		0: Data register not empty
	//		1: Data register empty
	//	- Set when DR is empty in transmission. TxE is not set during address phase.
	//	- Cleared by software writing to the DR register or by hardware after a start or a stop condition or when PE=0.
	//	TxE is not set if either a NACK is received, or if next byte to be transmitted is PEC (PEC=1)
	//	Note: TxE is not cleared by writing the first data being transmitted, or by writing data when
	//		BTF is set, as in both cases the data register is still empty.
	//
	//	BTF: 字节发送结束 (Byte transfer finished)
	//		0: 字节发送未完成
	//		1: 字节发送结束
	//	当NOSTRETCH=0时, 在下列情况下硬件将该位置'1'
	//	- 在接收时, 当收到一个新字节(包括ACK脉冲)且数据寄存器还未被读取(RxNE=1)
	//	- 在发送时, 当一个新数据将被发送且数据寄存器还未被写入新的数据(TxE=1)
	//	- 在软件读取SR1寄存器后, 对数据寄存器的读或写操作将清除该位; 或在传输中发送一个起始或停止条件后, 或当PE=0时, 由硬件清除该位
	//	注:在收到一个NACK后, BTF位不会被置位. 如果下一个要传输的字节是PEC(I2C_SR2寄存器中TRA为'1', 同时I2C_CR1寄存器中PEC 为'1'), BTF位不会被置位

	//	BTF: Byte transfer finished
	//		0: Data byte transfer not done
	//		1: Data byte transfer succeeded
	//	- Set by hardware when NOSTRETCH=0 and:
	//	- In reception when a new byte is received (including ACK pulse) and DR has not been read yet (RxNE=1).
	//	- In transmission when a new byte should be sent and DR has not been written yet (TxE=1).
	//	- Cleared by software reading SR1 followed by either a read or write in the DR register or by
	//	hardware after a start or a stop condition in transmission or when PE=0.
	//	Note: The BTF bit is not set after a NACK reception
	//	The BTF bit is not set if next byte to be transmitted is the PEC (TRA=1 in I2C_SR2 register and PEC=1 in I2C_CR1 register)

	while (((!LL_I2C_IsActiveFlag_TXE(c->regs)) || (!LL_I2C_IsActiveFlag_BTF(c->regs))) && (0 == (ret = f1_I2Ce_detect(c, t0)))) { }

	return ret;
}

static int f1_wait_btf(const struct i2c_ctx *c, uint32_t t0)
{
	int ret = 0;

	while (((!LL_I2C_IsActiveFlag_BTF(c->regs))) && (0 == (ret = f1_I2Ce_detect(c, t0)))) { }

	return ret;
}

static int f1_wait_rxne(const struct i2c_ctx *c, uint32_t t0)
{
	int ret = 0;

	while (((!LL_I2C_IsActiveFlag_RXNE(c->regs))) && (0 == (ret = f1_I2Ce_detect(c, t0)))) { }

	return ret;
}

static int f1_i2c_master_transmit(const struct i2c_ctx *c, uint8_t addr7, const uint8_t *buf, uint16_t len, uint32_t t0)
{
	int ret;

	ret = f1_wait_sb(c, t0);
	if (ret)
		return ret;

	//	发送地址信息
	LL_I2C_TransmitData8(c->regs, (addr7 << 1) | 0);

	ret = f1_wait_addr(c, t0);
	if (ret)
		return ret;

	LL_I2C_ClearFlag_ADDR(c->regs);

	if (0 == len)
		return 0;

	for (int i = 0; i < len; i++)
	{
		ret = f1_wait_txe(c, t0);
		if (ret)
			return ret;

		LL_I2C_TransmitData8(c->regs, buf[i]);
	}

	return f1_wait_txe_and_btf(c, t0);
}

static int f1_i2c_master_receive(const struct i2c_ctx *c, uint8_t addr7, uint8_t *buf, uint16_t len, uint32_t t0)
{
	int ret;

	ret = f1_wait_sb(c, t0);
	if (ret)
		return ret;

	//	发送地址信息
	LL_I2C_TransmitData8(c->regs, (addr7 << 1) | 1);

	ret = f1_wait_addr(c, t0);
	if (ret)
		return ret;

	if (0 == len)
	{
		LL_I2C_ClearFlag_ADDR(c->regs);
		LL_I2C_GenerateStopCondition(c->regs);	//	特殊情况, 只能提前发送STOP

		return ret;
	}
	else if (1 == len)
	{
		unsigned int key;
		LL_I2C_AcknowledgeNextData(c->regs, LL_I2C_NACK);

		key = irq_lock();
		LL_I2C_ClearFlag_ADDR(c->regs);
		LL_I2C_GenerateStopCondition(c->regs);	//	特殊情况, 只能提前发送STOP
		irq_unlock(key);
	}
	else if (2 == len)
	{
		unsigned int key;
		LL_I2C_EnableBitPOS(c->regs);

		key = irq_lock();
		LL_I2C_ClearFlag_ADDR(c->regs);
		LL_I2C_AcknowledgeNextData(c->regs, LL_I2C_NACK);
		irq_unlock(key);
	}
	else
	{
		LL_I2C_AcknowledgeNextData(c->regs, LL_I2C_ACK);
		LL_I2C_ClearFlag_ADDR(c->regs);
	}

	while (len)
	{
		if (len <= 3)
		{
			if (1 == len)
			{
				ret = f1_wait_rxne(c, t0);
				if (ret)
					return ret;

				*buf = LL_I2C_ReceiveData8(c->regs); buf++; len--;
			}
			else if (2 == len)
			{
				unsigned int key;
				ret = f1_wait_btf(c, t0);
				if (ret)
					return ret;

				key = irq_lock();
				LL_I2C_GenerateStopCondition(c->regs);	//	特殊情况, 只能提前发送STOP
				*buf = LL_I2C_ReceiveData8(c->regs); buf++; len--;
				irq_unlock(key);
				*buf = LL_I2C_ReceiveData8(c->regs); buf++; len--;
			}
			else
			{
				volatile int count;
				unsigned int key;

				ret = f1_wait_btf(c, t0);
				if (ret)
					return ret;

				LL_I2C_AcknowledgeNextData(c->regs, LL_I2C_NACK);

				key = irq_lock();

				*buf = LL_I2C_ReceiveData8(c->regs); buf++; len--;

				count = (I2C_TMO_CRITICAL_MS) * (SystemCoreClock / 25U / 1000U);
				do
				{
					//	因为这里禁止了中断,所以一定不能使用 EVx_wait_BTF 函数
					count--;
					if (0 == count)
					{
						irq_unlock(key);
						return -ETIMEDOUT;
					}
				}
				while (!LL_I2C_IsActiveFlag_BTF(c->regs));

				LL_I2C_GenerateStopCondition(c->regs);	//	特殊情况, 只能提前发送STOP

				*buf = LL_I2C_ReceiveData8(c->regs); buf++; len--;
				irq_unlock(key);
				*buf = LL_I2C_ReceiveData8(c->regs); buf++; len--;
			}
		}
		else
		{
			ret = f1_wait_rxne(c, t0);
			if (ret)
				return ret;

			*buf = LL_I2C_ReceiveData8(c->regs); buf++; len--;

			if (LL_I2C_IsActiveFlag_BTF(c->regs))
			{
				if (3 == len)
				{
					LL_I2C_AcknowledgeNextData(c->regs, LL_I2C_NACK);
				}

				*buf = LL_I2C_ReceiveData8(c->regs); buf++; len--;
			}
		}
	}

	return ret;
}

static int f1_i2c_seg(const struct i2c_bus *b, uint8_t addr7, const struct i2c_msg *m)
{
	const struct i2c_ctx *c = b->ctx;
	uint32_t t0 = tick_get();

	if (m->flags & I2C_MSG_READ)
		return f1_i2c_master_receive(c, addr7, m->buf, m->len, t0);
	else
		return f1_i2c_master_transmit(c, addr7, m->buf, m->len, t0);
}

static int f1_i2c_xfer(const struct i2c_bus *b, uint8_t addr7, const struct i2c_msg *m, int n)
{
	int ret = 0;

	for (int i = 0; i < n && 0 == ret; i++)
	{
		f1_i2c_start(b);
		ret = f1_i2c_seg(b, addr7, &m[i]);
	}

	f1_i2c_stop(b);

	return ret;
}

static void f1_i2c_config(const struct i2c_bus *b, uint32_t spd_hz)
{
	const struct i2c_ctx *c = b->ctx;
	LL_RCC_ClocksTypeDef rcc_clock;

	if (spd_hz == c->data->cur_spd_hz)
		return;

	c->data->cur_spd_hz = spd_hz;

	LL_RCC_GetSystemClocksFreq(&rcc_clock);

	LL_I2C_Disable(c->regs);

	LL_I2C_ConfigSpeed(c->regs, rcc_clock.PCLK1_Frequency, spd_hz, LL_I2C_DUTYCYCLE_2);

	LL_I2C_Enable(c->regs);

	return;
}

static int f1_i2c_init(const struct i2c_bus *b, const struct i2c_pins *pins)
{
	LL_RCC_ClocksTypeDef rcc_clock;
	const struct i2c_ctx *c = b->ctx;

	if (I2C1 == c->regs)
	{
		uint32_t mux = pins->scl->flags & PIN_MUX_MASK;

		if (0 != mux)
		{
			LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_AFIO);
			LL_GPIO_AF_EnableRemap_I2C1();
		}
		LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);
	}
	else
	{
		LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C2);
	}

	c->data->cur_spd_hz = I2C_DEFAULT_SPD_HZ;

	pin_configure(pins->scl);
	pin_configure(pins->sda);

	LL_I2C_Disable(c->regs);

	LL_RCC_GetSystemClocksFreq(&rcc_clock);

	LL_I2C_ConfigSpeed(c->regs, rcc_clock.PCLK1_Frequency, c->data->cur_spd_hz, LL_I2C_DUTYCYCLE_2);

	LL_I2C_SetOwnAddress1(c->regs, 0x00, LL_I2C_OWNADDRESS1_7BIT);

	LL_I2C_SetMode(c->regs, LL_I2C_MODE_I2C);

	LL_I2C_Enable(c->regs);

	LL_I2C_AcknowledgeNextData(c->regs, LL_I2C_NACK);

	return 0;
}

static const struct i2c_ops i2cd_ops =
{
	.init		= f1_i2c_init,
	.xfer		= f1_i2c_xfer,
	.config		= f1_i2c_config,
};

#if defined(I2C1) && defined(CONFIG_I2C1)
static struct i2c_dat i2c1_data = 
{
	.cur_spd_hz	= 0,
};
static const struct i2c_ctx i2c1_ctx =
{
	.regs		= I2C1,
	.data		= &i2c1_data,
};
const struct i2c_bus i2c1 =
{
	.ops		= &i2cd_ops,
	.ctx		= &i2c1_ctx,
	.name		= "i2c1",
};
#endif	//	I2C1

#if defined(I2C2) && defined(CONFIG_I2C2)
static struct i2c_dat i2c2_data =
{
	.cur_spd_hz	= 0,
};
static const struct i2c_ctx i2c2_ctx =
{
	.regs		= I2C2,
	.data		= &i2c2_data,
};
const struct i2c_bus i2c2 =
{
	.ops		= &i2cd_ops,
	.ctx		= &i2c2_ctx,
	.name		= "i2c2",
};
#endif	//	I2C2
#endif

