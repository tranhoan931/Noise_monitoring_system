#include "app_ipc.h"

QueueHandle_t xNoiseQueue;
QueueHandle_t xStatusQueue;

volatile UBaseType_t g_hwm_acquire, g_hwm_alarm, g_hwm_comm;
volatile size_t      g_free_heap;

BaseType_t app_ipc_init(void)
{
    xNoiseQueue  = xQueueCreate(NOISE_QUEUE_LEN,  sizeof(NoiseMsg_t));
    xStatusQueue = xQueueCreate(STATUS_QUEUE_LEN, sizeof(StatusMsg_t));

    return (xNoiseQueue != NULL && xStatusQueue != NULL) ? pdPASS : pdFAIL;
}
