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

#define TARGET_OFFSET 7.0            // kPa (7 = 52.5 mmHg)
#define TARGET_OFFSET_CHANGE 0.67    // kPa; interval to increase offset
#define TARGET_OFFSET_THRESHOLD 0.67 // kPa (0.67 = 5 mmHg)

// Used to simulate arterial readings to test pressure tracking
const double TEST_ARTERIAL_PRESSURES[7] = {2.67,  5.33, 8.0,  10.67,
                                           13.33, 16.0, 18.67}; // kPa

// Self-Regulating Tourniquet Modes
typedef struct _mode {
  enum { ON, OFF } power_status;
  enum { INFLATED, DEFLATED, INFLATING, PAUSED } inflation_status;
  double target_offset; // offset from arterial to target pressure
  double current_arterial_pressure;
} TourniquetConfig;

static void power_button_clicked(void *arg, void *usr_data) {
  TourniquetConfig *tourniquet_configs = (TourniquetConfig *)usr_data;
  ESP_LOGD(TAG, "Power button pressed.");

  if (tourniquet_configs->power_status == OFF) {
    // Turn on LCD
    turn_on_off_lcd(true);
    tourniquet_configs->power_status = ON;
  } else {
    // acts as e-stop; deflate the cuff now
    stop_pump();
    stop_solenoid();
    tourniquet_configs->inflation_status = DEFLATED;
  }
}

static void power_button_long_press(void *arg, void *usr_data) {
  TourniquetConfig *tourniquet_configs = (TourniquetConfig *)usr_data;
  ESP_LOGD(TAG, "Power button long pressed.");

  if (tourniquet_configs->power_status == ON) {
    // turn off system
    turn_on_off_lcd(false);
    stop_solenoid();
    stop_pump();
    tourniquet_configs->power_status = OFF;
  }
}

static void start_button_clicked(void *arg, void *usr_data) {
  TourniquetConfig *tourniquet_configs = (TourniquetConfig *)usr_data;
  ESP_LOGD(TAG, "Start button pressed.");

  if (tourniquet_configs->inflation_status == DEFLATED) {
    // start the feedback loop
    tourniquet_configs->inflation_status = INFLATING;
    start_solenoid();
    start_pump();
  } else {
    // Inflated or currently inflating; either way set it to hold pressure
    tourniquet_configs->inflation_status = PAUSED;
    stop_pump();
    start_solenoid(); // in case it was deflating
  }
}

void up_button_clicked(void *arg, void *usr_data) {
  TourniquetConfig *tourniquet_configs = (TourniquetConfig *)usr_data;

  // increase target pressure by set margin
  tourniquet_configs->target_offset += TARGET_OFFSET_CHANGE; // kPa
}

void down_button_clicked(void *arg, void *usr_data) {
  TourniquetConfig *tourniquet_configs = (TourniquetConfig *)usr_data;

  // decrease target pressure by set margin
  tourniquet_configs->target_offset -= TARGET_OFFSET_CHANGE; // kPa
}

