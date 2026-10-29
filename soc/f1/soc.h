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

#ifndef	__SOC_F1_INSTANCES_HEADER__
#define	__SOC_F1_INSTANCES_HEADER__

#if !defined(BOARD_C)
#error	"soc.h: board.c only; consumers use board.h"
#endif

struct i2c_bus;
struct spi_bus;
struct uart_port;
struct pin_port;
struct pwm_dev;
struct enc_dev;
struct adc_dev;

#if defined(CONFIG_I2C1)
extern const struct i2c_bus i2c1;
#endif
#if defined(CONFIG_I2C2)
extern const struct i2c_bus i2c2;
#endif
#if defined(CONFIG_SPI1)
extern const struct spi_bus spi1;
#endif
#if defined(CONFIG_SPI2)
extern const struct spi_bus spi2;
#endif
#if defined(CONFIG_SPI3)
extern const struct spi_bus spi3;
#endif
#if defined(CONFIG_USART1)
extern const struct uart_port usart1;
#endif
#if defined(CONFIG_USART2)
extern const struct uart_port usart2;
#endif
#if defined(CONFIG_USART3)
extern const struct uart_port usart3;
#endif
#if defined(CONFIG_UART4)
extern const struct uart_port uart4;
#endif
#if defined(CONFIG_UART5)
extern const struct uart_port uart5;
#endif

#if defined(CONFIG_PWM_TIM1)
extern const struct pwm_dev tim1;
#endif
#if defined(CONFIG_PWM_TIM2)
extern const struct pwm_dev tim2;
#endif
#if defined(CONFIG_PWM_TIM3)
extern const struct pwm_dev tim3;
#endif
#if defined(CONFIG_PWM_TIM4)
extern const struct pwm_dev tim4;
#endif
#if defined(CONFIG_PWM_TIM5)
extern const struct pwm_dev tim5;
#endif
#if defined(CONFIG_PWM_TIM8)
extern const struct pwm_dev tim8;
#endif

#if defined(CONFIG_ENC_TIM1)
extern const struct enc_dev tim1;
#endif
#if defined(CONFIG_ENC_TIM2)
extern const struct enc_dev tim2;
#endif
#if defined(CONFIG_ENC_TIM3)
extern const struct enc_dev tim3;
#endif
#if defined(CONFIG_ENC_TIM4)
extern const struct enc_dev tim4;
#endif
#if defined(CONFIG_ENC_TIM5)
extern const struct enc_dev tim5;
#endif
#if defined(CONFIG_ENC_TIM8)
extern const struct enc_dev tim8;
#endif

#if defined(CONFIG_ADC1)
extern const struct adc_dev adc1;
#endif
#if defined(CONFIG_ADC2)
extern const struct adc_dev adc2;
#endif
#if defined(CONFIG_ADC3)
extern const struct adc_dev adc3;
#endif

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
#if defined(GPIOE_PINS) && (0 != GPIOE_PINS)
#define	gpioe_PINS	GPIOE_PINS
extern const struct pin_port gpioe_port;
#endif

#endif	//	__SOC_F1_INSTANCES_HEADER__
