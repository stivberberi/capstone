#include "FreeRTOSConfig.h"
#include "custom_button.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "iot_button.h"
#include "lcd_screen.h"
#include "portmacro.h"
#include "pressure_sensor.h"
#include "pump.h"
#include <stdbool.h>
#include <stdio.h>

static char *TAG = "Main";

#define TARGET_PRESSURE 20.0

ESP_ERROR_CHECK(esp_log_set_level_master(ESP_LOG_DEBUG));

// Self-Regulating Tourniquet Modes
typedef struct _mode {
  enum { ON, OFF } power_status;
  enum { INFLATED, DEFLATED, INFLATING } inflation_status;
  double set_pressure;
  double current_arterial_pressure;
} TourniquetConfig;

static void power_button_clicked(void *arg, void *usr_data) {
  TourniquetConfig *tourniquet_configs = *(TourniquetConfig *)usr_data;
  ESP_LOGI(TAG, "Power button pressed.");
  tourniquet_configs.inflation_status = INFLATING;
  start_solenoid();
  start_pump();
}

static void solenoid_release_button_clicked(void *arg, void *usr_data) {
  TourniquetConfig tourniquet_configs = *(TourniquetConfig *)usr_data;
  ESP_LOGI(TAG, "Solenoid release button pressed.");
  tourniquet_configs.inflation_status = DEFLATED;
  stop_solenoid();
  stop_pump();
}

static void stay_at_set_pressure_button(void *arg, void *usr_data) {
  TourniquetConfig tourniquet_configs = *(TourniquetConfig *)usr_data;
  ESP_LOGI(TAG, "Stay at pressure button pressed.");
  tourniquet_configs.inflation_status = INFLATED;
  stop_pump();
}

void app_main(void) {
  // logs are called with an identifier tag and a message.
  ESP_LOGI(TAG, "Welcome to group 16's capstone!");

  // **************************************************************************
  // ----------------------------SETUP-----------------------------------------
  // **************************************************************************

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

  TourniquetConfig tourniquet_configs = {
      .power_status = OFF,
      .set_pressure = 30.0,
      .inflation_status = DEFLATED,
      .current_arterial_pressure = 30.0,
  };

  // button set up
  button_handle_t power_button_handle = setup_button(37);
  iot_button_register_cb(power_button_handle, BUTTON_SINGLE_CLICK,
                         power_button_clicked,
                         &tourniquet_configs); // last arg is *usr_data

  button_handle_t solenoid_release_button_handle = setup_button(38);
  iot_button_register_cb(solenoid_release_button_handle, BUTTON_SINGLE_CLICK,
                         solenoid_release_button_clicked,
                         &tourniquet_configs); // last arg is *usr_data

  button_handle_t stay_button_handle = setup_button(5);
  iot_button_register_cb(stay_button_handle, BUTTON_SINGLE_CLICK,
                         stay_at_set_pressure_button,
                         &tourniquet_configs); // last arg is *usr_data

  // **************************************************************************
  // ------------------------END-SETUP-----------------------------------------
  // **************************************************************************

  while (true) {
    vTaskDelay(500 / portTICK_PERIOD_MS);

    if (xQueueReceive(ps_queue, &ps_data, 100)) {
      // received data
      char text[20];
      sprintf(text, "Pressure: %.1lf kPa", ps_data);
      print_to_lcd(&lcd_handles, text);
      ESP_LOGD(TAG, "Received %lf as ps_data", ps_data);
    }

    if (tourniquet_configs.inflation_status == INFLATING) {
      if (ps_data < tourniquet_configs.current_arterial_pressure) {
        continue;
      } else {
        ESP_LOGI(TAG, "Reached target pressure");
        tourniquet_configs.inflation_status = INFLATED;
        stop_pump();
      }
    }
  }
}
