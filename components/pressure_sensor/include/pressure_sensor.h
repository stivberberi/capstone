#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_oneshot.h"
#include "hal/adc_types.h"

#define PRESSURE_SENSOR_ADC_CHANNEL ADC_CHANNEL_7       // GPIO35
#define PRESSURE_SENSOR_ADC_UNIT ADC_UNIT_1             // GPIO35
#define PRESSURE_SENSOR_ADC_ATTENUATION ADC_ATTEN_DB_11 // to go up to 3V3
#define PS_BUFR_SIZE 256

// struct to pass args into read_ps_adc
typedef struct _ps_args_ {
  adc_oneshot_unit_handle_t ps_adc_handle;
  adc_cali_handle_t ps_cali_handle;
  double pressure_buf[PS_BUFR_SIZE];
} PsArgs;

void setup_ps_adc(adc_oneshot_unit_handle_t *, adc_cali_handle_t *);
void read_ps_adc(void *);
void cleanup_ps_adc(PsArgs *);
double convert_voltage_to_pressure(int);
