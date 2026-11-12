/* task_comm.c - Nhan trang thai tu xStatusQueue, gui 1 dong text UART len Hercules/laptop */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "task_comm.h"
#include "app_cfg.h"
#include "app_ipc.h"
#include "uart.h"

static TaskHandle_t xCommTaskHandle = NULL;

static void prvCommTask(void *pvParameters)
{
    StatusMsg_t status;
    (void)pvParameters;

    for (;;) {
        if (xQueueReceive(xStatusQueue, &status, pdMS_TO_TICKS(COMM_WAIT_TIMEOUT_MS)) == pdPASS) {
            uart_print_status(status.db_x10, status.thr, status.alarm);
        }
        APP_HWM(g_hwm_comm);
#if APP_DEBUG_STACK
        g_free_heap = xPortGetFreeHeapSize();
#endif
    }
}

BaseType_t task_comm_create(void)
{
    return xTaskCreate(prvCommTask, "COMM", TASK_COMM_STACK, NULL,
                       TASK_COMM_PRIO, &xCommTaskHandle);
}
