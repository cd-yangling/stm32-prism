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


#ifndef	__APPS_YAHBOOM_SBR_CONTROL_MOVE_DIST_HEADER__
#define	__APPS_YAHBOOM_SBR_CONTROL_MOVE_DIST_HEADER__

#include <stdint.h>

struct move_dist_config
{
	int16_t	kp_num;
	uint8_t	kp_shr;
	int16_t	out_max;
	uint16_t timeout_ms;
};

enum move_dist_st
{
	MOVE_DIST_IDLE,
	MOVE_DIST_RUN,
	MOVE_DIST_DONE,
	MOVE_DIST_ETIMEOUT,
	MOVE_DIST_ABORTED,
};

int move_dist_measuring_start(int32_t mm);
void move_dist_measuring_abort(void);
int move_dist_state_get(void);
int32_t move_dist_dist_get(void);
uint32_t move_dist_measuring_elapsed_ms(void);
void move_dist_poll(int16_t vsum, const struct move_dist_config *cfg);

#endif	//	__APPS_YAHBOOM_SBR_CONTROL_MOVE_DIST_HEADER__
