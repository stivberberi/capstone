#include <stdio.h>
#include "button.h"
#include "button_gpio.h"
#include "iot_button.h"
#include "esp_log.h"

const static char *TAG = "Button"; // used as the tag for ESP_LOG's

esp_log_level_set("*", ESP_LOG_INFO);

void func(void)
{

// create gpio button
button_config_t gpio_btn_cfg = {
    .type = BUTTON_TYPE_GPIO,
    .long_press_ticks = CONFIG_BUTTON_LONG_PRESS_TIME_MS,
    .short_press_ticks = CONFIG_BUTTON_SHORT_PRESS_TIME_MS,
    .gpio_button_config = {
        .gpio_num = 0,
        .active_level = 0,
    },
};
button_handle_t gpio_btn = iot_button_create(&gpio_btn_cfg);
if(NULL == gpio_btn) {
    ESP_LOGE(TAG, "Button create failed");
}

}
