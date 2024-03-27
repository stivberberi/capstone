#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_oneshot.h"
#include "freertos/FreeRTOS.h"
#include "hal/adc_types.h"

#define PRESSURE_SENSOR_ADC_CHANNEL ADC_CHANNEL_7 // GPIO35
#define PRESSURE_SENSOR_ADC_UNIT ADC_UNIT_1       // GPIO35
#define PRESSURE_SENSOR_ADC_ATTENUATION ADC_ATTEN_DB_12

// struct to pass args into read_ps_adc
typedef struct _ps_args_ {
  adc_oneshot_unit_handle_t *ps_adc_handle;
  adc_cali_handle_t *ps_cali_handle;
  QueueHandle_t *ps_queue;
} PsHandle, *PsHandle_Ptr;

void setup_ps_adc(adc_oneshot_unit_handle_t *, adc_cali_handle_t *);
void read_ps_adc(void *);
void cleanup_ps_adc(PsHandle_Ptr);
double convert_voltage_to_pressure(int);
