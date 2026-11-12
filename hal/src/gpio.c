#include "gpio.h"

/* ================= Nut nhan ================= */
typedef enum {
    BTN_ST_IDLE = 0,
    BTN_ST_DEBOUNCE,
    BTN_ST_PRESSED,
    BTN_ST_WAIT_RELEASE
} btn_state_t;

static btn_state_t s_state;
static uint32_t    s_t0;

/* ================= Khoi tao ================= */
void gpio_init(void)
{
    GPIO_InitTypeDef gpio;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    /* LED PB0: output push-pull, tat luc khoi dong */
    GPIO_ResetBits(LED_ALARM_PORT, LED_ALARM_PIN);
    gpio.GPIO_Pin   = LED_ALARM_PIN;
    gpio.GPIO_Mode  = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(LED_ALARM_PORT, &gpio);

    /* Nut PB8: input keo len */
    gpio.GPIO_Pin  = BTN_PIN;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(BTN_PORT, &gpio);

    s_state = BTN_ST_IDLE;
    s_t0    = 0;
}

/* ================= LED ================= */
void led_alarm_set(uint8_t on)
{
    if (on) {
        GPIO_SetBits(LED_ALARM_PORT, LED_ALARM_PIN);
    } else {
        GPIO_ResetBits(LED_ALARM_PORT, LED_ALARM_PIN);
    }
}

void led_alarm_toggle(void)
{
    if (GPIO_ReadOutputDataBit(LED_ALARM_PORT, LED_ALARM_PIN) != Bit_RESET) {
        GPIO_ResetBits(LED_ALARM_PORT, LED_ALARM_PIN);
    } else {
        GPIO_SetBits(LED_ALARM_PORT, LED_ALARM_PIN);
    }
}

/* ================= Nut: state machine chong rung ================= */
button_event_t button_update(uint32_t now)
{
    uint8_t pressed = (GPIO_ReadInputDataBit(BTN_PORT, BTN_PIN) == BTN_ACTIVE_LEVEL) ? 1U : 0U;

    switch (s_state) {
    case BTN_ST_IDLE:
        if (pressed) {
            s_t0    = now;
            s_state = BTN_ST_DEBOUNCE;
        }
        break;

    case BTN_ST_DEBOUNCE:
        if (!pressed) {
            s_state = BTN_ST_IDLE;               /* nhieu / rung */
        } else if ((now - s_t0) >= BTN_DEBOUNCE_MS) {
            s_state = BTN_ST_PRESSED;
        }
        break;

    case BTN_ST_PRESSED:
        if (!pressed) {
            s_state = BTN_ST_IDLE;
            return BTN_EVENT_SHORT;
        }
        if ((now - s_t0) >= BTN_LONG_PRESS_MS) {
            s_state = BTN_ST_WAIT_RELEASE;
            return BTN_EVENT_LONG;
        }
        break;

    case BTN_ST_WAIT_RELEASE:
        if (!pressed) {
            s_state = BTN_ST_IDLE;
        }
        break;

    default:
        s_state = BTN_ST_IDLE;
        break;
    }
    return BTN_EVENT_NONE;
}
