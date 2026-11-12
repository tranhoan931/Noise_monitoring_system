/**
 * gpio.h - GPIO: 1 LED bao dong (PB0) + 1 nut SET (PB8) - StdPeriph, khong RTOS
 *
 * LED : PB0 (den do + dien tro 220-330 ohm xuong GND)
 * Nut : PB8 noi xuong GND (dien tro keo len noi bo), quet 10 ms, chong rung phan mem.
 *       Goi button_update(now_ms) moi BTN_SCAN_MS, now_ms = xTaskGetTickCount() (tick FreeRTOS = 1 ms).
 */
#ifndef GPIO_H
#define GPIO_H

#include "stm32f10x.h"
#include <stdint.h>

/* ---- LED ---- */
#define LED_ALARM_PORT      GPIOB
#define LED_ALARM_PIN       GPIO_Pin_0

/* ---- Nut nhan ---- */
#define BTN_PORT            GPIOB
#define BTN_PIN             GPIO_Pin_8
#define BTN_ACTIVE_LEVEL    Bit_RESET        /* nhan = muc thap */

#define BTN_SCAN_MS         10U
#define BTN_DEBOUNCE_MS     30U
#define BTN_LONG_PRESS_MS   1000U

typedef enum {
    BTN_EVENT_NONE = 0,
    BTN_EVENT_SHORT,     /* nhan roi tha truoc BTN_LONG_PRESS_MS */
    BTN_EVENT_LONG       /* giu qua BTN_LONG_PRESS_MS (bao 1 lan) */
} button_event_t;

void           gpio_init(void);              /* khoi tao LED + nut */

void           led_alarm_set(uint8_t on);
void           led_alarm_toggle(void);

button_event_t button_update(uint32_t now_ms);

#endif /* GPIO_H */
