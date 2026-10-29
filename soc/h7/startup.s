    .syntax unified
    .cpu cortex-m7
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
    .word PVD_AVD_IRQHandler
    .word TAMP_STAMP_IRQHandler
    .word RTC_WKUP_IRQHandler
    .word FLASH_IRQHandler
    .word RCC_IRQHandler
    .word EXTI0_IRQHandler
    .word EXTI1_IRQHandler
    .word EXTI2_IRQHandler
    .word EXTI3_IRQHandler
    .word EXTI4_IRQHandler
    .word DMA1_Stream0_IRQHandler
    .word DMA1_Stream1_IRQHandler
    .word DMA1_Stream2_IRQHandler
    .word DMA1_Stream3_IRQHandler
    .word DMA1_Stream4_IRQHandler
    .word DMA1_Stream5_IRQHandler
    .word DMA1_Stream6_IRQHandler
    .word ADC_IRQHandler
    .word FDCAN1_IT0_IRQHandler
    .word FDCAN2_IT0_IRQHandler
    .word FDCAN1_IT1_IRQHandler
    .word FDCAN2_IT1_IRQHandler
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
    .word TIM8_BRK_TIM12_IRQHandler
    .word TIM8_UP_TIM13_IRQHandler
    .word TIM8_TRG_COM_TIM14_IRQHandler
    .word TIM8_CC_IRQHandler
    .word DMA1_Stream7_IRQHandler
    .word FMC_IRQHandler
    .word SDMMC1_IRQHandler
    .word TIM5_IRQHandler
    .word SPI3_IRQHandler
    .word UART4_IRQHandler
    .word UART5_IRQHandler
    .word TIM6_DAC_IRQHandler
    .word TIM7_IRQHandler
    .word DMA2_Stream0_IRQHandler
    .word DMA2_Stream1_IRQHandler
    .word DMA2_Stream2_IRQHandler
    .word DMA2_Stream3_IRQHandler
    .word DMA2_Stream4_IRQHandler
    .word ETH_IRQHandler
    .word ETH_WKUP_IRQHandler
    .word FDCAN_CAL_IRQHandler
    .word DMA2_Stream5_IRQHandler
    .word DMA2_Stream6_IRQHandler
    .word DMA2_Stream7_IRQHandler
    .word USART6_IRQHandler
    .word I2C3_EV_IRQHandler
    .word I2C3_ER_IRQHandler
    .word OTG_HS_EP1_OUT_IRQHandler
    .word OTG_HS_EP1_IN_IRQHandler
    .word OTG_HS_WKUP_IRQHandler
    .word OTG_HS_IRQHandler
    .word DCMI_IRQHandler
    .word RNG_IRQHandler
    .word FPU_IRQHandler
    .word UART7_IRQHandler
    .word UART8_IRQHandler
    .word SPI4_IRQHandler
    .word SPI5_IRQHandler
    .word SPI6_IRQHandler
    .word SAI1_IRQHandler
    .word LTDC_IRQHandler
    .word LTDC_ER_IRQHandler
    .word DMA2D_IRQHandler
    .word SAI2_IRQHandler
    .word QUADSPI_IRQHandler
    .word LPTIM1_IRQHandler
    .word CEC_IRQHandler
    .word I2C4_EV_IRQHandler
    .word I2C4_ER_IRQHandler
    .word SPDIF_RX_IRQHandler
    .word OTG_FS_EP1_OUT_IRQHandler
    .word OTG_FS_EP1_IN_IRQHandler
    .word OTG_FS_WKUP_IRQHandler
    .word OTG_FS_IRQHandler
    .word DMAMUX1_OVR_IRQHandler
    .word HRTIM1_Master_IRQHandler
    .word HRTIM1_TIMA_IRQHandler
    .word HRTIM1_TIMB_IRQHandler
    .word HRTIM1_TIMC_IRQHandler
    .word HRTIM1_TIMD_IRQHandler
    .word HRTIM1_TIME_IRQHandler
    .word HRTIM1_FLT_IRQHandler
    .word DFSDM1_FLT0_IRQHandler
    .word DFSDM1_FLT1_IRQHandler
    .word DFSDM1_FLT2_IRQHandler
    .word DFSDM1_FLT3_IRQHandler
    .word SAI3_IRQHandler
    .word SWPMI1_IRQHandler
    .word TIM15_IRQHandler
    .word TIM16_IRQHandler
    .word TIM17_IRQHandler
    .word MDIOS_WKUP_IRQHandler
    .word MDIOS_IRQHandler
    .word JPEG_IRQHandler
    .word MDMA_IRQHandler
    .word SDMMC2_IRQHandler
    .word HSEM1_IRQHandler
    .word ADC3_IRQHandler
    .word DMAMUX2_OVR_IRQHandler
    .word BDMA_Channel0_IRQHandler
    .word BDMA_Channel1_IRQHandler
    .word BDMA_Channel2_IRQHandler
    .word BDMA_Channel3_IRQHandler
    .word BDMA_Channel4_IRQHandler
    .word BDMA_Channel5_IRQHandler
    .word BDMA_Channel6_IRQHandler
    .word BDMA_Channel7_IRQHandler
    .word COMP1_IRQHandler
    .word LPTIM2_IRQHandler
    .word LPTIM3_IRQHandler
    .word LPTIM4_IRQHandler
    .word LPTIM5_IRQHandler
    .word LPUART1_IRQHandler
    .word CRS_IRQHandler
    .word ECC_IRQHandler
    .word SAI4_IRQHandler
    .word WAKEUP_PIN_IRQHandler

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
# 时钟不动
4:
    ldr  r0, =0xE000ED88
    ldr  r1, [r0]
    orr  r1, r1, #0xF00000
    str  r1, [r0]
    dsb
    isb
