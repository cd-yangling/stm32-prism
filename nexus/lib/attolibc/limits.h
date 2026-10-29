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

#ifndef	__NEXUS_LIB_ATTOLIBC_LIMITS_HEADER__
#define	__NEXUS_LIB_ATTOLIBC_LIMITS_HEADER__

#define	CHAR_BIT	8

#define	SCHAR_MIN	(-__SCHAR_MAX__ - 1)
#define	SCHAR_MAX	__SCHAR_MAX__
#define	UCHAR_MAX	(__SCHAR_MAX__ * 2u + 1u)

#if defined(__CHAR_UNSIGNED__)
#define	CHAR_MIN	0
#define	CHAR_MAX	UCHAR_MAX
#else
#define	CHAR_MIN	SCHAR_MIN
#define	CHAR_MAX	SCHAR_MAX
#endif

#define	SHRT_MIN	(-__SHRT_MAX__ - 1)
#define	SHRT_MAX	__SHRT_MAX__
#define	USHRT_MAX	(__SHRT_MAX__ * 2u + 1u)

#define	INT_MIN		(-__INT_MAX__ - 1)
#define	INT_MAX		__INT_MAX__
#define	UINT_MAX	(__INT_MAX__ * 2u + 1u)

#define	LONG_MIN	(-__LONG_MAX__ - 1L)
#define	LONG_MAX	__LONG_MAX__
#define	ULONG_MAX	(__LONG_MAX__ * 2UL + 1UL)

#define	LLONG_MIN	(-__LONG_LONG_MAX__ - 1LL)
#define	LLONG_MAX	__LONG_LONG_MAX__
#define	ULLONG_MAX	(__LONG_LONG_MAX__ * 2ULL + 1ULL)

#endif	//	__NEXUS_LIB_ATTOLIBC_LIMITS_HEADER__
