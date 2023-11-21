#include "FreeRTOSConfig.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "portmacro.h"
#include "pressure_sensor.h"
#include <stdio.h>

void app_main(void) {
  // logs are called with an identifier tag and a message.
  ESP_LOGI("Main", "hello, world!\n");
  TaskHandle_t read_ps_handle = NULL;

  // ADC and calibration handles for the pressure sensor:
  adc_oneshot_unit_handle_t ps_adc_handle;
  adc_cali_handle_t ps_cali_handle;
  setup_ps_adc(&ps_adc_handle, &ps_cali_handle);
  PsHandle ps_task_args = {
      .ps_adc_handle = &ps_adc_handle,
      .ps_cali_handle = &ps_cali_handle,
  };
  xTaskCreate(read_ps_adc, "Reading Pressure Sensor", 2048, &ps_task_args, 5,
              &read_ps_handle);
  configASSERT(read_ps_handle);

  while (1) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
