/* Ham hook cua FreeRTOS: bat loi tran stack / het heap */
#include "FreeRTOS.h"
#include "task.h"
#include "gpio.h"

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;
    led_alarm_set(1);                 /* LED sang cung = loi nghiem trong */
    taskDISABLE_INTERRUPTS();
    for (;;) { }
}

void vApplicationMallocFailedHook(void)
{
    led_alarm_set(1);
    taskDISABLE_INTERRUPTS();
    for (;;) { }
}
