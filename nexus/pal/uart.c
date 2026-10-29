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
#include <limits.h>
#include <errno.h>
#include "pal/uart.h"

int uart_dev_init(const struct uart_dev *d, const struct uart_pins *pins)
{
	return d->port->ops->init(d->port, pins, d->cfg);
}

void uart_poll_putc(const struct uart_port *p, uint8_t c)
{
	(void)p->ops->poll_out(p, c);
}

int uart_poll_write(const struct uart_port *p, const uint8_t *buf, uint32_t len)
{
	uint32_t i;

	if ((uint32_t)INT_MAX < len)
	{
		len = (uint32_t)INT_MAX;
	}

	for (i = 0; i < len; i++)
	{
		if (0 != p->ops->poll_out(p, buf[i]))
			break;
	}

	return (int)i;
}

int uart_poll_getc(const struct uart_port *p, uint8_t *c)
{
	return p->ops->poll_in(p, c);
}

#ifdef CONFIG_UART_INTERRUPT_DRIVEN
int uart_fifo_fill(const struct uart_port *p, const uint8_t *tx, uint32_t len)
{
	if (NULL == p->ops->fifo_fill)
		return -ENOSYS;

	if ((uint32_t)INT_MAX < len)
	{
		len = (uint32_t)INT_MAX;
	}

	return p->ops->fifo_fill(p, tx, len);
}

int uart_fifo_read(const struct uart_port *p, uint8_t *rx, uint32_t len)
{
	if (NULL == p->ops->fifo_read)
		return -ENOSYS;

	if ((uint32_t)INT_MAX < len)
	{
		len = (uint32_t)INT_MAX;
	}

	return p->ops->fifo_read(p, rx, len);
}

int uart_irq_update(const struct uart_port *p)
{
	if (NULL == p->ops->irq_update)
		return -ENOSYS;

	return p->ops->irq_update(p);
}

int uart_irq_callback_set(const struct uart_port *p, uart_irq_cb_t cb, void *ud)
{
	if (NULL == p->ops->irq_callback_set)
		return -ENOSYS;

	return p->ops->irq_callback_set(p, cb, ud);
}

int uart_irq_is_pending(const struct uart_port *p)
{
	if (NULL == p->ops->irq_is_pending)
		return -ENOSYS;

	return p->ops->irq_is_pending(p);
}

int uart_irq_tx_ready(const struct uart_port *p)
{
	if (NULL == p->ops->irq_tx_ready)
		return -ENOSYS;

	return p->ops->irq_tx_ready(p);
}

int uart_irq_rx_ready(const struct uart_port *p)
{
	if (NULL == p->ops->irq_rx_ready)
		return -ENOSYS;

	return p->ops->irq_rx_ready(p);
}

void uart_irq_tx_disable(const struct uart_port *p)
{
	if (NULL == p->ops->irq_tx_disable)
		return;

	p->ops->irq_tx_disable(p);
}

void uart_irq_rx_disable(const struct uart_port *p)
{
	if (NULL == p->ops->irq_rx_disable)
		return;

	p->ops->irq_rx_disable(p);
}

void uart_irq_tx_enable(const struct uart_port *p)
{
	if (NULL == p->ops->irq_tx_enable)
		return;

	p->ops->irq_tx_enable(p);
}

void uart_irq_rx_enable(const struct uart_port *p)
{
	if (NULL == p->ops->irq_rx_enable)
		return;

	p->ops->irq_rx_enable(p);
}
#endif

#ifdef CONFIG_UART_ASYNC_API
int uart_callback_set(const struct uart_port *p, uart_async_cb_t cb, void *ud)
{
	if (NULL == p->ops->callback_set)
		return -ENOSYS;

	return p->ops->callback_set(p, cb, ud);
}

int uart_tx(const struct uart_port *p, const uint8_t *buf, uint32_t len)
{
	if (NULL == p->ops->tx)
		return -ENOSYS;

	return p->ops->tx(p, buf, len);
}

int uart_tx_abort(const struct uart_port *p)
{
	if (NULL == p->ops->tx_abort)
		return -ENOSYS;

	return p->ops->tx_abort(p);
}

int uart_rx_enable(const struct uart_port *p, uint8_t *buf, uint32_t len)
{
	if (NULL == p->ops->rx_enable)
		return -ENOSYS;

	return p->ops->rx_enable(p, buf, len);
}

int uart_rx_disable(const struct uart_port *p)
{
	if (NULL == p->ops->rx_disable)
		return -ENOSYS;

	return p->ops->rx_disable(p);
}
#endif
