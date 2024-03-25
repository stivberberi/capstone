#include "lcd_screen.h"
#include "core/lv_obj.h"
#include "core/lv_obj_style.h"
#include "core/lv_obj_style_gen.h"
#include "core/lv_obj_tree.h"
#include "esp_err.h"
#include "esp_heap_caps.h"
#include "esp_lcd_panel_ops.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "esp_lvgl_port_disp.h"
#include "misc/lv_color.h"
#include "misc/lv_style.h"
#include "misc/lv_types.h"
#include "widgets/label/lv_label.h"

const static char *TAG = "lcd_screen";

void setup_lcd(LCDStruct_Ptr lcd_handles) {
  ESP_LOGI(TAG, "Initialize SPI bus");
  const spi_bus_config_t bus_config = ILI9341_PANEL_BUS_SPI_CONFIG(
      LCD_CLK, LCD_MOSI, LCD_H_RES * LCD_DRAW_BUFF_HEIGHT * sizeof(uint16_t));
  ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &bus_config, SPI_DMA_CH_AUTO));

  ESP_LOGI(TAG, "Install panel IO");
  static esp_lcd_panel_io_handle_t io_handle = NULL;

  // The NULL's are meant to handle CB functions; unclear if needed as of now
  const esp_lcd_panel_io_spi_config_t io_config =
      ILI9341_PANEL_IO_SPI_CONFIG(LCD_CS, LCD_DC, NULL, NULL);

  ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST,
                                           &io_config, &io_handle));

  ESP_LOGI(TAG, "Install ILI9341 panel driver");
  static esp_lcd_panel_handle_t panel_handle = NULL;
  const esp_lcd_panel_dev_config_t panel_config = {
    .reset_gpio_num = -1, // Set to -1 if not use

#if ESP_IDF_VERSION <                                                          \
    ESP_IDF_VERSION_VAL(5, 0, 0) // Implemented by LCD command `36h`
    .color_space = ESP_LCD_COLOR_SPACE_RGB,
#else
    .rgb_endian = LCD_RGB_ENDIAN_RGB,
#endif
    .bits_per_pixel = 16, // Implemented by LCD command `3Ah` (16/18)
    // .vendor_config = &vendor_config,            // Uncomment this line if use
    // custom initialization commands
  };
  ESP_ERROR_CHECK(
      esp_lcd_new_panel_ili9341(io_handle, &panel_config, &panel_handle));
  ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
  ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
  ESP_ERROR_CHECK(esp_lcd_panel_disp_off(panel_handle, false));
#else
  // turn display on
  ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
#endif

  // swap orientation
  esp_lcd_panel_mirror(panel_handle, true, false);

  // save handles to struct
  lcd_handles->io_handle = &io_handle;
  lcd_handles->panel_handle = &panel_handle;
}

void setup_lvgl_disp(LCDStruct_Ptr lcd_handles) {
  const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
  ESP_ERROR_CHECK(lvgl_port_init(&lvgl_cfg));

  static lv_disp_t *disp_handle;

  const lvgl_port_display_cfg_t disp_cfg = {
      .io_handle = *lcd_handles->io_handle,
      .panel_handle = *lcd_handles->panel_handle,
      .buffer_size = LCD_H_RES * LCD_DRAW_BUFF_HEIGHT * sizeof(uint16_t),
      .double_buffer = true,
      .hres = LCD_H_RES,
      .vres = LCD_V_RES,
      .monochrome = false,
      .mipi_dsi = false,
      /* Rotation values must be same as used in esp_lcd for initial settings of
         the screen */
      .rotation =
          {
              .swap_xy = false,
              .mirror_x = false,
              .mirror_y = false,
          },
      .flags = {
          .buff_dma = true,
          .swap_bytes = false,
      }};

  ESP_LOGI(TAG, "FREE BUFFER SIZE: %zul",
           heap_caps_get_free_size(MALLOC_CAP_DEFAULT));
  disp_handle = lvgl_port_add_disp(&disp_cfg);

  // save to struct
  lcd_handles->disp_handle = disp_handle;
}

int print_to_lcd(LCDStruct_Ptr lcd_handles, char *text) {
  // according to esp_lvgl_port we need this before and after any screen
  // operations
  lvgl_port_lock(0);

  // might want this in LCDStruct_Ptr?
  lv_obj_t *screen = lv_disp_get_scr_act(lcd_handles->disp_handle);

  // hello world lvgl example
  static lv_style_t style_label;
  lv_obj_clean(screen);
  lv_style_init(&style_label);
  lv_style_set_text_font(&style_label, &lv_font_montserrat_20);
  lv_obj_set_style_bg_color(screen, lv_color_hex(0x003a57), LV_PART_MAIN);
  lv_obj_t *label = lv_label_create(screen);
  lv_label_set_text(label, text);
  lv_obj_set_style_text_color(screen, lv_color_hex(0xffffff), LV_PART_MAIN);
  lv_obj_add_style(label, &style_label, LV_PART_MAIN);
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

  // in conjuction with lvgl_port_lock
  lvgl_port_unlock();
  return 0;
}

void cleanup_lcd(LCDStruct_Ptr lcd_handles) {
  lvgl_port_remove_disp(lcd_handles->disp_handle);
}
