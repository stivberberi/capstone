#include "../components/lcd_screen/include/lv_conf.h"
#include "FreeRTOSConfig.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "lcd_screen.h"
#include "portmacro.h"
#include "pressure_sensor.h"
#include "pump.h"
#include <stdbool.h>
#include <stdio.h>

static char *TAG = "Main";

#define TARGET_PRESSURE 30.0

void app_main(void) {
  // logs are called with an identifier tag and a message.
  ESP_LOGI(TAG, "hello, world!\n");

  // ADC and calibration handles for the pressure sensor:
  adc_oneshot_unit_handle_t ps_adc_handle;
  adc_cali_handle_t ps_cali_handle;
  setup_ps_adc(&ps_adc_handle, &ps_cali_handle);

  // create pressure sensor single data queue.
  QueueHandle_t ps_queue = xQueueCreate(1, sizeof(double));
  configASSERT(ps_queue != 0);
  PsHandle ps_task_args = {
      .ps_adc_handle = &ps_adc_handle,
      .ps_cali_handle = &ps_cali_handle,
      .ps_queue = &ps_queue,
  };

  TaskHandle_t read_ps_handle = NULL;
  xTaskCreate(read_ps_adc, "Reading Pressure Sensor", 2048, &ps_task_args, 5,
              &read_ps_handle);
  configASSERT(read_ps_handle);

  // setup LCD screen
  LCDStruct lcd_handles;
  setup_lcd(&lcd_handles);
  setup_lvgl_disp(&lcd_handles);
  print_to_lcd(&lcd_handles, "Group 16 Capstone");

  // setup solenoid and air pump
  setup_pump_and_solenoid();
  double ps_data;

  bool run_demo = true;
  start_pump();
  start_solenoid();
  while (true) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    if (xQueueReceive(ps_queue, &ps_data, 100)) {
      // received data
      char text[20];
      sprintf(text, "Pressure: %.1lf kPa", ps_data);
      print_to_lcd(&lcd_handles, text);
      ESP_LOGD(TAG, "Received %lf as ps_data", ps_data);
    }
    if (run_demo) {
      if (ps_data < TARGET_PRESSURE) {
        continue;
      } else {
        ESP_LOGI(TAG, "Reached target pressure");
        stop_pump();
        run_demo = false;
      }
    }
  }
}
