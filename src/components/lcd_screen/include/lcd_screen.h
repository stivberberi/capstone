#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_lcd_ili9341.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lvgl_port.h"

#define LCD_CLK GPIO_NUM_18
#define LCD_MOSI GPIO_NUM_23
#define LCD_MISO GPIO_NUM_19
#define LCD_CS GPIO_NUM_10
#define LCD_DC GPIO_NUM_9
// #define LCD_RST GPIO_NUM_10
#define LCD_BACKLITE GPIO_NUM_15

#define LCD_V_RES (240)
#define LCD_H_RES (320)
#define LCD_BIT_PER_PIXEL (16)
#define LCD_HOST SPI3_HOST

#define LCD_DRAW_BUFF_HEIGHT (50)

// struct to hold all LCD handles
typedef struct _lcd_struct {
  esp_lcd_panel_io_handle_t *io_handle;
  esp_lcd_panel_handle_t *panel_handle;
  lv_disp_t *disp_handle;
  lv_obj_t *cuff_pressure_label;
  lv_obj_t *arterial_pressure_label;
  lv_obj_t *set_pressure_label;
} LCDStruct, *LCDStruct_Ptr;

// Sets up ILI9341 LCD Panel
void setup_lcd(LCDStruct_Ptr);

// Sets up the LVGL library for using the LCD panel
void setup_lvgl_disp(LCDStruct_Ptr);

// Prints text to center of screen
void update_pressure(lv_disp_t *, lv_obj_t *, char *);

// turns on or off LCD
void turn_on_off_lcd(bool);
