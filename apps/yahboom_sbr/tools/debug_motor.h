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

#ifndef	__APPS_YAHBOOM_SBR_TOOLS_DEBUG_MOTOR_HEADER__
#define	__APPS_YAHBOOM_SBR_TOOLS_DEBUG_MOTOR_HEADER__

//	电机基本测试
void debug_motor_basic_test(void);

// 1/2: 方向标定测试(标定完毕,写board.c的配置)
void debug_motor_calibrate_dir(void);

// 3:死区标定,轮始动瞬间记下(标定完毕,写board.c的配置)
void debug_motor_calibrate_deadband(void);

//	4:满占空极速测试(危险: key1 按下-松开放行后双轮满转 3s, 100ms 一拍, 自动刹停)
void debug_motor_full_speed(void);

#endif	//	__APPS_YAHBOOM_SBR_TOOLS_DEBUG_MOTOR_HEADER__

