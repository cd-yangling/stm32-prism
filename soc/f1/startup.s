    .syntax unified
    .cpu cortex-m3
    .thumb

.macro WEAK name
    .weak \name
    .thumb_set \name, Default_Handler
.endm

    .section .isr_vector, "a", %progbits
    .align 2
    .global g_pfnVectors
g_pfnVectors:
    .word _estack
    .word Reset_Handler
    .word NMI_Handler
    .word HardFault_Handler
    .word MemManage_Handler
    .word BusFault_Handler
    .word UsageFault_Handler
    .word 0, 0, 0, 0
    .word SVC_Handler
    .word DebugMon_Handler
    .word 0
    .word PendSV_Handler
    .word SysTick_Handler
    .word WWDG_IRQHandler
    .word PVD_IRQHandler
    .word TAMPER_IRQHandler
    .word RTC_IRQHandler
    .word FLASH_IRQHandler
    .word RCC_IRQHandler
    .word EXTI0_IRQHandler
    .word EXTI1_IRQHandler
    .word EXTI2_IRQHandler
    .word EXTI3_IRQHandler
    .word EXTI4_IRQHandler
    .word DMA1_Channel1_IRQHandler
    .word DMA1_Channel2_IRQHandler
    .word DMA1_Channel3_IRQHandler
    .word DMA1_Channel4_IRQHandler
    .word DMA1_Channel5_IRQHandler
    .word DMA1_Channel6_IRQHandler
    .word DMA1_Channel7_IRQHandler
    .word ADC1_2_IRQHandler
    .word USB_HP_CAN1_TX_IRQHandler
    .word USB_LP_CAN1_RX0_IRQHandler
    .word CAN1_RX1_IRQHandler
    .word CAN1_SCE_IRQHandler
    .word EXTI9_5_IRQHandler
    .word TIM1_BRK_IRQHandler
    .word TIM1_UP_IRQHandler
    .word TIM1_TRG_COM_IRQHandler
    .word TIM1_CC_IRQHandler
    .word TIM2_IRQHandler
    .word TIM3_IRQHandler
    .word TIM4_IRQHandler
    .word I2C1_EV_IRQHandler
    .word I2C1_ER_IRQHandler
    .word I2C2_EV_IRQHandler
    .word I2C2_ER_IRQHandler
    .word SPI1_IRQHandler
    .word SPI2_IRQHandler
    .word USART1_IRQHandler
    .word USART2_IRQHandler
    .word USART3_IRQHandler
    .word EXTI15_10_IRQHandler
    .word RTC_Alarm_IRQHandler
    .word USBWakeUp_IRQHandler
    .word TIM8_BRK_IRQHandler
    .word TIM8_UP_IRQHandler
    .word TIM8_TRG_COM_IRQHandler
    .word TIM8_CC_IRQHandler
    .word ADC3_IRQHandler
    .word FSMC_IRQHandler
    .word SDIO_IRQHandler
    .word TIM5_IRQHandler
    .word SPI3_IRQHandler
    .word UART4_IRQHandler
    .word UART5_IRQHandler
    .word TIM6_IRQHandler
    .word TIM7_IRQHandler
    .word DMA2_Channel1_IRQHandler
    .word DMA2_Channel2_IRQHandler
    .word DMA2_Channel3_IRQHandler
    .word DMA2_Channel4_5_IRQHandler

    .section .text
    .thumb_func
    .global Reset_Handler
Reset_Handler:
# 搬 .data:初值在 Flash(_sidata),运行地址 _sdata.._edata
    ldr  r0, =_sidata
    ldr  r1, =_sdata
    ldr  r2, =_edata
1:
    cmp  r1, r2
    bcs  2f
    ldr  r3, [r0], #4
    str  r3, [r1], #4
    b    1b
# 清 .bss
2:
    ldr  r0, =__bss_start
    ldr  r1, =__bss_end
    movs r2, #0
3:
    cmp  r0, r1
    bcs  4f
    str  r2, [r0], #4
    b    3b
4:
# VTOR 指到本表
    ldr  r0, =_sisr
    ldr  r1, =0xE000ED08
    str  r0, [r1]
    bl   soc_init
    bl   brd_init
    bl   main
5:
    b    5b

    .thumb_func
Default_Handler:
    b    Default_Handler

# 内核异常 + 全部外设中断的弱别名
    WEAK NMI_Handler
    WEAK HardFault_Handler
    WEAK MemManage_Handler
    WEAK BusFault_Handler
    WEAK UsageFault_Handler
    WEAK SVC_Handler
    WEAK DebugMon_Handler
    WEAK PendSV_Handler
    WEAK SysTick_Handler
    WEAK WWDG_IRQHandler
    WEAK PVD_IRQHandler
    WEAK TAMPER_IRQHandler
    WEAK RTC_IRQHandler
    WEAK FLASH_IRQHandler
    WEAK RCC_IRQHandler
    WEAK EXTI0_IRQHandler
    WEAK EXTI1_IRQHandler
    WEAK EXTI2_IRQHandler
    WEAK EXTI3_IRQHandler
    WEAK EXTI4_IRQHandler
    WEAK DMA1_Channel1_IRQHandler
    WEAK DMA1_Channel2_IRQHandler
    WEAK DMA1_Channel3_IRQHandler
    WEAK DMA1_Channel4_IRQHandler
    WEAK DMA1_Channel5_IRQHandler
    WEAK DMA1_Channel6_IRQHandler
    WEAK DMA1_Channel7_IRQHandler
    WEAK ADC1_2_IRQHandler
    WEAK USB_HP_CAN1_TX_IRQHandler
    WEAK USB_LP_CAN1_RX0_IRQHandler
    WEAK CAN1_RX1_IRQHandler
    WEAK CAN1_SCE_IRQHandler
    WEAK EXTI9_5_IRQHandler
    WEAK TIM1_BRK_IRQHandler
    WEAK TIM1_UP_IRQHandler
    WEAK TIM1_TRG_COM_IRQHandler
    WEAK TIM1_CC_IRQHandler
    WEAK TIM2_IRQHandler
    WEAK TIM3_IRQHandler
    WEAK TIM4_IRQHandler
    WEAK I2C1_EV_IRQHandler
    WEAK I2C1_ER_IRQHandler
    WEAK I2C2_EV_IRQHandler
    WEAK I2C2_ER_IRQHandler
    WEAK SPI1_IRQHandler
    WEAK SPI2_IRQHandler
    WEAK USART1_IRQHandler
    WEAK USART2_IRQHandler
    WEAK USART3_IRQHandler
    WEAK EXTI15_10_IRQHandler
    WEAK RTC_Alarm_IRQHandler
    WEAK USBWakeUp_IRQHandler
    WEAK TIM8_BRK_IRQHandler
    WEAK TIM8_UP_IRQHandler
    WEAK TIM8_TRG_COM_IRQHandler
    WEAK TIM8_CC_IRQHandler
    WEAK ADC3_IRQHandler
    WEAK FSMC_IRQHandler
    WEAK SDIO_IRQHandler
    WEAK TIM5_IRQHandler
    WEAK SPI3_IRQHandler
    WEAK UART4_IRQHandler
    WEAK UART5_IRQHandler
    WEAK TIM6_IRQHandler
    WEAK TIM7_IRQHandler
    WEAK DMA2_Channel1_IRQHandler
    WEAK DMA2_Channel2_IRQHandler
    WEAK DMA2_Channel3_IRQHandler
    WEAK DMA2_Channel4_5_IRQHandler
