#include "pressure_sensor.h"
#include "FreeRTOSConfig.h"
#include "adc_cali_schemes.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal/adc_types.h"
#include "portmacro.h"

const static char *TAG = "Pressure Sensor"; // used as the tag for ESP_LOG's

/*
 * Sets up the ADC for the pressure sensor.
 * Sets the adc_oneshot_unit_handle_t and adc_cali_handle_t parameters
 * */
void setup_ps_adc(adc_oneshot_unit_handle_t *ps_adc_handle,
                  adc_cali_handle_t *ps_cali_handle) {
  /*-------------ADC Init---------------*/
  adc_oneshot_unit_init_cfg_t ps_init_config = {.unit_id =
                                                    PRESSURE_SENSOR_ADC_UNIT};
  ESP_ERROR_CHECK(adc_oneshot_new_unit(&ps_init_config, ps_adc_handle));

  /*-------------ADC Config---------------*/
  adc_oneshot_chan_cfg_t ps_config = {
      .atten = PRESSURE_SENSOR_ADC_ATTENUATION,
      .bitwidth = ADC_BITWIDTH_DEFAULT // automatically sets highest resolution
  };
  ESP_ERROR_CHECK(adc_oneshot_config_channel(
      *ps_adc_handle, PRESSURE_SENSOR_ADC_CHANNEL, &ps_config));

// Calibrate the ADC
#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
  ESP_LOGI(TAG, "Starting ADC calibration...");
  adc_cali_line_fitting_config_t cali_config = {
      .unit_id = PRESSURE_SENSOR_ADC_UNIT,
      .bitwidth = ADC_BITWIDTH_DEFAULT,
      .atten = PRESSURE_SENSOR_ADC_ATTENUATION,
      .default_vref = ADC_CALI_LINE_FITTING_EFUSE_VAL_EFUSE_VREF,
  };
  esp_err_t rc =
      adc_cali_create_scheme_line_fitting(&cali_config, ps_cali_handle);
  if (rc == ESP_OK) {
    ESP_LOGI(TAG, "ADC calibration successful.");
  }
#endif
}

// take pressure sensor readings; meant to be run as RTOS task
void read_ps_adc(void *ps_args) {
  for (;;) {
    PsHandle_Ptr args = (PsHandle_Ptr)ps_args;
    int adc_raw_reading;
    int voltage; // millivoltage reading

    ESP_ERROR_CHECK(adc_oneshot_read(
        *args->ps_adc_handle, PRESSURE_SENSOR_ADC_CHANNEL, &adc_raw_reading));
    ESP_LOGI(TAG, "ADC raw reading: %d", adc_raw_reading);
    if (args->ps_cali_handle != NULL) {
      // get a voltage reading from the calibration configuration. Otherwise use
      // an estimated conversion.
      ESP_ERROR_CHECK(adc_cali_raw_to_voltage(*args->ps_cali_handle,
                                              adc_raw_reading, &voltage));
      ESP_LOGI(TAG, "Calibrated voltage: %d mV", voltage);
    } else {
      // Max of 2450 mV from this:
      // https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-reference/peripherals/adc.html
      // Definitely might be wrong / needs updating / testing however.
      voltage = adc_raw_reading * 2450 / 4095;
      ESP_LOGI(TAG, "Uncalibrated voltage: %d voltage", voltage);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void cleanup_ps_adc(PsHandle_Ptr ps_handles) {
  ESP_LOGI(TAG, "Cleaning up ADC handle...");
  ESP_ERROR_CHECK(adc_oneshot_del_unit(*ps_handles->ps_adc_handle));
  if (ps_handles->ps_cali_handle != NULL) {
    ESP_LOGI(TAG, "Cleaning up ps calibration scheme...");
    ESP_ERROR_CHECK(
        adc_cali_delete_scheme_line_fitting(*ps_handles->ps_cali_handle));
  }
}
