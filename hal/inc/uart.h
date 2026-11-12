/**
 * uart.h - UART (USART1) gui text cho Hercules - StdPeriph, khong RTOS
 *
 * UART : PA9 (TX), 115200 8N1, gui van ban ASCII (polling, ~3.5 ms / dong).
 *        Noi PA9 -> RXD cua mach USB-UART (CP2102/CH340/FT232), GND chung.
 *        Hercules: Serial -> chon COM, Baud 115200, Data 8, Parity none, Stop 1, Handshake OFF.
 *
 * Moi 100 ms gui 1 dong:
 *   Noise: 65.3 dB | Thr: 70 dB | NORMAL
 */
#ifndef UART_H
#define UART_H

#include "stm32f10x.h"
#include <stdint.h>

#define UART_BAUDRATE   115200U
#define UART_TIMEOUT    100000UL     /* so vong cho co TXE/TC, tranh treo task */

void uart_init(void);
void uart_send_char(char c);
void uart_send_string(const char *str);
/* db_x10 = dB * 10 (vi du 653 = 65.3 dB) */
void uart_print_status(int db_x10, uint8_t threshold, uint8_t alarm);

#endif /* UART_H */
