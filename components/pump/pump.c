#include "pump.h"
#include "FreeRTOSConfig.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "hal/gpio_types.h"
#include "portmacro.h"

const static char *TAG = "Pump";

void setup_pump() {
  gpio_config_t pump_gpio_config = {.intr_type = GPIO_INTR_DISABLE,
                                    .mode = GPIO_MODE_OUTPUT,
                                    .pin_bit_mask = (1UL << RELAY_GPIO),
                                    .pull_up_en = 0,
                                    .pull_down_en = 0};
  gpio_config(&pump_gpio_config);
}

void start_pump() {
  ESP_LOGI(TAG, "Starting pump...");
  gpio_set_level(RELAY_GPIO, 1);
}

void stop_pump() {
  ESP_LOGI(TAG, "Stopping pump...");
  gpio_set_level(RELAY_GPIO, 0);
}
