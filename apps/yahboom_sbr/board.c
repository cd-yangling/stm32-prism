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

#include <stdint.h>
#include <stddef.h>
#include "pal/pin.h"
#include "pal/uart.h"
#include "pal/i2c.h"
#include "pal/i2c_bitbang.h"
#include "pal/led.h"
#include "pal/display.h"
#include "pal/sensor.h"
#include "pal/reset.h"
#include "pal/adc.h"
#include "pal/pwm.h"
#include "pal/button.h"
#include "pal/enc.h"
#include "subsys/input.h"
#include "subsys/tty.h"
#include "tick.h"
#include "dev/at8236.h"
#include "motor.h"
#include "dev/ssd1306.h"
#include "dev/mpu6050.h"
#include "dev/voltage_divider.h"
#include "board.h"

#define	BOARD_C
#include "soc.h"

#if defined(CONFIG_LED)
PIN_DESC(led_1, gpiob, 3, PIN_OUT | PIN_SPEED_LOW | PIN_INIT_INACTIVE);

static const struct led_dev led1= { .ops = &led_gpio_ops, .ctx = &led_1,   .name = "led1" };
#define	BOARD_LED1	(&led1)
#else
#define	BOARD_LED1	(NULL)
#endif

static void brd_led_init(void)
{
#if defined(CONFIG_LED)
	led_init(&led1);
#endif
}

#if defined(CONFIG_BUZZER)
PIN_DESC(buzzer, gpioa, 11, PIN_OUT | PIN_SPEED_LOW | PIN_INIT_INACTIVE);
#define	BOARD_BUZZER	(&buzzer)
#else
#define	BOARD_BUZZER	(NULL)
#endif

#if defined(CONFIG_SONAR)
PIN_DESC(sonar_trig, gpioa, 0, PIN_OUT | PIN_SPEED_LOW | PIN_INIT_INACTIVE);
PIN_DESC(sonar_echo, gpioa, 1, PIN_IN);
#define	BOARD_SONAR_TRIG	(&sonar_trig)
#define	BOARD_SONAR_ECHO	(&sonar_echo)
#else
#define	BOARD_SONAR_TRIG	(NULL)
#define	BOARD_SONAR_ECHO	(NULL)
#endif

#if defined(CONFIG_KEY)
PIN_DESC(key1, gpioa, 8, PIN_IN | PIN_PULL_UP);
BUTTON_DAT(key1_dat);

static const struct button_dev key1_dev =
	{ &key1, 20, INPUT_KEY_1, key1_dat, "key1" };
#define	BOARD_KEY1	(&key1_dev)
#else
#define	BOARD_KEY1	(NULL)
#endif

#if defined(CONFIG_USART1)
PIN_DESC(usart1_tx, gpioa, 9,  PIN_ALT | PIN_SPEED_HIGH);
PIN_DESC(usart1_rx, gpioa, 10, PIN_IN);
static const struct uart_pins usart1_pins =
{
	.tx	= &usart1_tx,
	.rx	= &usart1_rx,
};
static const struct uart_config con_cfg =
{
	.baud = 1152000,
	.data_bits = 8,
	.parity = 0,
	.stop_bits = 1,
};
static const struct uart_dev con_dev =
{
	.port	= &usart1,
	.cfg	= &con_cfg,
};

#if defined(CONFIG_UART_INTERRUPT_DRIVEN) || defined(CONFIG_UART_ASYNC_API)
static unsigned char con_rx_buf[256];
static unsigned char con_tx_buf[256];
#endif
#ifdef CONFIG_UART_ASYNC_API
static unsigned char con_dma_tx_buf[96];
static unsigned char con_dma_rx_buf[64];
#endif

static struct tty con_tty =
{
	.dev		= &con_dev,
#if defined(CONFIG_UART_ASYNC_API)
	.mode		= TTY_ASYNC,
#elif defined(CONFIG_UART_INTERRUPT_DRIVEN)
	.mode		= TTY_IRQ,
#else
	.mode		= TTY_POLL,
#endif
#if defined(CONFIG_UART_INTERRUPT_DRIVEN) || defined(CONFIG_UART_ASYNC_API)
	.rx_buf		= con_rx_buf,
	.rx_len		= sizeof(con_rx_buf),
	.tx_buf		= con_tx_buf,
	.tx_len		= sizeof(con_tx_buf),
#endif
#ifdef CONFIG_UART_ASYNC_API
	.dma_tx_buf	= con_dma_tx_buf,
	.dma_tx_len	= sizeof(con_dma_tx_buf),
	.dma_rx_buf	= con_dma_rx_buf,
	.dma_rx_len	= sizeof(con_dma_rx_buf),
#endif
};
#define	BOARD_CONSOLE	(&con_tty)
#else
#define	BOARD_CONSOLE	(NULL)
#endif

