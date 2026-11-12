#include "uart.h"
#include <stdio.h>

void uart_init(void)
{
    GPIO_InitTypeDef  gpio;
    USART_InitTypeDef usart;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO |
                           RCC_APB2Periph_USART1, ENABLE);

    /* PA9 = TX (AF push-pull), PA10 = RX (input floating) */
    gpio.GPIO_Pin   = GPIO_Pin_9;
    gpio.GPIO_Mode  = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);
    gpio.GPIO_Pin   = GPIO_Pin_10;
    gpio.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio);

    /* USART1 115200 8N1, khong ngat, khong DMA */
    usart.USART_BaudRate            = UART_BAUDRATE;
    usart.USART_WordLength          = USART_WordLength_8b;
    usart.USART_StopBits            = USART_StopBits_1;
    usart.USART_Parity              = USART_Parity_No;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart.USART_Mode                = USART_Mode_Tx;
    USART_Init(USART1, &usart);
    USART_Cmd(USART1, ENABLE);
}

/* Gui 1 ky tu: cho thanh ghi du lieu trong (TXE) roi ghi vao */
void uart_send_char(char c)
{
    uint32_t t = UART_TIMEOUT;

    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET) {
        if (--t == 0U) { return; }
    }
    USART_SendData(USART1, (uint16_t)(uint8_t)c);
}

/* Gui ca chuoi, cho ky tu cuoi di het ra day (TC) */
void uart_send_string(const char *str)
{
    uint32_t t;

    while (*str != '\0') {
        uart_send_char(*str);
        str++;
    }
    t = UART_TIMEOUT;
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {
        if (--t == 0U) { return; }
    }
}

/* Gui 1 dong trang thai, vi du:
 *   Noise: 65.3 dB | Thr: 70 dB | NORMAL
 */
void uart_print_status(int db_x10, uint8_t threshold, uint8_t alarm)
{
    char line[48];

    snprintf(line, sizeof(line), "Noise: %d.%d dB | Thr: %u dB | %s\r\n",
             db_x10 / 10, db_x10 % 10,
             (unsigned)threshold,
             alarm ? "ALARM!" : "NORMAL");
    uart_send_string(line);
}
