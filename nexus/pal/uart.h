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

#ifndef	__NEXUS_PAL_UART_HEADER__
#define	__NEXUS_PAL_UART_HEADER__

#include <stdint.h>

struct uart_port;
struct uart_pins;

#ifdef CONFIG_UART_INTERRUPT_DRIVEN
typedef void (*uart_irq_cb_t)(const struct uart_port *p, void *ud);
#endif

#ifdef CONFIG_UART_ASYNC_API

enum uart_event_type
{
	UART_TX_DONE,
	UART_TX_ABORTED,
	UART_RX_RDY,
	UART_RX_DISABLED,
};

struct uart_event
{
	enum uart_event_type	type;
	union
	{
		struct
		{
			const uint8_t	*buf;
			uint32_t	len;
		}			tx;
		struct
		{
			uint8_t		*buf;
			uint32_t	offset;
			uint32_t	len;
		}			rx;
	}			data;
};

typedef void (*uart_async_cb_t)(const struct uart_port *p,
		const struct uart_event *evt, void *ud);

#endif

struct uart_config
{
	uint32_t	baud;
	uint8_t		data_bits;	// 8 / 9
	uint8_t		parity;		// 0 无 / 1 偶 / 2 奇
	uint8_t		stop_bits;	// 1 / 2
};

struct uart_ops
{
	int  (*init)(const struct uart_port *p, const struct uart_pins *pins,
			const struct uart_config *cfg);
	int  (*poll_out)(const struct uart_port *p, uint8_t c);
	int  (*poll_in)(const struct uart_port *p, uint8_t *c);
#ifdef CONFIG_UART_INTERRUPT_DRIVEN
	int  (*fifo_fill)(const struct uart_port *p, const uint8_t *tx, uint32_t len);
	int  (*fifo_read)(const struct uart_port *p, uint8_t *rx, uint32_t len);
	int  (*irq_update)(const struct uart_port *p);
	int  (*irq_callback_set)(const struct uart_port *p, uart_irq_cb_t cb, void *ud);
	int  (*irq_is_pending)(const struct uart_port *p);
	int  (*irq_tx_ready)(const struct uart_port *p);
	int  (*irq_rx_ready)(const struct uart_port *p);
	void (*irq_tx_disable)(const struct uart_port *p);
	void (*irq_rx_disable)(const struct uart_port *p);
	void (*irq_tx_enable)(const struct uart_port *p);
	void (*irq_rx_enable)(const struct uart_port *p);
#endif
#ifdef CONFIG_UART_ASYNC_API
	int  (*callback_set)(const struct uart_port *p, uart_async_cb_t cb, void *ud);
	int  (*tx)(const struct uart_port *p, const uint8_t *buf, uint32_t len);
	int  (*tx_abort)(const struct uart_port *p);
	int  (*rx_enable)(const struct uart_port *p, uint8_t *buf, uint32_t len);
	int  (*rx_disable)(const struct uart_port *p);
#endif
};

struct uart_port
{
	const struct uart_ops	*	ops;
	const void				*	ctx;
	const char				*	name;
};

struct uart_dev
{
	const struct uart_port	*	port;
	const struct uart_config*	cfg;
};

int  uart_dev_init(const struct uart_dev *d, const struct uart_pins *pins);
void uart_poll_putc(const struct uart_port *p, uint8_t c);
int  uart_poll_write(const struct uart_port *p, const uint8_t *buf, uint32_t len);
int  uart_poll_getc(const struct uart_port *p, uint8_t *c);

#ifdef CONFIG_UART_INTERRUPT_DRIVEN
int  uart_fifo_fill(const struct uart_port *p, const uint8_t *tx, uint32_t len);
int  uart_fifo_read(const struct uart_port *p, uint8_t *rx, uint32_t len);
int  uart_irq_update(const struct uart_port *p);
int  uart_irq_callback_set(const struct uart_port *p, uart_irq_cb_t cb, void *ud);
int  uart_irq_is_pending(const struct uart_port *p);
int  uart_irq_tx_ready(const struct uart_port *p);
int  uart_irq_rx_ready(const struct uart_port *p);
void uart_irq_tx_disable(const struct uart_port *p);
void uart_irq_rx_disable(const struct uart_port *p);
void uart_irq_tx_enable(const struct uart_port *p);
void uart_irq_rx_enable(const struct uart_port *p);
#endif

#ifdef CONFIG_UART_ASYNC_API
int  uart_callback_set(const struct uart_port *p, uart_async_cb_t cb, void *ud);
int  uart_tx(const struct uart_port *p, const uint8_t *buf, uint32_t len);
int  uart_tx_abort(const struct uart_port *p);
int  uart_rx_enable(const struct uart_port *p, uint8_t *buf, uint32_t len);
int  uart_rx_disable(const struct uart_port *p);
#endif

struct pin_desc;

struct uart_pins
{
	const struct pin_desc	*tx;
	const struct pin_desc	*rx;
};

#endif	//	__NEXUS_PAL_UART_HEADER__
