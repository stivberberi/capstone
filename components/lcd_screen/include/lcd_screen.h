#include "driver/gpio.h"
#include "driver/spi_master.h"

#define LCD_CLK GPIO_NUM_18
#define LCD_MOSI GPIO_NUM_23
#define LCD_MISO GPIO_NUM_19
#define LCD_CS GPIO_NUM_10
#define LCD_DC GPIO_NUM_9

#define LCD_H_RES              (320)
#define LCD_V_RES              (240)
#define LCD_BIT_PER_PIXEL      (16)
#define LCD_HOST               SPI3_HOST

void setup_lcd(void);
