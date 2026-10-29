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

#ifndef	__APPS_YAHBOOM_SBR_TUNING_HEADER__
#define	__APPS_YAHBOOM_SBR_TUNING_HEADER__

#include <stdint.h>

struct balance_pd_config;
struct velocity_pi_config;
struct turn_pd_config;
struct tty;

#ifdef TUNE_LOCK
#define	TUNE_RO	const
#else
#define	TUNE_RO
#endif

extern TUNE_RO struct balance_pd_config tune_bal;
extern TUNE_RO struct velocity_pi_config tune_vel;
extern TUNE_RO struct turn_pd_config tune_turn;

void tuning_init(struct tty *t);
void tuning_poll(void);
uint8_t tune_run_enabled(void);
uint8_t tune_vsw_enabled(void);
void tune_feed(int16_t pitch_cd, int16_t out);

#endif	//	__APPS_YAHBOOM_SBR_TUNING_HEADER__
