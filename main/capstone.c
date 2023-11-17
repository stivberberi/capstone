#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "portmacro.h"
#include "pressure_sensor.h"

void app_main(void)
{
  while (1) {
    printf("hello, world!\n");
    pressure_hello();
    // delay 1s
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    fflush(stdout);
  }
}