#if defined(CONFIG_UART5)
PIN_DESC(uart5_tx, gpioc, 12, PIN_ALT | PIN_SPEED_HIGH);
PIN_DESC(uart5_rx, gpiod, 2, PIN_IN);
static const struct uart_pins uart5_pins =
{
	.tx	= &uart5_tx,
	.rx	= &uart5_rx,
};
static const struct uart_config bt_cfg =
{
	.baud = 9600,
	.data_bits = 8,
	.parity = 0,
	.stop_bits = 1,
};
static const struct uart_dev bt_dev =
{
	.port	= &uart5,
	.cfg	= &bt_cfg,
};

#if defined(CONFIG_UART_INTERRUPT_DRIVEN) || defined(CONFIG_UART_ASYNC_API)
static unsigned char bt_rx_buf[64];
static unsigned char bt_tx_buf[512];
#endif

static struct tty bt_tty =
{
	.dev	= &bt_dev,
#if defined(CONFIG_UART_ASYNC_API) && !defined(CONFIG_UART_INTERRUPT_DRIVEN)
	#error "uart5 has no DMA support; pure async (DMA-only) mode requires CONFIG_UART_INTERRUPT_DRIVEN."
#elif defined(CONFIG_UART_INTERRUPT_DRIVEN)
	.mode	= TTY_IRQ,
#else
	.mode	= TTY_POLL,
#endif
#if defined(CONFIG_UART_INTERRUPT_DRIVEN) || defined(CONFIG_UART_ASYNC_API)
	.rx_buf	= bt_rx_buf,
	.rx_len	= sizeof(bt_rx_buf),
	.tx_buf	= bt_tx_buf,
	.tx_len	= sizeof(bt_tx_buf),
#endif
};
#define	BOARD_BT	(&bt_tty)
#else
#define	BOARD_BT	(NULL)
#endif

static void brd_uart_init(void)
{
#if defined(CONFIG_USART1)
	uart_dev_init(&con_dev, &usart1_pins);
#endif
#if defined(CONFIG_UART5)
	uart_dev_init(&bt_dev, &uart5_pins);
#endif
}

#if defined(CONFIG_I2C1)
PIN_DESC(i2c1_scl, gpiob, 8, PIN_ALT | PIN_OD | PIN_SPEED_HIGH | PIN_MUX(1));
PIN_DESC(i2c1_sda, gpiob, 9, PIN_ALT | PIN_OD | PIN_SPEED_HIGH | PIN_MUX(1));
static const struct i2c_pins i2c1_pins =
{
	.scl	= &i2c1_scl,
	.sda	= &i2c1_sda,
};
#define	OLED_I2C_BUS	(&i2c1)
#endif

#if defined(CONFIG_I2C_BITBANG)
PIN_DESC(i2c_bb1_scl, gpiob, 11, PIN_OUT | PIN_OD | PIN_SPEED_HIGH);
PIN_DESC(i2c_bb1_sda, gpiob, 10, PIN_OUT | PIN_OD | PIN_SPEED_HIGH);
static const struct i2c_pins i2c_bb1_pins =
{
	.scl	= &i2c_bb1_scl,
	.sda	= &i2c_bb1_sda,
};
I2C_BITBANG_BUS(i2c_bb1_bus, &i2c_bb1_pins);
#endif

static void brd_i2c_init(void)
{
#if defined(CONFIG_I2C1)
	i2c_bus_init(&i2c1, &i2c1_pins);
#endif
#if defined(CONFIG_I2C_BITBANG)
	i2c_bus_init(&i2c_bb1_bus, &i2c_bb1_pins);
#endif
}

#if defined(CONFIG_SSD1306)
static const struct i2c_dev oled1 =
{
	.bus = OLED_I2C_BUS,
	.addr7 = 0x3C,
	.spd_hz = 400 * 1000,
	.name = "ssd1306",
};

