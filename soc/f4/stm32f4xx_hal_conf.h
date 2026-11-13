#ifndef __STM32F4xx_HAL_CONF_H
#define __STM32F4xx_HAL_CONF_H
#include "irq.h"
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED

#if !defined (HSE_VALUE)
#define HSE_VALUE               8000000UL
#endif
#if !defined (HSE_STARTUP_TIMEOUT)
#define HSE_STARTUP_TIMEOUT     100UL
#endif
#if !defined (HSI_VALUE)
#define HSI_VALUE               16000000UL
#endif
#if !defined (LSI_VALUE)
#define LSI_VALUE               32000UL
#endif
#if !defined (LSE_VALUE)
#define LSE_VALUE               32768UL
#endif
#if !defined (LSE_STARTUP_TIMEOUT)
#define LSE_STARTUP_TIMEOUT     5000UL
#endif
#if !defined (EXTERNAL_CLOCK_VALUE)
#define EXTERNAL_CLOCK_VALUE    12288000UL
#endif

#define VDD_VALUE               3300UL
#define TICK_INT_PRIORITY       IRQ_PRIO_SYSTICK
#define USE_RTOS                0U

#define assert_param(expr)      ((void)0U)

#ifdef HAL_RCC_MODULE_ENABLED
#include "stm32f4xx_hal_rcc.h"
#endif
#ifdef HAL_GPIO_MODULE_ENABLED
#include "stm32f4xx_hal_gpio.h"
#endif
#ifdef HAL_DMA_MODULE_ENABLED
#include "stm32f4xx_hal_dma.h"
#endif
#ifdef HAL_FLASH_MODULE_ENABLED
#include "stm32f4xx_hal_flash.h"
#endif
#ifdef HAL_CORTEX_MODULE_ENABLED
#include "stm32f4xx_hal_cortex.h"
#endif

#ifdef __cplusplus
}
#endif
#endif
