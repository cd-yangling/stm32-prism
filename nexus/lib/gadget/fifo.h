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

#ifndef __NEXUS_LIB_GADGET_FIFO_HEADER__
#define __NEXUS_LIB_GADGET_FIFO_HEADER__

struct fifo
{
	volatile unsigned int in;	// 生产者推进
	volatile unsigned int out;	// 消费者推进
	unsigned int       mask;	// size-1(2 的幂减一)
	unsigned char     *data;	// 外部缓冲
};

int fifo_init(struct fifo *q, void *buf, unsigned int len);

unsigned int fifo_size(const struct fifo *q);

unsigned int fifo_used(const struct fifo *q);

unsigned int fifo_avail(const struct fifo *q);

int fifo_is_empty(const struct fifo *q);

int fifo_is_full(const struct fifo *q);

unsigned int fifo_enqueue(struct fifo *q, const void *src, unsigned int len);

unsigned int fifo_dequeue(struct fifo *q, void *dst, unsigned int len);

int fifo_putc(struct fifo *q, unsigned char c);

int fifo_getc(struct fifo *q);

#endif /* __NEXUS_LIB_GADGET_FIFO_HEADER__ */

