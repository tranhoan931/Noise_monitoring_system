/**
 * app_ipc.h - Doi tuong IPC dung chung giua cac task
 *
 *   T_Acquire --xNoiseQueue--> T_Alarm --xStatusQueue--> T_Comm
 */
#ifndef APP_IPC_H
#define APP_IPC_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdint.h>

/* dB do duoc (x10) */
typedef struct {
    int16_t db_x10;
} NoiseMsg_t;

/* Trang thai day du gui len laptop */
typedef struct {
    int16_t db_x10;
    uint8_t thr;
    uint8_t alarm;
} StatusMsg_t;

extern QueueHandle_t xNoiseQueue;
extern QueueHandle_t xStatusQueue;

BaseType_t app_ipc_init(void);       /* tao queue, goi truoc khi tao task */

/* Debug stack (xem trong Watch window cua Keil khi APP_DEBUG_STACK = 1) */
extern volatile UBaseType_t g_hwm_acquire, g_hwm_alarm, g_hwm_comm;
extern volatile size_t      g_free_heap;

#include "app_cfg.h"
#if APP_DEBUG_STACK
#define APP_HWM(var)   ((var) = uxTaskGetStackHighWaterMark(NULL))
#else
#define APP_HWM(var)   ((void)0)
#endif

#endif /* APP_IPC_H */
