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

#ifndef	__NEXUS_IRQ_HEADER__
#define	__NEXUS_IRQ_HEADER__

#define	IRQ_PRIO_SYSTICK	15u

#ifndef	IRQ_PRIO_EXTI
#define	IRQ_PRIO_EXTI		0u
#endif

#ifndef	IRQ_PRIO_EXTI0
#define	IRQ_PRIO_EXTI0		IRQ_PRIO_EXTI
#endif

#ifndef	IRQ_PRIO_EXTI1
#define	IRQ_PRIO_EXTI1		IRQ_PRIO_EXTI
#endif

#ifndef	IRQ_PRIO_EXTI2
#define	IRQ_PRIO_EXTI2		IRQ_PRIO_EXTI
#endif

#ifndef	IRQ_PRIO_EXTI3
#define	IRQ_PRIO_EXTI3		IRQ_PRIO_EXTI
#endif

#ifndef	IRQ_PRIO_EXTI4
#define	IRQ_PRIO_EXTI4		IRQ_PRIO_EXTI
#endif

#ifndef	IRQ_PRIO_EXTI9_5
#define	IRQ_PRIO_EXTI9_5	IRQ_PRIO_EXTI
#endif

#ifndef	IRQ_PRIO_EXTI15_10
#define	IRQ_PRIO_EXTI15_10	IRQ_PRIO_EXTI
#endif

#ifndef	IRQ_PRIO_UART
#define	IRQ_PRIO_UART		0u
#endif

#ifndef	IRQ_PRIO_USART1
#define	IRQ_PRIO_USART1	IRQ_PRIO_UART
#endif

#ifndef	IRQ_PRIO_USART2
#define	IRQ_PRIO_USART2	IRQ_PRIO_UART
#endif

#ifndef	IRQ_PRIO_USART3
#define	IRQ_PRIO_USART3	IRQ_PRIO_UART
#endif

#ifndef	IRQ_PRIO_UART4
#define	IRQ_PRIO_UART4		IRQ_PRIO_UART
#endif

#ifndef	IRQ_PRIO_UART5
#define	IRQ_PRIO_UART5		IRQ_PRIO_UART
#endif

static inline unsigned int irq_lock(void)
{
	unsigned int key;

	__asm volatile ("mrs %0, primask\n\tcpsid i"
		: "=r" (key)
		:
		: "memory");
	return key;
}

static inline void irq_unlock(unsigned int key)
{
	__asm volatile ("msr primask, %0"
		:
		: "r" (key)
		: "memory");
}

#endif	//	__NEXUS_IRQ_HEADER__
