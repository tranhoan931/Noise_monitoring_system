/* task_acquire.c - Thu mau micro, tinh dB, gui qua xNoiseQueue */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "task_acquire.h"
#include "app_cfg.h"
#include "app_ipc.h"
#include "hal_adc_dma.h"

static TaskHandle_t xAcquireTaskHandle = NULL;

static void prvAcquireTask(void *pvParameters)
{
    const uint16_t *blk;
    NoiseMsg_t      msg;
    (void)pvParameters;

    mic_start();                                      /* bat TIM3 + ADC + DMA */

    for (;;) {
        /* cho khoi 100 ms moi tu ISR DMA, co timeout */
        if (mic_wait_block(&blk, ACQ_WAIT_TIMEOUT_MS)) {
            msg.db_x10 = (int16_t)(mic_compute_db(blk, MIC_BLOCK_SAMPLES) * 10.0f + 0.5f);
            (void)xQueueSend(xNoiseQueue, &msg, 0);   /* day thi bo, khong cho */
        }
        APP_HWM(g_hwm_acquire);
    }
}

BaseType_t task_acquire_create(void)
{
    return xTaskCreate(prvAcquireTask, "ACQUIRE", TASK_ACQUIRE_STACK, NULL,
                       TASK_ACQUIRE_PRIO, &xAcquireTaskHandle);
}
