#include "lcd_screen.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_lcd_ili9341.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_log.h"

const static char *TAG = "lcd_screen";

void example_callback(void *ctx, spi_transaction_t *trans) {
  // SPI callback function implementation
  // Add your code here
  // This is where you can implement your logic for the SPI callback function

  // For example, you can access the received data from the SPI transaction
  // using the 'trans' parameter and perform some operations on it

  // Here's an example of printing the received data to the console
  ESP_LOGI(TAG, "Received data: %s", (char *)trans->rx_buffer);

  // You can also access the context pointer 'ctx' if you need to pass any
  // additional data

  // Remember to handle any error conditions and cleanup resources if necessary
}

void setup_lcd(void) {

  ESP_LOGI(TAG, "Initialize SPI bus");
  const spi_bus_config_t bus_config = ILI9341_PANEL_BUS_SPI_CONFIG(
      LCD_CLK, LCD_MOSI, LCD_H_RES * 80 * sizeof(uint16_t));
  ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &bus_config, SPI_DMA_CH_AUTO));

  ESP_LOGI(TAG, "Install panel IO");
  esp_lcd_panel_io_handle_t io_handle = NULL;
  const esp_lcd_panel_io_spi_config_t io_config =
      ILI9341_PANEL_IO_SPI_CONFIG(LCD_CS, LCD_DC, example_callback, NULL);

  ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST,
                                           &io_config, &io_handle));

  ESP_LOGI(TAG, "Install ILI9341 panel driver");
  esp_lcd_panel_handle_t panel_handle = NULL;
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
  ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
#endif
}

int print_to_lcd(char *text) {
  // print to LCD screen
}