# VTOR 指到本表
    ldr  r0, =_sisr
    ldr  r1, =0xE000ED08
    str  r0, [r1]
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
    WEAK PVD_AVD_IRQHandler
    WEAK TAMP_STAMP_IRQHandler
    WEAK RTC_WKUP_IRQHandler
    WEAK FLASH_IRQHandler
    WEAK RCC_IRQHandler
    WEAK EXTI0_IRQHandler
    WEAK EXTI1_IRQHandler
    WEAK EXTI2_IRQHandler
    WEAK EXTI3_IRQHandler
    WEAK EXTI4_IRQHandler
    WEAK DMA1_Stream0_IRQHandler
    WEAK DMA1_Stream1_IRQHandler
    WEAK DMA1_Stream2_IRQHandler
    WEAK DMA1_Stream3_IRQHandler
    WEAK DMA1_Stream4_IRQHandler
    WEAK DMA1_Stream5_IRQHandler
    WEAK DMA1_Stream6_IRQHandler
    WEAK ADC_IRQHandler
    WEAK FDCAN1_IT0_IRQHandler
    WEAK FDCAN2_IT0_IRQHandler
    WEAK FDCAN1_IT1_IRQHandler
    WEAK FDCAN2_IT1_IRQHandler
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
    WEAK TIM8_BRK_TIM12_IRQHandler
    WEAK TIM8_UP_TIM13_IRQHandler
    WEAK TIM8_TRG_COM_TIM14_IRQHandler
    WEAK TIM8_CC_IRQHandler
    WEAK DMA1_Stream7_IRQHandler
    WEAK FMC_IRQHandler
    WEAK SDMMC1_IRQHandler
    WEAK TIM5_IRQHandler
    WEAK SPI3_IRQHandler
    WEAK UART4_IRQHandler
    WEAK UART5_IRQHandler
    WEAK TIM6_DAC_IRQHandler
    WEAK TIM7_IRQHandler
    WEAK DMA2_Stream0_IRQHandler
    WEAK DMA2_Stream1_IRQHandler
    WEAK DMA2_Stream2_IRQHandler
    WEAK DMA2_Stream3_IRQHandler
    WEAK DMA2_Stream4_IRQHandler
    WEAK ETH_IRQHandler
    WEAK ETH_WKUP_IRQHandler
    WEAK FDCAN_CAL_IRQHandler
    WEAK DMA2_Stream5_IRQHandler
    WEAK DMA2_Stream6_IRQHandler
    WEAK DMA2_Stream7_IRQHandler
    WEAK USART6_IRQHandler
    WEAK I2C3_EV_IRQHandler
    WEAK I2C3_ER_IRQHandler
    WEAK OTG_HS_EP1_OUT_IRQHandler
    WEAK OTG_HS_EP1_IN_IRQHandler
    WEAK OTG_HS_WKUP_IRQHandler
    WEAK OTG_HS_IRQHandler
    WEAK DCMI_IRQHandler
    WEAK RNG_IRQHandler
    WEAK FPU_IRQHandler
    WEAK UART7_IRQHandler
    WEAK UART8_IRQHandler
    WEAK SPI4_IRQHandler
    WEAK SPI5_IRQHandler
    WEAK SPI6_IRQHandler
    WEAK SAI1_IRQHandler
    WEAK LTDC_IRQHandler
    WEAK LTDC_ER_IRQHandler
    WEAK DMA2D_IRQHandler
    WEAK SAI2_IRQHandler
    WEAK QUADSPI_IRQHandler
    WEAK LPTIM1_IRQHandler
    WEAK CEC_IRQHandler
    WEAK I2C4_EV_IRQHandler
    WEAK I2C4_ER_IRQHandler
    WEAK SPDIF_RX_IRQHandler
    WEAK OTG_FS_EP1_OUT_IRQHandler
    WEAK OTG_FS_EP1_IN_IRQHandler
    WEAK OTG_FS_WKUP_IRQHandler
    WEAK OTG_FS_IRQHandler
    WEAK DMAMUX1_OVR_IRQHandler
    WEAK HRTIM1_Master_IRQHandler
    WEAK HRTIM1_TIMA_IRQHandler
    WEAK HRTIM1_TIMB_IRQHandler
    WEAK HRTIM1_TIMC_IRQHandler
    WEAK HRTIM1_TIMD_IRQHandler
    WEAK HRTIM1_TIME_IRQHandler
    WEAK HRTIM1_FLT_IRQHandler
    WEAK DFSDM1_FLT0_IRQHandler
    WEAK DFSDM1_FLT1_IRQHandler
    WEAK DFSDM1_FLT2_IRQHandler
    WEAK DFSDM1_FLT3_IRQHandler
    WEAK SAI3_IRQHandler
    WEAK SWPMI1_IRQHandler
    WEAK TIM15_IRQHandler
    WEAK TIM16_IRQHandler
    WEAK TIM17_IRQHandler
    WEAK MDIOS_WKUP_IRQHandler
    WEAK MDIOS_IRQHandler
    WEAK JPEG_IRQHandler
    WEAK MDMA_IRQHandler
    WEAK SDMMC2_IRQHandler
    WEAK HSEM1_IRQHandler
    WEAK ADC3_IRQHandler
    WEAK DMAMUX2_OVR_IRQHandler
    WEAK BDMA_Channel0_IRQHandler
    WEAK BDMA_Channel1_IRQHandler
    WEAK BDMA_Channel2_IRQHandler
    WEAK BDMA_Channel3_IRQHandler
    WEAK BDMA_Channel4_IRQHandler
    WEAK BDMA_Channel5_IRQHandler
    WEAK BDMA_Channel6_IRQHandler
    WEAK BDMA_Channel7_IRQHandler
    WEAK COMP1_IRQHandler
    WEAK LPTIM2_IRQHandler
    WEAK LPTIM3_IRQHandler
    WEAK LPTIM4_IRQHandler
    WEAK LPTIM5_IRQHandler
    WEAK LPUART1_IRQHandler
    WEAK CRS_IRQHandler
    WEAK ECC_IRQHandler
    WEAK SAI4_IRQHandler
    WEAK WAKEUP_PIN_IRQHandler