int convert_kpa_to_mmHg(double kpa_reading) {
  return (int)(kpa_reading * 7.50062);
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

  // create pressure sensor single data queue.
  QueueHandle_t fs_queue = xQueueCreate(1, sizeof(double));
  configASSERT(fs_queue != 0);
  FsHandle fs_task_args = {
      .fs_adc_handle = &ps_adc_handle,
      .fs_cali_handle = &ps_cali_handle,
      .fs_queue = &fs_queue,
  };

  TaskHandle_t read_ps_handle = NULL;
  xTaskCreate(read_ps_adc, "Reading Pressure Sensor", 2048, &ps_task_args, 5,
              &read_ps_handle);
  configASSERT(read_ps_handle);

  // Task for Fluid Pressure Sensor
  TaskHandle_t read_fs_handle = NULL;
  xTaskCreate(read_fs_adc, "Reading Fluid Pressure Sensor", 2048, &fs_task_args,
              5, &read_fs_handle);
  configASSERT(read_fs_handle);

  // setup LCD screen
  LCDStruct lcd_handles;
  setup_lcd(&lcd_handles);
  setup_lvgl_disp(&lcd_handles);

  // setup solenoid and air pump
  setup_pump_and_solenoid();
  double ps_data;
  double fs_data; // Fluid Sensor

  TourniquetConfig tourniquet_configs = {
      .power_status = OFF,
      .target_offset = TARGET_OFFSET,
      .inflation_status = DEFLATED,
      .current_arterial_pressure = 30.0,
  };

  // button set up
  button_handle_t power_button_handle = setup_button(37);
  iot_button_register_cb(power_button_handle, BUTTON_SINGLE_CLICK,
                         power_button_clicked, &tourniquet_configs);
  iot_button_register_cb(power_button_handle, BUTTON_LONG_PRESS_HOLD,
                         power_button_long_press, &tourniquet_configs);

  button_handle_t start_button_handle = setup_button(38);
  iot_button_register_cb(start_button_handle, BUTTON_SINGLE_CLICK,
                         start_button_clicked, &tourniquet_configs);

  button_handle_t up_button_handle = setup_button(5);
  iot_button_register_cb(up_button_handle, BUTTON_SINGLE_CLICK,
                         up_button_clicked, &tourniquet_configs);

  button_handle_t down_button_handle = setup_button(32);
  iot_button_register_cb(down_button_handle, BUTTON_SINGLE_CLICK,
                         down_button_clicked, &tourniquet_configs);

  // **************************************************************************
  // ------------------------END-SETUP-----------------------------------------
  // **************************************************************************
  char text[44];
  int target_pressure_idx = 0;
  double target_pressure = TEST_ARTERIAL_PRESSURES[target_pressure_idx];
  ESP_LOGI(TAG, "Target pressure: %.1lf", target_pressure);

  while (true) {
    vTaskDelay(100 / portTICK_PERIOD_MS);

    if (xQueueReceive(ps_queue, &ps_data, 100)) {
      // received data
      sprintf(text, "Cuff Pressure: %d mmHg", convert_kpa_to_mmHg(ps_data));
      update_pressure(lcd_handles.disp_handle, lcd_handles.cuff_pressure_label,
                      text);
      // print_to_lcd(&lcd_handles, text);
      ESP_LOGD(TAG, "Received %lf as ps_data", ps_data);
    }

    if (xQueueReceive(fs_queue, &fs_data, 100)) {
      // received data
      sprintf(text, "Arterial Pressure: %d mmHg", convert_kpa_to_mmHg(fs_data));
      update_pressure(lcd_handles.disp_handle,
                      lcd_handles.arterial_pressure_label, text);
      // print_to_lcd(&lcd_handles, text);
      ESP_LOGD(TAG, "Received %lf as fs_data", fs_data);
    }

    // print arterial pressure + offset
    // target_pressure = fs_data + tourniquet_configs.target_offset;
    sprintf(text, "Target Pressure: %d mmHg\nOffset: %d mmHg",
            convert_kpa_to_mmHg(target_pressure),
            convert_kpa_to_mmHg(tourniquet_configs.target_offset));
    update_pressure(lcd_handles.disp_handle, lcd_handles.set_pressure_label,
                    text);

    /**
     Demo code: prints out the taret and current pressure readings so we can
     plot it to compare to other forms of PID controller*/
    ESP_LOGI(TAG, "Cuff Pressure: %d mmHg", convert_kpa_to_mmHg(ps_data));
    ESP_LOGI(TAG, "Target Pressure: %d mmHg",
             convert_kpa_to_mmHg(target_pressure));

    if (tourniquet_configs.inflation_status == INFLATING ||
        tourniquet_configs.inflation_status == INFLATED) {
      // continue to check this even if we are inflated in case we go outside
      // the pressure thresholds
      if (ps_data < target_pressure - TARGET_OFFSET_THRESHOLD) {
        // run pump and solenoid to reach target pressure
        start_pump();
        start_solenoid();
      } else if (ps_data > target_pressure + TARGET_OFFSET_THRESHOLD) {
        // above the threshold, release solenoid to get back
        stop_pump();
        stop_solenoid();
      } else {
        ESP_LOGI(TAG, "Reached target pressure");
        tourniquet_configs.inflation_status = INFLATED;
        stop_pump();
        start_solenoid();
        // For simulation; delay 2 seconds then move to next value
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        if (target_pressure_idx == 7) {
          continue;
        } else {
          target_pressure_idx++;
          target_pressure = TEST_ARTERIAL_PRESSURES[target_pressure_idx];
          ESP_LOGI(TAG, "Target pressure: %.1lf", target_pressure);
        }
      }
    }
  }
}
