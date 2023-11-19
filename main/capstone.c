#include "FreeRTOSConfig.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "portmacro.h"
#include "pressure_sensor.h"
#include <stdio.h>

void app_main(void) {
  // logs are called with an identifier tag and a message.
  ESP_LOGI("Main", "hello, world!\n");
  TaskHandle_t read_pressure_handle = NULL;
  xTaskCreate(read_ps_adc, "Reading Pressure Sensor", 2048, NULL, 5,
              &read_pressure_handle);
  configASSERT(read_pressure_handle);
  while (1) {
    vTaskDelay(1000);
  }
}
