#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "lcd_screen.h"
#include "esp_log.h"

const static char *TAG = "LCD Screen";

void setup_lcd(void) {

  ESP_LOGI(TAG, "Initialize SPI bus");
  const spi_bus_config_t bus_config = {
      .mosi_io_num = LCD_MOSI,
      .miso_io_num = LCD_MISO,
      .sclk_io_num = LCD_CLK,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
      .max_transfer_sz = LCD_H_RES * LCD_V_RES * sizeof(uint16_t),
  };

  spi_bus_initialize(HSPI_HOST, &bus_config, 1);
  ESP_LOGI(TAG, "Initialize ILI9341 LCD");

  // Configure ILI9341 LCD
  const spi_device_interface_config_t dev_config = {
      .clock_speed_hz = 10 * 1000 * 1000,   // 10 MHz
      .mode = 0,                            // SPI mode 0
      .spics_io_num = LCD_CS,                // Chip select pin
      .queue_size = 1,                       // We want to be able to queue 1 transaction at a time
  };

  spi_device_handle_t spi;
  spi_bus_add_device(HSPI_HOST, &dev_config, &spi);

  // Send initialization commands to the LCD
  lcd_send_cmd(spi, ILI9341_SWRESET);       // Software reset
  vTaskDelay(100 / portTICK_PERIOD_MS);     // Delay for 100ms
  lcd_send_cmd(spi, ILI9341_SLPOUT);        // Sleep out
  vTaskDelay(100 / portTICK_PERIOD_MS);     // Delay for 100ms
  lcd_send_cmd(spi, ILI9341_DISPON);        // Display on

  // Set the cursor position
  lcd_set_cursor(spi, 0, 0);

  // Write "Hello World" to the LCD
  lcd_write_string(spi, "Hello World");

  // Clean up
  spi_bus_remove_device(spi);
  spi_bus_free(HSPI_HOST);

}