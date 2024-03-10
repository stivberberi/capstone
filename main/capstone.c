#include "FreeRTOSConfig.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "portmacro.h"
#include "pressure_sensor.h"
#include "pump.h"
#include "iot_button.h"
#include "custom_button.h"

static void button_single_click_cb(void *arg,void *usr_data)
{
  ESP_LOGI("Sarahs button", "BUTTON_SINGLE_CLICK");
}


void app_main(void) {
  // logs are called with an identifier tag and a message.
  ESP_LOGI("Main", "hello, world!\n");
  // TaskHandle_t read_ps_handle = NULL;

  // // ADC and calibration handles for the pressure sensor:
  // adc_oneshot_unit_handle_t ps_adc_handle;
  // adc_cali_handle_t ps_cali_handle;
  // setup_ps_adc(&ps_adc_handle, &ps_cali_handle);
  // PsHandle ps_task_args = {
  //     .ps_adc_handle = &ps_adc_handle,
  //     .ps_cali_handle = &ps_cali_handle,
  // };
  // xTaskCreate(read_ps_adc, "Reading Pressure Sensor", 2048, &ps_task_args, 5,
  //             &read_ps_handle);
  // configASSERT(read_ps_handle);

  // button set up
  button_handle_t button_1_handle = setup_button(26);
  button_handle_t button_2_handle = setup_button(30);
  

  iot_button_regiscter_cb(button_1_handle, BUTTON_SINGLE_CLICK, button_single_click_cb,NULL);
  iot_button_regiscter_cb(button_2_handle, BUTTON_SINGLE_CLICK, button_single_click_cb,NULL);

  // test pump code
  // setup_pump_and_solenoid();
  // start_solenoid();
  // start_pump();
  // vTaskDelay(30000 / portTICK_PERIOD_MS);
  // stop_solenoid();
  // stop_pump();

  while (1) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
