#include "FreeRTOSConfig.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "lcd_screen.h"
#include "portmacro.h"
#include "pressure_sensor.h"
#include "pump.h"

void app_main(void) {
  // logs are called with an identifier tag and a message.
  ESP_LOGI("Main", "hello, world!\n");

  // ADC and calibration handles for the pressure sensor:
  adc_oneshot_unit_handle_t ps_adc_handle;
  adc_cali_handle_t ps_cali_handle;
  setup_ps_adc(&ps_adc_handle, &ps_cali_handle);
  PsHandle ps_task_args = {
      .ps_adc_handle = &ps_adc_handle,
      .ps_cali_handle = &ps_cali_handle,
  };

  // TaskHandle_t read_ps_handle = NULL;
  // xTaskCreate(read_ps_adc, "Reading Pressure Sensor", 2048, &ps_task_args, 5,
  //             &read_ps_handle);
  // configASSERT(read_ps_handle);

  // setup LCD screen
  LCDStruct lcd_handles;
  setup_lcd(&lcd_handles);
  setup_lvgl_disp(&lcd_handles);
  print_to_lcd(&lcd_handles, "Hello World!");

  // test pump code
  setup_pump_and_solenoid();
  start_solenoid();
  start_pump();
  vTaskDelay(30000 / portTICK_PERIOD_MS);
  stop_solenoid();
  stop_pump();

  while (1) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
