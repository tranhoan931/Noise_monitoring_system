/* task_alarm.c - Nut SET, cai nguong, canh bao co tre, LED, LCD */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>
#include "task_alarm.h"
#include "app_cfg.h"
#include "app_ipc.h"
#include "gpio.h"
#include "hal_lcd_i2c.h"

static TaskHandle_t xAlarmTaskHandle = NULL;

static void prvLcdDraw(uint8_t set_mode, int16_t db_x10, uint8_t thr, uint8_t alarm)
{
    char line[24];

    if (set_mode) {
        lcd_print_line(0, "SET THRESHOLD");
        snprintf(line, sizeof(line), "Thr: %3u dB", (unsigned)thr);
        lcd_print_line(1, line);
    } else {
        snprintf(line, sizeof(line), "Noise:%3d.%d dB", db_x10 / 10, db_x10 % 10);
        lcd_print_line(0, line);
        snprintf(line, sizeof(line), "Thr:%3u %s", (unsigned)thr, alarm ? "ALARM!" : "NORMAL");
        lcd_print_line(1, line);
    }
}

static void prvAlarmTask(void *pvParameters)
{
    TickType_t  xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod  = pdMS_TO_TICKS(BTN_SCAN_MS);   /* 10 ms */

    uint8_t     set_mode = 0;
    uint8_t     alarm    = 0;
    uint8_t     thr      = THRESHOLD_DEFAULT_DB;
    int16_t     db_x10   = 0;
    uint32_t    set_tick = 0;
    NoiseMsg_t  noise;
    StatusMsg_t status;
    (void)pvParameters;

    prvLcdDraw(set_mode, db_x10, thr, alarm);

    for (;;) {
        uint32_t       now      = xTaskGetTickCount();     /* tick = 1 ms */
        uint8_t        new_data = 0;
        uint8_t        redraw   = 0;
        button_event_t ev       = button_update(now);

        /* 1. Lay dB moi nhat tu T_Acquire (khong cho) */
        while (xQueueReceive(xNoiseQueue, &noise, 0) == pdPASS) {
            db_x10   = noise.db_x10;
            new_data = 1;
        }

        /* 2. Nut SET */
        if (!set_mode) {
            if (ev != BTN_EVENT_NONE) {                  /* nhan -> vao che do SET */
                set_mode = 1;
                set_tick = now;
                redraw   = 1;
            }
        } else {
            if (ev == BTN_EVENT_SHORT) {                 /* tang 1 dB, het khoang ve min */
                thr      = (thr >= THRESHOLD_MAX_DB) ? THRESHOLD_MIN_DB : (uint8_t)(thr + 1U);
                set_tick = now;
                redraw   = 1;
            } else if (ev == BTN_EVENT_LONG || (now - set_tick) >= SET_TIMEOUT_MS) {
                set_mode = 0;                            /* xac nhan nguong */
                redraw   = 1;
            }
        }

        /* 3. Canh bao co tre + LED */
        if (!alarm && db_x10 >= (int16_t)(thr * 10)) {
            alarm = 1;
        } else if (alarm && db_x10 < (int16_t)(thr * 10 - ALARM_HYST_X10)) {
            alarm = 0;
        }
        led_alarm_set(alarm);

        /* 4. Co dB moi: gui trang thai cho T_Comm */
        if (new_data) {
            status.db_x10 = db_x10;
            status.thr    = thr;
            status.alarm  = alarm;
            (void)xQueueSend(xStatusQueue, &status, 0);
        }

        /* 5. LCD: ve khi co dB moi (100 ms) hoac khi thao tac nut */
        if (new_data || redraw) {
            prvLcdDraw(set_mode, db_x10, thr, alarm);
        }

        APP_HWM(g_hwm_alarm);
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

BaseType_t task_alarm_create(void)
{
    return xTaskCreate(prvAlarmTask, "ALARM", TASK_ALARM_STACK, NULL,
                       TASK_ALARM_PRIO, &xAlarmTaskHandle);
}
