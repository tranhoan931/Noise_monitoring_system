/**
 * app_cfg.h - Bang thong so RTOS (nhom thong nhat, KHONG sua tuy tien)
 */
#ifndef APP_CFG_H
#define APP_CFG_H

#include "FreeRTOS.h"
#include "task.h"

/* ---- Do uu tien: Acquire cao nhat vi co deadline 100 ms (DMA ghi de neu tre) ---- */
#define TASK_ACQUIRE_PRIO     (tskIDLE_PRIORITY + 3)
#define TASK_ALARM_PRIO       (tskIDLE_PRIORITY + 2)
#define TASK_COMM_PRIO        (tskIDLE_PRIORITY + 1)

/* ---- Stack (words, 1 word = 4 byte) ---- */
#define TASK_ACQUIRE_STACK    256U     /* sqrtf, log10f */
#define TASK_ALARM_STACK      256U     /* snprintf */
#define TASK_COMM_STACK       256U     /* snprintf trong uart_print_status */

/* ---- Queue ---- */
#define NOISE_QUEUE_LEN       2U       /* T_Acquire -> T_Alarm  (NoiseMsg_t)  */
#define STATUS_QUEUE_LEN      4U       /* T_Alarm   -> T_Comm   (StatusMsg_t) */

/* ---- Timeout cho cac ham cho (khong block vinh vien) ---- */
#define ACQ_WAIT_TIMEOUT_MS   300U     /* binh thuong 100 ms/khoi */
#define COMM_WAIT_TIMEOUT_MS  1000U

/* ---- Tham so ung dung ---- */
#define THRESHOLD_DEFAULT_DB  70U
#define THRESHOLD_MIN_DB      40U
#define THRESHOLD_MAX_DB      110U
#define ALARM_HYST_X10        20       /* tat bao dong khi dB < nguong - 2.0 */
#define SET_TIMEOUT_MS        5000U    /* khong thao tac 5 s thi thoat che do SET */

/* 1 = moi task ghi stack high-water-mark ra bien toan cuc de xem bang debugger */
#define APP_DEBUG_STACK       1

#endif /* APP_CFG_H */
