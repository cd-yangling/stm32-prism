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

#ifndef	__NEXUS_PAL_RESET_HEADER__
#define	__NEXUS_PAL_RESET_HEADER__

#include <stdint.h>

#define	RESET_CAUSE_PIN			(1u << 0)
#define	RESET_CAUSE_SOFTWARE	(1u << 1)
#define	RESET_CAUSE_POR			(1u << 2)
#define	RESET_CAUSE_IWDG		(1u << 3)
#define	RESET_CAUSE_WWDG		(1u << 4)
#define	RESET_CAUSE_LPWR		(1u << 5)
#define	RESET_CAUSE_BOR			(1u << 6)

void	reset_cause_consume(uint32_t *cause);

#endif	//	__NEXUS_PAL_RESET_HEADER__
