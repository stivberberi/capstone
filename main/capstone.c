#include "FreeRTOSConfig.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "portmacro.h"
#include "pressure_sensor.h"
#include "pump.h"
#include <stdio.h>

void app_main(void) {
  // logs are called with an identifier tag and a message.
  ESP_LOGI("Main", "hello, world!\n");
  TaskHandle_t read_ps_handle = NULL;

  // ADC and calibration handles for the pressure sensor:
  adc_oneshot_unit_handle_t ps_adc_handle = NULL;
  adc_cali_handle_t ps_cali_handle = NULL;
  setup_ps_adc(&ps_adc_handle, &ps_cali_handle);

  // Pressure sensor reading task
  double ps_bufr[PS_BUFR_SIZE] = {0.0};
  PsArgs ps_task_args = {
      .ps_adc_handle = ps_adc_handle,
      .ps_cali_handle = ps_cali_handle,
      .pressure_buf = ps_bufr,
  };
  xTaskCreate(read_ps_adc, "Reading Pressure Sensor", 2048, &ps_task_args, 5,
              &read_ps_handle);

  // test pump code
  setup_pump();

  while (1) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
