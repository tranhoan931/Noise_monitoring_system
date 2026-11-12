/**
 * main.c - Giam sat tieng on STM32F103C8T6 - StdPeriph + FreeRTOS
 * Chi: khoi tao phan cung, tao IPC, tao task, chay scheduler.
 *
 *   T_Acquire --Noise--> T_Alarm --Status--> T_Comm
 *
 * 4 module HAL: gpio (LED + nut), uart, lcd_i2c, micro_max
 *
 * Project: startup_stm32f10x_md.s + system_stm32f10x.c (72 MHz, HSE 8 MHz),
 *          define USE_STDPERIPH_DRIVER, STM32F10X_MD, link them -lm.
 */
#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"

#include "gpio.h"
#include "uart.h"
#include "hal_lcd_i2c.h"
#include "hal_adc_dma.h"

#include "app_ipc.h"
#include "task_acquire.h"
#include "task_alarm.h"
#include "task_comm.h"

/* Loi luc khoi tao (chay truoc scheduler): chop LED lien tuc */
static void Error_Handler(void)
{
    volatile uint32_t i;

    for (;;) {
        led_alarm_toggle();
        for (i = 0; i < 1000000UL; i++) { }
    }
}

int main(void)
{
    /* 1. Hardware & HAL init (SystemInit da chay trong startup, 72 MHz) */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);   /* bat buoc voi FreeRTOS */

    uart_init();                                      /* USART1 */
    uart_send_string("Noise monitor ready\r\n");
    gpio_init();                                      /* LED + nut SET */
    mic_init();                                       /* TIM3 + ADC1 + DMA1 (chua chay) */

    if (lcd_init() != 0) {
        Error_Handler();                              /* kiem tra dia chi I2C va day noi */
    }

    /* 2. IPC (queue) */
    if (app_ipc_init() != pdPASS) { Error_Handler(); }

    /* 3. Tao task */
    if (task_acquire_create() != pdPASS) { Error_Handler(); }
    if (task_alarm_create()   != pdPASS) { Error_Handler(); }
    if (task_comm_create()    != pdPASS) { Error_Handler(); }

    /* 4. Chay scheduler */
    vTaskStartScheduler();
    for (;;) { }
}
