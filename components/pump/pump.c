#include "pump.h"
#include "driver/gpio.h"
#include "hal/gpio_types.h"

void setup_pump() {
  gpio_config_t pump_gpio_config = {.intr_type = GPIO_INTR_DISABLE,
                                    .mode = GPIO_MODE_OUTPUT,
                                    .pin_bit_mask = (1UL << RELAY_GPIO),
                                    .pull_up_en = 0,
                                    .pull_down_en = 0};
  gpio_config(&pump_gpio_config);
}
