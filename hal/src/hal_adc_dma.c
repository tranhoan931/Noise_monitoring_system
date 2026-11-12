#include "hal_adc_dma.h"
#include <math.h>          /* link voi -lm */
#include "FreeRTOS.h"
#include "semphr.h"

static uint16_t          s_buf[MIC_BUFFER_SAMPLES];
static volatile uint8_t  s_ready_half;     /* 0 = nua dau vua day, 1 = nua sau */
static volatile uint32_t s_overruns;
static SemaphoreHandle_t s_sem;            /* ISR give, task take */

void mic_init(void)
{
    GPIO_InitTypeDef         gpio;
    TIM_TimeBaseInitTypeDef  tim;
    DMA_InitTypeDef          dma;
    ADC_InitTypeDef          adc;
    NVIC_InitTypeDef         nvic;

    s_sem = xSemaphoreCreateBinary();

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);                 /* ADCCLK = 72/6 = 12 MHz */

    /* PA1: ngo vao analog */
    gpio.GPIO_Pin  = GPIO_Pin_1;
    gpio.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &gpio);

    /* TIM3: tran moi 1/16000 s, phat TRGO moi lan tran */
    TIM_TimeBaseStructInit(&tim);
    tim.TIM_Prescaler     = 0;
    tim.TIM_Period        = (uint16_t)((SystemCoreClock / MIC_SAMPLE_RATE_HZ) - 1U); /* 4499 */
    tim.TIM_ClockDivision = TIM_CKD_DIV1;
    tim.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &tim);
    TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update);

    /* DMA1 kenh 1: ADC1->DR -> s_buf, vong, half-word */
    DMA_DeInit(DMA1_Channel1);
    dma.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
    dma.DMA_MemoryBaseAddr     = (uint32_t)s_buf;
    dma.DMA_DIR                = DMA_DIR_PeripheralSRC;
    dma.DMA_BufferSize         = MIC_BUFFER_SAMPLES;
    dma.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    dma.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    dma.DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord;
    dma.DMA_Mode               = DMA_Mode_Circular;
    dma.DMA_Priority           = DMA_Priority_High;
    dma.DMA_M2M                = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &dma);
    DMA_ITConfig(DMA1_Channel1, DMA_IT_HT | DMA_IT_TC, ENABLE);

    nvic.NVIC_IRQChannel                   = DMA1_Channel1_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 6;       /* >= 5 de goi duoc API FromISR */
    nvic.NVIC_IRQChannelSubPriority        = 0;
    nvic.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&nvic);

    /* ADC1: 1 kenh (IN1), trigger ngoai tu TIM3 TRGO, xin DMA */
    ADC_DeInit(ADC1);
    adc.ADC_Mode               = ADC_Mode_Independent;
    adc.ADC_ScanConvMode       = DISABLE;
    adc.ADC_ContinuousConvMode = DISABLE;
    adc.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_T3_TRGO;
    adc.ADC_DataAlign          = ADC_DataAlign_Right;
    adc.ADC_NbrOfChannel       = 1;
    ADC_Init(ADC1, &adc);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_55Cycles5);
    ADC_DMACmd(ADC1, ENABLE);
    ADC_Cmd(ADC1, ENABLE);

    /* Hieu chuan ADC */
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1)) { }
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1)) { }

    ADC_ExternalTrigConvCmd(ADC1, ENABLE);            /* ADC chi chay khi TIM3 phat TRGO */
}

void mic_start(void)
{
    DMA_Cmd(DMA1_Channel1, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
}

void mic_stop(void)
{
    TIM_Cmd(TIM3, DISABLE);
    DMA_Cmd(DMA1_Channel1, DISABLE);
}

uint8_t mic_wait_block(const uint16_t **block, uint32_t timeout_ms)
{
    if (xSemaphoreTake(s_sem, pdMS_TO_TICKS(timeout_ms)) != pdTRUE) {
        return 0;                                     /* het gio, khong co khoi moi */
    }
    *block = &s_buf[s_ready_half ? MIC_BLOCK_SAMPLES : 0U];
    return 1;
}

uint32_t mic_get_overruns(void)
{
    return s_overruns;
}

float mic_compute_rms(const uint16_t *block, uint32_t len)
{
    uint32_t i;
    uint32_t sum = 0;
    int32_t  mean;
    uint64_t sq = 0;

    if (block == 0 || len == 0U) {
        return 0.0f;
    }
    for (i = 0; i < len; i++) {
        sum += block[i];
    }
    mean = (int32_t)(sum / len);                      /* thanh phan mot chieu */

    for (i = 0; i < len; i++) {
        int32_t d = (int32_t)block[i] - mean;
        sq += (uint64_t)(d * d);
    }
    return sqrtf((float)sq / (float)len);
}

float mic_rms_to_db(float rms)
{
    if (rms < 1.0f) {
        rms = 1.0f;
    }
    return 20.0f * log10f(rms) + MIC_DB_OFFSET;
}

float mic_compute_db(const uint16_t *block, uint32_t len)
{
    return mic_rms_to_db(mic_compute_rms(block, len));
}

/* Ngat DMA1 kenh 1: nua buffer (HT) hoac day buffer (TC) -> danh thuc task */
void DMA1_Channel1_IRQHandler(void)
{
    BaseType_t woken = pdFALSE;
    uint8_t    got   = 0;

    if (DMA_GetITStatus(DMA1_IT_HT1) != RESET) {
        DMA_ClearITPendingBit(DMA1_IT_HT1);
        s_ready_half = 0;
        got = 1;
    }
    if (DMA_GetITStatus(DMA1_IT_TC1) != RESET) {
        DMA_ClearITPendingBit(DMA1_IT_TC1);
        s_ready_half = 1;
        got = 1;
    }
    /* semaphore con day = task chua kip lay khoi truoc -> dem 1 lan bo lo */
    if (got && xSemaphoreGiveFromISR(s_sem, &woken) == pdFALSE) {
        s_overruns++;
    }
    portYIELD_FROM_ISR(woken);
}
