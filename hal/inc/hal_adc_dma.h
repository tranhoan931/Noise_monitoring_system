/**
 * micro_max.h - Micro MAX9814: TIM3 TRGO -> ADC1 -> DMA1 (StdPeriph + FreeRTOS)
 *
 * MAX9814 OUT -> PA1 (ADC1_IN1), lay mau 16 kHz.
 * Ngat DMA1_Channel1 give semaphore; task goi mic_wait_block() (co timeout) de cho khoi moi.
 */
#ifndef HAL_ADC_DMA_H
#define HAL_ADC_DMA_H

#include "stm32f10x.h"
#include <stdint.h>

#define MIC_SAMPLE_RATE_HZ   16000U
#define MIC_BLOCK_SAMPLES    1600U                    /* 1 khoi = 100 ms */
#define MIC_BUFFER_SAMPLES   (MIC_BLOCK_SAMPLES * 2U) /* buffer vong ping-pong */

/* dB = 20*log10(RMS_adc) + MIC_DB_OFFSET. Hieu chuan lai bang may do dB chuan. */
#define MIC_DB_OFFSET        30.0f

void     mic_init(void);                             /* GPIO, TIM3, ADC1, DMA1, NVIC */
void     mic_start(void);
void     mic_stop(void);

/* Cho toi da timeout_ms den khi co khoi 100 ms moi. Tra ve 1 + con tro khoi, 0 neu het gio.
 * Dung semaphore (task bi block, khong busy-wait). */
uint8_t  mic_wait_block(const uint16_t **block, uint32_t timeout_ms);
uint32_t mic_get_overruns(void);                     /* so khoi bi bo lo do task xu ly cham */

float    mic_compute_rms(const uint16_t *block, uint32_t len);
float    mic_rms_to_db(float rms);
float    mic_compute_db(const uint16_t *block, uint32_t len);

#endif /* MICRO_MAX_H */
