#include "pressure_sensor.h"
#include "FreeRTOSConfig.h"
#include "adc_cali_schemes.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "hal/adc_types.h"
#include "portmacro.h"

const static char *TAG = "Pressure_Sensor"; // used as the tag for ESP_LOG's

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
  ESP_ERROR_CHECK(adc_oneshot_config_channel(
      *ps_adc_handle, FLUID_SENSOR_ADC_CHANNEL, &ps_config));
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
    int voltage; // millivolt reading

    ESP_ERROR_CHECK(adc_oneshot_read(
        *args->ps_adc_handle, PRESSURE_SENSOR_ADC_CHANNEL, &adc_raw_reading));
    // ESP_LOGI(TAG, "ADC raw reading: %d", adc_raw_reading);
    if (args->ps_cali_handle != NULL) {
      // get a voltage reading from the calibration configuration. Otherwise use
      // an estimated conversion.
      ESP_ERROR_CHECK(adc_cali_raw_to_voltage(*args->ps_cali_handle,
                                              adc_raw_reading, &voltage));
      // ESP_LOGI(TAG, "Calibrated voltage: %d mV", voltage);
    } else {
      // Max of 2450 mV from this:
      // https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-reference/peripherals/adc.html
      // Definitely might be wrong / needs updating / testing however.
      voltage = adc_raw_reading * 2450 / 4095;
      ESP_LOGD(TAG, "Uncalibrated voltage: %d voltage", voltage);
    }
    double converted_pressure = convert_voltage_to_pressure(voltage);
    ESP_LOGD(TAG, "Converted pressure: %lf kPa\n", converted_pressure);

    // pass converted pressure
    xQueueOverwrite(*args->ps_queue, &converted_pressure);

    // run once every 1000 ms.
    vTaskDelay(500 / portTICK_PERIOD_MS);
    ESP_LOGD(TAG, "Raw voltage: %d", voltage);
    ESP_LOGD(TAG, "Converted Pressure: %lf", converted_pressure);
  }
}

// Fluid Pressure Sensor
void read_fs_adc(void *fs_args) {
  int index_read = 0;
  int index_moving = 0;

  double pressure_values[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  double pressure_peaks[3] = {0, 0, 0};

  double pressure_avg;

  for (;;) {
    FsHandle_Ptr args = (FsHandle_Ptr)fs_args;
    int adc_raw_reading;
    int voltage; // voltage reading

    ESP_ERROR_CHECK(adc_oneshot_read(
        *args->fs_adc_handle, PRESSURE_SENSOR_ADC_CHANNEL, &adc_raw_reading));
    // ESP_LOGI(TAG, "ADC raw reading: %d", adc_raw_reading);
    if (args->fs_cali_handle != NULL) {
      // get a voltage reading from the calibration configuration. Otherwise use
      // an estimated conversion.
      ESP_ERROR_CHECK(adc_cali_raw_to_voltage(*args->fs_cali_handle,
                                              adc_raw_reading, &voltage));
      // ESP_LOGI(TAG, "Calibrated voltage: %d mV", voltage);
    } else {
      // Max of 2450 mV from this:
      // https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-reference/peripherals/adc.html
      // Definitely might be wrong / needs updating / testing however.
      voltage = adc_raw_reading * 2450 / 4095;
      ESP_LOGI(TAG, "Uncalibrated voltage: %d voltage", voltage);
    }
    double converted_pressure = convert_voltage_to_fluid(voltage);
    ESP_LOGI(TAG, "Converted pressure: %lf kPa\n", converted_pressure);

    // Setting up moving average
    pressure_values[index_read] = converted_pressure;

    pressure_peaks[index_moving] = findMax(pressure_values, 10);

    pressure_avg =
        findAvg(pressure_peaks, 3); // This is the result of moving average

    // pass converted pressure
    xQueueOverwrite(*args->fs_queue, &converted_pressure);

    // Update Indices
    index_read = (index_read + 1) % 10;
    index_moving = (index_moving + 1) % 3;

    // run once every 1000 ms.
    vTaskDelay(100 / portTICK_PERIOD_MS);
    ESP_LOGI(TAG, "Raw voltage: %d", voltage);
    ESP_LOGI(TAG, "Converted Pressure: %lf", converted_pressure);
    ESP_LOGI(TAG, "Average Pressure: %lf", pressure_avg);
  }
}

int findMax(double arr[], int n) {
  int max = arr[0]; // Start with the first element
  for (int i = 1; i < n; i++) {
    if (arr[i] > max) {
      max = arr[i]; // Update max if current element is greater
    }
  }
  return max;
}

double findAvg(double arr[], int n) {
  int sum = 0;

  for (int i = 0; i < n; i++) {
    sum += arr[i];
  }

  // Calculate average
  double average = (double)sum / n;

  return average;
}

double convert_voltage_to_pressure(int voltage_mv) {
  // NOTE: probably don't want to be calculating this everytime?
  // voltage_in * Max pressure / (Span Voltage * Gain)
  // Max pressure = 37 kPa, Span voltage = 31.0 mV, Gain ~= 100
  // Also subtract 3.3 from final, to account for ~2.5mV offset * ~100 gain
  double pressure = voltage_mv * 37.0 / (31 * 100) - 1.694838;
  return pressure;
}

double convert_voltage_to_fluid(int voltage) {
  // This is for the fluid pressure sensor; Vout: 0.5-4.5V; Working Pressure
  // Rate Range: 0~1.2Mpa
  // https://www.seeedstudio.com/Water-Pressure-Sensor-G1-4-1-2MPa-p-2887.html
  double offset = 0;
  // P = 4/3 * (Vout/Vcc - 0.1)
  double pressure = (1.33333333) * (voltage / 5.0 - 0.1) + offset;
  return pressure; // This may be in MPa, so multiply by 1000 to get kPa
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