static const struct display_caps oled_caps =
{
	.x_res = 128,
	.y_res = 32,
	.fmt = DISPLAY_FMT_MONO_VTILED,
};

static const struct display disp_dev =
{
	.ops = &ssd1306_ops,
	.ctx = &oled1,
	.caps = &oled_caps,
	.name = "oled",
};
#define	BOARD_DISPLAY	(&disp_dev)
#else
#define	BOARD_DISPLAY	(NULL)
#endif

#if defined(CONFIG_MPU6050)
#if defined(CONFIG_MPU6050_INT)
PIN_DESC(mpu_int, gpioa, 12, PIN_IN);
#endif
static const struct i2c_dev mpu1 =
{
	.bus	= &i2c_bb1_bus,
	.addr7	= 0x68,
	.spd_hz	= 400 * 1000,
	.name	= "mpu6050",
};
MPU6050_DAT(mpu1_dat);
static const struct mpu6050_axis_map mpu1_axis_map = MPU6050_AXIS_MAP_XRIGHT_YFRONT_ZUP_INIT;
static const struct mpu6050_dev mpu6050_dev =
{
	.i2c		= &mpu1,
#if defined(CONFIG_MPU6050_INT)
	.int_pin	= &mpu_int,
#else
	.int_pin	= NULL,
#endif
	.axis_map	= &mpu1_axis_map,
#define MPU6050_CFG_SET	3
#if MPU6050_CFG_SET == 1
	.smplrt_div	= 15,	//	500Hz 2ms 周期解算
	.dlpf_cfg	= 0,	//	DLPF 不使用
	.gfs_sel	= 1,	//	500dps
	.afs_sel	= 1,	//	+/- 4g
#elif MPU6050_CFG_SET == 2
	.smplrt_div	= 1,	//	500Hz 2ms 周期解算
	.dlpf_cfg	= 1,	//	Accel 2ms Gryo 2ms
	.gfs_sel	= 1,	//	500dps
	.afs_sel	= 1,	//	+/- 4g
#elif MPU6050_CFG_SET == 3
	.smplrt_div	= 1,	//	500Hz 2ms 周期解算
	.dlpf_cfg	= 2,	//	Accel 3.0ms Gyro 2.8ms
	.gfs_sel	= 1,	//	500dps
	.afs_sel	= 1,	//	+/- 4g
#else
	#error "Invalid MPU6050_CFG_SET, choose 1-3"
#endif
	.dat	= mpu1_dat,
};
static const struct sensor imu_dev =
{
	.ops	= &mpu6050_sensor_ops,
	.ctx	= &mpu6050_dev,
	.name	= "mpu6050",
};
#define	BOARD_IMU1	(&imu_dev)
#else
#define	BOARD_IMU1	(NULL)
#endif

#if defined(CONFIG_VOLTAGE_DIVIDER)
VOLTAGE_DIVIDER_DAT(vdiv_dat);
static const struct voltage_divider_dev vdiv_dev =
{
	.adc			= &adc1,
	.ch				= 5u,
	.full_ohms		= 13300u,
	.output_ohms	= 3300u,
	.dat			= vdiv_dat,
};
static const struct sensor vbat_sen =
{
	.ops	= &voltage_divider_sensor_ops,
	.ctx	= &vdiv_dev,
	.name	= "vbat_sen",
};
#define	BOARD_VBAT_SEN	(&vbat_sen)
#else
#define	BOARD_VBAT_SEN	(NULL)
#endif

static void brd_adc_init(void)
{
#if defined(CONFIG_ADC1)
	adc_init(&adc1);
#endif
}

#if defined(CONFIG_ENC_TIM3) && defined(CONFIG_ENC_TIM4)
PIN_DESC(enc_l_a, gpioa, 6, PIN_IN);
PIN_DESC(enc_l_b, gpioa, 7, PIN_IN);
PIN_DESC(enc_r_a, gpiob, 6, PIN_IN);
PIN_DESC(enc_r_b, gpiob, 7, PIN_IN);

static const struct enc_ch enc_l =
{
	.dev	= &tim3,
	.pin_a	= &enc_l_a,
	.pin_b	= &enc_l_b,
	.mode	= ENC_MODE_X4,
	.filter	= 0,
	.ckd	= ENC_CKD_DIV1,
};

