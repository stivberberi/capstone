#include "../components/lcd_screen/include/lv_conf.h"
#include "FreeRTOSConfig.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "lcd_screen.h"
#include "portmacro.h"
#include "pressure_sensor.h"
#include "pump.h"
#include "iot_button.h"
#include "custom_button.h"

static void button_single_click_cb(void *arg,void *usr_data)
{
  ESP_LOGI("Sarahs button", "BUTTON_SINGLE_CLICK");
}


static char *TAG = "Main";

void app_main(void) {
  // logs are called with an identifier tag and a message.
  ESP_LOGI(TAG, "hello, world!\n");

  // button set up
  button_handle_t button_1_handle = setup_button(26);
  button_handle_t button_2_handle = setup_button(30);

  iot_button_regiscter_cb(button_1_handle, BUTTON_SINGLE_CLICK, button_single_click_cb,NULL);
  iot_button_regiscter_cb(button_2_handle, BUTTON_SINGLE_CLICK, button_single_click_cb,NULL);

  // ADC and calibration handles for the pressure sensor:
  adc_oneshot_unit_handle_t ps_adc_handle;
  adc_cali_handle_t ps_cali_handle;
  setup_ps_adc(&ps_adc_handle, &ps_cali_handle);

  // create pressure sensor single data queue.
  QueueHandle_t ps_queue = xQueueCreate(1, sizeof(double));
  configASSERT(ps_queue == 0);
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
  print_to_lcd(&lcd_handles, "Hi BeReal");

  // setup solenoid and air pump
  setup_pump_and_solenoid();
  double ps_data;

  while (1) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    if (xQueueReceive(ps_queue, &ps_data, 100)) {
      // received data
      print_to_lcd(&lcd_handles, "Pressure: %lf kPa", ps_data);
      ESP_LOGD(TAG, "Received %lf as ps_data", ps_data);
    }
  }
}
