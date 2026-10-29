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
#include "pal/i2c.h"
#include "pal/pin.h"
#include "pal/display.h"
#include "pal/uart.h"
#include "pal/led.h"
#include "pal/sensor.h"
#include "pal/reset.h"
#include "tick.h"
#include "dev/ssd1306.h"
#include "dev/mpu6050.h"
#include "subsys/tty.h"
#include "board.h"

#define	BOARD_C
#include "soc.h"

#if defined(CONFIG_LED)
PIN_DESC(led_1, gpioc, 13, PIN_OUT | PIN_SPEED_LOW | PIN_ACTIVE_LOW | PIN_INIT_INACTIVE);

static const struct led_dev led1= { .ops = &led_gpio_ops, .ctx = &led_1,   .name = "led1" };

static void brd_led_init(void)
{
	led_init(&led1);
}
#define	BOARD_LED1	(&led1)
#else
#define	BOARD_LED1	(NULL)
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

static void brd_usart_init(void)
{
	uart_dev_init(&con_dev, &usart1_pins);
}

static struct tty con_tty =
{
	.dev	= &con_dev,
	.mode	= TTY_POLL,
};
#define	BOARD_CONSOLE	(&con_tty)
#else
#define	BOARD_CONSOLE	(NULL)
#endif

#if defined(CONFIG_I2C1)
PIN_DESC(i2c1_scl, gpiob, 6, PIN_ALT | PIN_OD | PIN_SPEED_HIGH);
PIN_DESC(i2c1_sda, gpiob, 7, PIN_ALT | PIN_OD | PIN_SPEED_HIGH);
static const struct i2c_pins i2c1_pins =
{
	.scl	= &i2c1_scl,
	.sda	= &i2c1_sda,
};
#endif

#if defined(CONFIG_I2C2)
PIN_DESC(i2c2_scl, gpiob, 10, PIN_ALT | PIN_OD | PIN_SPEED_HIGH);
PIN_DESC(i2c2_sda, gpiob, 11, PIN_ALT | PIN_OD | PIN_SPEED_HIGH);
static const struct i2c_pins i2c2_pins =
{
	.scl	= &i2c2_scl,
	.sda	= &i2c2_sda,
};
#endif

static void brd_i2c_init(void)
{
#if defined(CONFIG_I2C1)
	i2c_bus_init(&i2c1, &i2c1_pins);
#endif
#if defined(CONFIG_I2C2)
	i2c_bus_init(&i2c2, &i2c2_pins);
#endif
}

#if defined(CONFIG_MPU6050)
static const struct i2c_dev mpu1 =
{
	.bus	= &i2c1,
	.addr7	= 0x68,
	.spd_hz	= 400 * 1000,
	.name	= "mpu6050",
};
MPU6050_DAT(mpu1_dat);
static const struct mpu6050_dev mpu6050_dev =
{
	.i2c		= &mpu1,
	.int_pin	= NULL,
	.smplrt_div	= 9,
	.dlpf_cfg	= 3,
	.gfs_sel	= 0,
	.afs_sel	= 0,
	.dat		= mpu1_dat,
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

#if defined(CONFIG_SSD1306)
static const struct i2c_dev oled1 =
{
	.bus = &i2c1,
	.addr7 = 0x3C,
	.spd_hz = 400 * 1000,
	.name = "ssd1306",
};

static const struct display_caps oled_caps =
{
	.x_res = 128,
	.y_res = 64,
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

const struct board board =
{
	.display	= BOARD_DISPLAY,
	.console	= BOARD_CONSOLE,
	.imu1		= BOARD_IMU1,
	.led1		= BOARD_LED1,
};

void brd_init(void)
{
	uint32_t cause;

	reset_cause_consume(&cause);
	if (RESET_CAUSE_POR & cause)
	{
		delay_ms(200u);
	}
#if defined(CONFIG_LED)
	brd_led_init();
#endif
#if defined(CONFIG_I2C1) || defined(CONFIG_I2C2)
	brd_i2c_init();
#endif
#if defined(CONFIG_USART1)
	brd_usart_init();
#endif
}
