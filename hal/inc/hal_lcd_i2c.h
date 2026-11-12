/**
 * lcd_i2c.h - LCD 1602 qua module I2C PCF8574, che do 4 bit - StdPeriph, khong RTOS
 *
 * I2C1: PB6 (SCL), PB7 (SDA), 100 kHz, polling co timeout. Dia chi thuong gap 0x27 hoac 0x3F.
 * PCF8574: P0=RS, P1=RW, P2=EN, P3=den nen, P4..P7=D4..D7.
 * Delay dung bo dem chu ky DWT (khong can SysTick).
 */
#ifndef HAL_LCD_I2C_H
#define HAL_LCD_I2C_H

#include "stm32f10x.h"
#include <stdint.h>

#define LCD_I2C_ADDR        (0x27U << 1)     /* doi thanh (0x3FU << 1) neu dung dia chi 0x3F */
#define LCD_I2C_SPEED_HZ    100000U
#define LCD_I2C_TIMEOUT     60000UL          /* so vong cho co I2C */
#define LCD_COLS            16U
#define LCD_ROWS            2U

/* Tra ve 0 neu OK, -1 neu LCD khong phan hoi */
int  lcd_init(void);
void lcd_clear(void);
void lcd_home(void);
void lcd_backlight(uint8_t on);
void lcd_set_cursor(uint8_t col, uint8_t row);
void lcd_print(const char *str);
/* Ghi ca 1 dong (tu chen khoang trang cho du 16 ky tu) */
void lcd_print_line(uint8_t row, const char *str);

#endif /* LCD_I2C_H */
