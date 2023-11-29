#include "pump.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "hal/gpio_types.h"

const static char *TAG = "Pump";

void setup_pump_and_solenoid() {
  ESP_LOGI(TAG, "Setting up air pump and solenoid GPIO...");

  gpio_config_t pump_gpio_config = {
      .intr_type = GPIO_INTR_DISABLE,
      .mode = GPIO_MODE_OUTPUT,
      .pin_bit_mask = (1UL << RELAY_GPIO),
      .pull_up_en = 0,
      .pull_down_en = 0,
  };
  gpio_config_t solenoid_gpio_config = {
      .intr_type = GPIO_INTR_DISABLE,
      .mode = GPIO_MODE_OUTPUT,
      .pin_bit_mask = (1UL << SOLENOID_GPIO),
      .pull_up_en = 0,
      .pull_down_en = 0,
  };
  ESP_ERROR_CHECK(gpio_config(&pump_gpio_config));
  ESP_ERROR_CHECK(gpio_config(&solenoid_gpio_config));
}

/*-----Pump Control-----*/
void start_pump() {
  ESP_LOGI(TAG, "Starting pump...");
  gpio_set_level(RELAY_GPIO, 1);
}

void stop_pump() {
  ESP_LOGI(TAG, "Stopping pump...");
  gpio_set_level(RELAY_GPIO, 0);
}

/*-----Solenoid Control-----*/
void start_solenoid() {
  ESP_LOGI(TAG, "Starting solenoid...");
  gpio_set_level(SOLENOID_GPIO, 1);
}

void stop_solenoid() {
  ESP_LOGI(TAG, "Stopping solenoid...");
  gpio_set_level(SOLENOID_GPIO, 0);
}
