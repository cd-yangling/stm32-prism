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

#ifndef	__NEXUS_LIB_ATTOLIBC_STDINT_HEADER__
#define	__NEXUS_LIB_ATTOLIBC_STDINT_HEADER__

/* ---- 精确宽度 ---- */
typedef	__INT8_TYPE__		int8_t;
typedef	__INT16_TYPE__		int16_t;
typedef	__INT32_TYPE__		int32_t;
typedef	__INT64_TYPE__		int64_t;
typedef	__UINT8_TYPE__		uint8_t;
typedef	__UINT16_TYPE__	uint16_t;
typedef	__UINT32_TYPE__	uint32_t;
typedef	__UINT64_TYPE__	uint64_t;

/* ---- 最小宽度 ---- */
typedef	__INT_LEAST8_TYPE__	int_least8_t;
typedef	__INT_LEAST16_TYPE__	int_least16_t;
typedef	__INT_LEAST32_TYPE__	int_least32_t;
typedef	__INT_LEAST64_TYPE__	int_least64_t;
typedef	__UINT_LEAST8_TYPE__	uint_least8_t;
typedef	__UINT_LEAST16_TYPE__	uint_least16_t;
typedef	__UINT_LEAST32_TYPE__	uint_least32_t;
typedef	__UINT_LEAST64_TYPE__	uint_least64_t;

/* ---- 最快宽度 ---- */
typedef	__INT_FAST8_TYPE__		int_fast8_t;
typedef	__INT_FAST16_TYPE__		int_fast16_t;
typedef	__INT_FAST32_TYPE__		int_fast32_t;
typedef	__INT_FAST64_TYPE__		int_fast64_t;
typedef	__UINT_FAST8_TYPE__		uint_fast8_t;
typedef	__UINT_FAST16_TYPE__	uint_fast16_t;
typedef	__UINT_FAST32_TYPE__	uint_fast32_t;
typedef	__UINT_FAST64_TYPE__	uint_fast64_t;

/* ---- 指针与最大 ---- */
typedef	__INTPTR_TYPE__		intptr_t;
typedef	__UINTPTR_TYPE__	uintptr_t;
typedef	__INTMAX_TYPE__		intmax_t;
typedef	__UINTMAX_TYPE__	uintmax_t;

/* ---- 精确宽度极值 ---- */
#define	INT8_MIN	(-__INT8_MAX__ - 1)
#define	INT8_MAX	__INT8_MAX__
#define	INT16_MIN	(-__INT16_MAX__ - 1)
#define	INT16_MAX	__INT16_MAX__
#define	INT32_MIN	(-__INT32_MAX__ - 1)
#define	INT32_MAX	__INT32_MAX__
#define	INT64_MIN	(-__INT64_MAX__ - 1)
#define	INT64_MAX	__INT64_MAX__
#define	UINT8_MAX	(__INT8_MAX__ * 2u + 1u)
#define	UINT16_MAX	(__INT16_MAX__ * 2u + 1u)
#define	UINT32_MAX	(__INT32_MAX__ * 2u + 1u)
#define	UINT64_MAX	(__INT64_MAX__ * 2ULL + 1ULL)

/* ---- 最小宽度极值 ---- */
#define	INT_LEAST8_MIN	(-__INT_LEAST8_MAX__ - 1)
#define	INT_LEAST8_MAX	__INT_LEAST8_MAX__
#define	INT_LEAST16_MIN	(-__INT_LEAST16_MAX__ - 1)
#define	INT_LEAST16_MAX	__INT_LEAST16_MAX__
#define	INT_LEAST32_MIN	(-__INT_LEAST32_MAX__ - 1)
#define	INT_LEAST32_MAX	__INT_LEAST32_MAX__
#define	INT_LEAST64_MIN	(-__INT_LEAST64_MAX__ - 1)
#define	INT_LEAST64_MAX	__INT_LEAST64_MAX__
#define	UINT_LEAST8_MAX	(__INT_LEAST8_MAX__ * 2u + 1u)
#define	UINT_LEAST16_MAX	(__INT_LEAST16_MAX__ * 2u + 1u)
#define	UINT_LEAST32_MAX	(__INT_LEAST32_MAX__ * 2u + 1u)
#define	UINT_LEAST64_MAX	(__INT_LEAST64_MAX__ * 2ULL + 1ULL)

/* ---- 最快宽度极值 ---- */
#define	INT_FAST8_MIN	(-__INT_FAST8_MAX__ - 1)
#define	INT_FAST8_MAX	__INT_FAST8_MAX__
#define	INT_FAST16_MIN	(-__INT_FAST16_MAX__ - 1)
#define	INT_FAST16_MAX	__INT_FAST16_MAX__
#define	INT_FAST32_MIN	(-__INT_FAST32_MAX__ - 1)
#define	INT_FAST32_MAX	__INT_FAST32_MAX__
#define	INT_FAST64_MIN	(-__INT_FAST64_MAX__ - 1)
#define	INT_FAST64_MAX	__INT_FAST64_MAX__
#define	UINT_FAST8_MAX	(__INT_FAST8_MAX__ * 2u + 1u)
#define	UINT_FAST16_MAX	(__INT_FAST16_MAX__ * 2ULL + 1ULL)
#define	UINT_FAST32_MAX	(__INT_FAST32_MAX__ * 2ULL + 1ULL)
#define	UINT_FAST64_MAX	(__INT_FAST64_MAX__ * 2ULL + 1ULL)

/* ---- 指针/最大/杂项极值 ---- */
#define	INTPTR_MIN		(-__INTPTR_MAX__ - 1)
#define	INTPTR_MAX		__INTPTR_MAX__
#define	UINTPTR_MAX		(__INTPTR_MAX__ * 2ULL + 1ULL)
#define	INTMAX_MIN		(-__INTMAX_MAX__ - 1)
#define	INTMAX_MAX		__INTMAX_MAX__
#define	UINTMAX_MAX		(__INTMAX_MAX__ * 2ULL + 1ULL)

#define	PTRDIFF_MIN		(-__PTRDIFF_MAX__ - 1)
#define	PTRDIFF_MAX		__PTRDIFF_MAX__
#define	SIG_ATOMIC_MIN	(-__SIG_ATOMIC_MAX__ - 1)
#define	SIG_ATOMIC_MAX	__SIG_ATOMIC_MAX__
#define	SIZE_MAX		__SIZE_MAX__

#if defined(__WCHAR_UNSIGNED__)
#define	WCHAR_MIN	0u
#else
#define	WCHAR_MIN	(-__WCHAR_MAX__ - 1)
#endif
#define	WCHAR_MAX	__WCHAR_MAX__

#define	INT8_C(c)	c
#define	INT16_C(c)	c
#define	INT32_C(c)	c
#define	INT64_C(c)	c ## LL
#define	UINT8_C(c)	c
#define	UINT16_C(c)	c
#define	UINT32_C(c)	c ## U
#define	UINT64_C(c)	c ## ULL
#define	INTMAX_C(c)	c ## LL
#define	UINTMAX_C(c)	c ## ULL

#endif	//	__NEXUS_LIB_ATTOLIBC_STDINT_HEADER__