static const struct enc_ch enc_r =
{
	.dev	= &tim4,
	.pin_a	= &enc_r_a,
	.pin_b	= &enc_r_b,
	.mode	= ENC_MODE_X4,
	.filter	= 0,
	.ckd	= ENC_CKD_DIV1,
};
#define	BOARD_ENC_L	(&enc_l)
#define	BOARD_ENC_R	(&enc_r)
#else
#define	BOARD_ENC_L	(NULL)
#define	BOARD_ENC_R	(NULL)
#endif

#if defined(CONFIG_AT8236)
PIN_DESC(at8236_l_in1, gpioc, 6, PIN_ALT | PIN_SPEED_HIGH);
PIN_DESC(at8236_l_in2, gpioc, 7, PIN_ALT | PIN_SPEED_HIGH);
PIN_DESC(at8236_r_in1, gpioc, 8, PIN_ALT | PIN_SPEED_HIGH);
PIN_DESC(at8236_r_in2, gpioc, 9, PIN_ALT | PIN_SPEED_HIGH);

static const struct pwm_ch at8236_l_in1_ch =
{
	.dev	= &tim8,
	.ch		= 1u,
	.pin	= &at8236_l_in1,
	.flags	= PWM_POLARITY_NORMAL,
};

static const struct pwm_ch at8236_l_in2_ch =
{
	.dev	= &tim8,
	.ch		= 2u,
	.pin	= &at8236_l_in2,
	.flags	= PWM_POLARITY_NORMAL,
};

static const struct pwm_ch at8236_r_in1_ch =
{
	.dev	= &tim8,
	.ch		= 3u,
	.pin	= &at8236_r_in1,
	.flags	= PWM_POLARITY_NORMAL,
};

static const struct pwm_ch at8236_r_in2_ch =
{
	.dev	= &tim8,
	.ch		= 4u,
	.pin	= &at8236_r_in2,
	.flags	= PWM_POLARITY_NORMAL,
};

static const struct at8236_dev at8236_l =
{
	.in1		= &at8236_l_in1_ch,
	.in2		= &at8236_l_in2_ch,
	.period_ns	= PWM_PERIOD_NS(25000u),
	.flags		= AT8236_DECAY_FAST,
};

static const struct at8236_dev at8236_r =
{
	.in1		= &at8236_r_in1_ch,
	.in2		= &at8236_r_in2_ch,
	.period_ns	= PWM_PERIOD_NS(25000u),
	.flags		= AT8236_DECAY_FAST,
};
#define	BOARD_AT8236_L	(&at8236_l)
#define	BOARD_AT8236_R	(&at8236_r)

static const struct motor_desc motor_l =
{
	.h			= &at8236_l,
	.dir		= -1,
	.deadband	= 530,
};

static const struct motor_desc motor_r =
{
	.h			= &at8236_r,
	.dir		= 1,
	.deadband	= 530,
};
#define	BOARD_MOTOR_L	(&motor_l)
#define	BOARD_MOTOR_R	(&motor_r)
#else
#define	BOARD_AT8236_L	(NULL)
#define	BOARD_AT8236_R	(NULL)
#define	BOARD_MOTOR_L	(NULL)
#define	BOARD_MOTOR_R	(NULL)
#endif

const struct board board =
{
	.display	= BOARD_DISPLAY,
	.console	= BOARD_CONSOLE,
	.imu1		= BOARD_IMU1,
	.led1		= BOARD_LED1,
	.vbat_sen	= BOARD_VBAT_SEN,
	.at8236_l	= BOARD_AT8236_L,
	.at8236_r	= BOARD_AT8236_R,
	.motor_l	= BOARD_MOTOR_L,
	.motor_r	= BOARD_MOTOR_R,
	.buzzer		= BOARD_BUZZER,
	.sonar_trig	= BOARD_SONAR_TRIG,
	.sonar_echo	= BOARD_SONAR_ECHO,
	.bt		= BOARD_BT,
	.key1		= BOARD_KEY1,
	.enc_l		= BOARD_ENC_L,
	.enc_r		= BOARD_ENC_R,
};

void brd_init(void)
{
	uint32_t cause;

	reset_cause_consume(&cause);
	if (RESET_CAUSE_POR & cause)
	{
		delay_ms(200u);
	}
	brd_led_init();
	brd_i2c_init();
	brd_uart_init();
	brd_adc_init();
}
