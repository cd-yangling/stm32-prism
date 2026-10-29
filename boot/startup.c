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

/* startup.c -- 极简跳板(只是为了磨损平衡flash):读头两个字(SP/PC),如果合法就跳,
 * 否则 WFI 挂死.什么都不初始化.FLASH_SIZE/SRAM_SIZE/APP_OFFSET 由 Makefile 传. */
#include <stdint.h>

#ifndef APP_OFFSET
#error "APP_OFFSET undefined (boot/Makefile must pass it from offset.mk)"
#endif

#define FLASH_BASE 0x08000000UL
#define APP_BASE    (FLASH_BASE + APP_OFFSET)
#define SRAM_BASE   0x20000000UL

#define SCB_VTOR (*(volatile uint32_t *)0xE000ED08UL)

extern uint32_t _estack;

void reset_handler(void);

void default_handler(void)
{
    while (1) {
    }
}

/* 内核 16 项向量表 0x08000000 */
__attribute__((section(".isr_vector"), used))
const uint32_t vector_table[] = {
    (uint32_t)(uintptr_t)&_estack,
    (uint32_t)(uintptr_t)reset_handler,
    [2 ... 15] = (uint32_t)(uintptr_t)default_handler,
};

void reset_handler(void)
{
    const volatile uint32_t *app_vt = (const volatile uint32_t *)APP_BASE;
    uint32_t app_sp = app_vt[0];
    uint32_t app_pc = app_vt[1];

    /* 1: SP 落片内 2: RAM;PC 落~Flash 末;thumb 入口(bit0=1) */
    if ((app_sp >= SRAM_BASE) && (app_sp <= SRAM_BASE + SRAM_SIZE)
        && (app_pc >= APP_BASE) && (app_pc < FLASH_BASE + FLASH_SIZE)
        && ((app_pc & 3UL) == 1UL))
    {
        __asm volatile (
            "cpsid i                   \n\t"
            "str  %[vtor], [%[vreg]]   \n\t"
            "msr  msp, %[sp]           \n\t"
            "dsb                       \n\t"
            "isb                       \n\t"
            "cpsie i                   \n\t"
            "bx   %[entry]             \n\t"
            :
            : [vtor] "r" (APP_BASE),
              [vreg] "r" (&SCB_VTOR),
              [sp] "r" (app_sp),
              [entry] "r" (app_pc)
            : "memory");
        __builtin_unreachable();
    }

#if defined(CHIP_SERIES_F1)
    *(volatile uint32_t *)0xE0042004UL |= 1UL;
#elif defined(CHIP_SERIES_H7)
    *(volatile uint32_t *)0x5C001004UL |= 1UL;
#endif
    while (1)
	{
        __asm volatile ("wfi");
    }
}
