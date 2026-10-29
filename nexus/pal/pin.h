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

#ifndef	__NEXUS_PAL_PIN_HEADER__
#define	__NEXUS_PAL_PIN_HEADER__

#include <stdint.h>

//	方向 2bit
#define	PIN_IN					(0u << 0)
#define	PIN_OUT					(1u << 0)
#define	PIN_ALT					(2u << 0)
#define	PIN_ANALOG				(3u << 0)
#define	PIN_DIR_MASK			(3u << 0)

//	输出形态(OUT/ALT 有效)
#define	PIN_OD					(1u << 2)

//	上下拉(IN 有效)
#define	PIN_PULL_UP				(1u << 3)
#define	PIN_PULL_DOWN			(1u << 4)

//	速度(OUT/ALT 必填)
#define	PIN_SPEED_LOW			(0u << 5)
#define	PIN_SPEED_MID			(1u << 5)
#define	PIN_SPEED_HIGH			(2u << 5)
#define	PIN_SPEED_VERY_HIGH		(3u << 5)
#define	PIN_SPEED_MASK			(3u << 5)

//	初始电平: 逻辑
#define	PIN_INIT_ACTIVE			(1u << 7)
#define	PIN_INIT_INACTIVE		(2u << 7)

//	极性: 物理
#define	PIN_ACTIVE_LOW			(1u << 9)

//	复用 4bit:F1=remap 选择(外设 drv init 处理);F4/F7/H7=AF0-15(configure 处理)
#define	PIN_MUX(n)				(((n) & 0xFu) << 10)
#define	PIN_MUX_MASK			(0xFu << 10)

//	触发沿:irq_configure 的 flags 参数域(位 10-13 是 MUX 域)
#define	PIN_INT_DISABLE			(0u)
#define	PIN_INT_RISING			(1u << 14)
#define	PIN_INT_FALLING			(1u << 15)
#define	PIN_INT_BOTH			(PIN_INT_RISING | PIN_INT_FALLING)

struct pin_port;

struct pin_callback
{
	void (*handler)(const struct pin_port *pt, struct pin_callback *cb);
	uint8_t			 		pin;
};

static inline void pin_callback_init(struct pin_callback *cb,
	void (*handler)(const struct pin_port *, struct pin_callback *),
	uint8_t pin)
{
	cb->handler	= handler;
	cb->pin		= pin;
}

struct pin_ops
{
	void (*configure)(const struct pin_port *pt, uint8_t pin, uint32_t flags);
	void (*toggle)(const struct pin_port *pt, uint8_t pin);
	void (*set)(const struct pin_port *pt, uint8_t pin, int on);
	int (*get)(const struct pin_port *pt, uint8_t pin);
	int (*irq_configure)(const struct pin_port *pt, uint8_t pin, uint32_t flags);
	int (*manage_cb)(const struct pin_port *pt, struct pin_callback *cb, int set);
};

struct pin_port
{
	const struct pin_ops	*ops;
	const void				*ctx;
	const char				*name;
};

struct pin_desc
{
	const struct pin_port	*port;
	uint8_t			 		pin;
	uint32_t			 	flags;
};

void pin_configure(const struct pin_desc *d);

static inline __attribute__((always_inline)) void pin_set(const struct pin_desc *d, int on)
{
	if (0 != (d->flags & PIN_ACTIVE_LOW))
	{
		on = !on;
	}

	d->port->ops->set(d->port, d->pin, on);
}

static inline __attribute__((always_inline)) void pin_toggle(const struct pin_desc *d)
{
	d->port->ops->toggle(d->port, d->pin);
}

static inline __attribute__((always_inline)) int pin_get(const struct pin_desc *d)
{
	int v = d->port->ops->get(d->port, d->pin);

	if (0 != (d->flags & PIN_ACTIVE_LOW))
	{
		v = !v;
	}

	return v;
}
int pin_irq_configure(const struct pin_desc *d, uint32_t flags);
int pin_add_callback(const struct pin_port *pt, struct pin_callback *cb);
int pin_remove_callback(const struct pin_port *pt, struct pin_callback *cb);

//	接线宏: 缺脚/pin>=16 编译期检查(PINS 掩码 chip 层面定义)
#define	_PIN_CAT_(a, b)	a##b
#define	_PIN_CAT(a, b)	_PIN_CAT_(a, b)

#define	PIN_DESC(var, port, n, fl) \
	_Static_assert(((n) < 16) && (_PIN_CAT(port, _PINS) & (1u << (n))), \
		#port " missing pin " #n); \
	static const struct pin_desc var = \
	{ \
		&_PIN_CAT(port, _port), \
		(n), \
		(fl) \
	}

#endif	//	__NEXUS_PAL_PIN_HEADER__
