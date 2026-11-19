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

#ifndef	__SOC_F4_INSTANCES_HEADER__
#define	__SOC_F4_INSTANCES_HEADER__

#if !defined(BOARD_C)
#error	"soc.h: board.c only; consumers use board.h"
#endif

struct pin_port;

#if defined(GPIOA_PINS) && (0 != GPIOA_PINS)
#define	gpioa_PINS	GPIOA_PINS
extern const struct pin_port gpioa_port;
#endif
#if defined(GPIOB_PINS) && (0 != GPIOB_PINS)
#define	gpiob_PINS	GPIOB_PINS
extern const struct pin_port gpiob_port;
#endif
#if defined(GPIOC_PINS) && (0 != GPIOC_PINS)
#define	gpioc_PINS	GPIOC_PINS
extern const struct pin_port gpioc_port;
#endif
#if defined(GPIOD_PINS) && (0 != GPIOD_PINS)
#define	gpiod_PINS	GPIOD_PINS
extern const struct pin_port gpiod_port;
#endif
#if defined(GPIOH_PINS) && (0 != GPIOH_PINS)
#define	gpioh_PINS	GPIOH_PINS
extern const struct pin_port gpioh_port;
#endif

#endif	//	__SOC_F4_INSTANCES_HEADER__
