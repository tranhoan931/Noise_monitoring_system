#include "hal_lcd_i2c.h"

/* Bit dieu khien tren PCF8574 (P0..P3), P4..P7 = D4..D7 cua LCD */
#define PCF_RS   0x01U
#define PCF_EN   0x04U
#define PCF_BL   0x08U

#define LCD_CMD_CLEAR        0x01U
#define LCD_CMD_HOME         0x02U
#define LCD_CMD_ENTRY_MODE   0x06U
#define LCD_CMD_DISPLAY_OFF  0x08U
#define LCD_CMD_DISPLAY_ON   0x0CU
#define LCD_CMD_FUNC_SET     0x28U   /* 4 bit, 2 dong, 5x8 */
#define LCD_CMD_SET_DDRAM    0x80U

static uint8_t s_backlight = PCF_BL;

/* ================= Delay bang DWT->CYCCNT ================= */
static void dwt_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;
}

static void delay_ms(uint32_t ms)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = ms * (SystemCoreClock / 1000U);
    while ((uint32_t)(DWT->CYCCNT - start) < ticks) { }
}

/* ================= I2C: gui 1 byte toi PCF8574 =================
 * Moi lan goi = 1 giao dich day du:
 *   START -> dia chi -> 1 byte du lieu -> STOP
 * Tra ve 0 neu OK, -1 neu loi (timeout hoac khong co ACK).
 */
static int i2c_wait_event(uint32_t event)
{
    uint32_t t = LCD_I2C_TIMEOUT;

    while (!I2C_CheckEvent(I2C1, event)) {
        if (--t == 0U) {
            return -1;
        }
    }
    return 0;
}

static int i2c_fail(void)
{
    I2C_GenerateSTOP(I2C1, ENABLE);
    I2C_ClearFlag(I2C1, I2C_FLAG_AF);            /* xoa co NACK */
    return -1;
}

static int i2c_send_byte(uint8_t data)
{
    uint32_t t = LCD_I2C_TIMEOUT;

    /* 1. Cho bus ranh */
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY)) {
        if (--t == 0U) {
            return i2c_fail();
        }
    }

    /* 2. START */
    I2C_GenerateSTART(I2C1, ENABLE);
    if (i2c_wait_event(I2C_EVENT_MASTER_MODE_SELECT) != 0) {
        return i2c_fail();
    }

    /* 3. Gui dia chi, cho PCF8574 tra ACK */
    I2C_Send7bitAddress(I2C1, LCD_I2C_ADDR, I2C_Direction_Transmitter);
    if (i2c_wait_event(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) != 0) {
        return i2c_fail();
    }

    /* 4. Gui 1 byte du lieu */
    I2C_SendData(I2C1, data);
    if (i2c_wait_event(I2C_EVENT_MASTER_BYTE_TRANSMITTED) != 0) {
        return i2c_fail();
    }

    /* 5. STOP */
    I2C_GenerateSTOP(I2C1, ENABLE);
    return 0;
}

/* ================= LCD qua PCF8574 ================= */

/* Gui 4 bit cao cua 'nibble' toi LCD: bat EN roi tat EN de LCD chot du lieu */
static int lcd_write_nibble(uint8_t nibble, uint8_t rs)
{
    uint8_t d = (uint8_t)((nibble & 0xF0U) | s_backlight | (rs ? PCF_RS : 0U));

    if (i2c_send_byte((uint8_t)(d | PCF_EN)) != 0) {
        return -1;
    }
    return i2c_send_byte(d);
}

/* Gui 1 byte = 2 nibble (cao truoc, thap sau). rs=0: lenh, rs=1: ky tu */
static void lcd_write_byte(uint8_t value, uint8_t rs)
{
    (void)lcd_write_nibble(value, rs);                         /* 4 bit cao */
    (void)lcd_write_nibble((uint8_t)(value << 4), rs);         /* 4 bit thap */
}

static void lcd_cmd(uint8_t cmd)
{
    lcd_write_byte(cmd, 0);
    if (cmd == LCD_CMD_CLEAR || cmd == LCD_CMD_HOME) {
        delay_ms(2);                             /* lenh xoa / ve dau can ~1.5 ms */
    }
}

static void lcd_data(uint8_t ch)
{
    lcd_write_byte(ch, 1);
}

/* ================= Ham cong khai ================= */
int lcd_init(void)
{
    GPIO_InitTypeDef gpio;
    I2C_InitTypeDef  i2c;

    dwt_init();

    /* PB6 = SCL, PB7 = SDA (open-drain, module da co dien tro keo len) */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    gpio.GPIO_Pin   = GPIO_Pin_6 | GPIO_Pin_7;
    gpio.GPIO_Mode  = GPIO_Mode_AF_OD;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpio);

    I2C_DeInit(I2C1);
    i2c.I2C_Mode                = I2C_Mode_I2C;
    i2c.I2C_DutyCycle           = I2C_DutyCycle_2;
    i2c.I2C_OwnAddress1         = 0x00;
    i2c.I2C_Ack                 = I2C_Ack_Enable;
    i2c.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    i2c.I2C_ClockSpeed          = LCD_I2C_SPEED_HZ;
    I2C_Init(I2C1, &i2c);
    I2C_Cmd(I2C1, ENABLE);

    s_backlight = PCF_BL;
    delay_ms(50);                                /* doi LCD len nguon */

    /* Chuoi khoi dong chuyen LCD sang che do 4 bit (theo datasheet HD44780) */
    if (lcd_write_nibble(0x30U, 0) != 0) {
        return -1;                               /* khong co ACK: sai dia chi / noi day */
    }
    delay_ms(5);
    (void)lcd_write_nibble(0x30U, 0);
    delay_ms(2);
    (void)lcd_write_nibble(0x30U, 0);
    delay_ms(2);
    (void)lcd_write_nibble(0x20U, 0);            /* chuyen sang 4 bit */
    delay_ms(2);

    lcd_cmd(LCD_CMD_FUNC_SET);
    lcd_cmd(LCD_CMD_DISPLAY_OFF);
    lcd_cmd(LCD_CMD_CLEAR);
    lcd_cmd(LCD_CMD_ENTRY_MODE);
    lcd_cmd(LCD_CMD_DISPLAY_ON);
    return 0;
}

void lcd_clear(void)
{
    lcd_cmd(LCD_CMD_CLEAR);
}

void lcd_home(void)
{
    lcd_cmd(LCD_CMD_HOME);
}

void lcd_backlight(uint8_t on)
{
    s_backlight = on ? PCF_BL : 0U;
    (void)i2c_send_byte(s_backlight);
}

void lcd_set_cursor(uint8_t col, uint8_t row)
{
    uint8_t addr;

    if (row >= LCD_ROWS) { row = LCD_ROWS - 1U; }
    if (col >= LCD_COLS) { col = LCD_COLS - 1U; }

    addr = (row == 0U) ? 0x00U : 0x40U;          /* dong 0 bat dau 0x00, dong 1 bat dau 0x40 */
    lcd_cmd((uint8_t)(LCD_CMD_SET_DDRAM | (addr + col)));
}

void lcd_print(const char *str)
{
    while (*str != '\0') {
        lcd_data((uint8_t)*str);
        str++;
    }
}

void lcd_print_line(uint8_t row, const char *str)
{
    uint8_t i;

    lcd_set_cursor(0, row);
    for (i = 0; i < LCD_COLS; i++) {
        if (*str != '\0') {
            lcd_data((uint8_t)*str);
            str++;
        } else {
            lcd_data(' ');                       /* chen khoang trang cho du 16 ky tu */
        }
    }
}
