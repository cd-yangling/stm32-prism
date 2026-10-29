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

#include "pal/pin.h"

void pin_configure(const struct pin_desc *d)
{
	d->port->ops->configure(d->port, d->pin, d->flags);
}

int pin_irq_configure(const struct pin_desc *d, uint32_t flags)
{
	return d->port->ops->irq_configure(d->port, d->pin, flags);
}

int pin_add_callback(const struct pin_port *pt, struct pin_callback *cb)
{
	return pt->ops->manage_cb(pt, cb, 1);
}

int pin_remove_callback(const struct pin_port *pt, struct pin_callback *cb)
{
	return pt->ops->manage_cb(pt, cb, 0);
}
