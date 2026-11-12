# Giam sat tieng on - STM32F103C8T6 + FreeRTOS (StdPeriph)

Task: T_Acquire (mic -> dB) -> T_Alarm (nut, nguong, LED, LCD) -> T_Comm (UART text)

## Cau hinh Keil
- Define: USE_STDPERIPH_DRIVER, STM32F10X_MD. Link -lm.
- Include path: hal/inc, app/inc, middlewares,
  <FreeRTOS>/Source/include, <FreeRTOS>/Source/portable/RVDS/ARM_CM3
- Them file kernel: tasks.c, queue.c, list.c, portable/RVDS/ARM_CM3/port.c, portable/MemMang/heap_4.c
- Them file project: main.c, hal/src/*.c, app/src/*.c
- Neu co stm32f10x_it.c: xoa SVC_Handler, PendSV_Handler, SysTick_Handler.

## Kiem tra khi chay (APP_DEBUG_STACK = 1)
Watch: g_hwm_acquire, g_hwm_alarm, g_hwm_comm (word stack con trong, nen > 20), g_free_heap.
