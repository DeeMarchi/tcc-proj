#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <driver/gpio.h>
#include <rom/ets_sys.h>
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_system.h"

#include "wi_fi.h"
#include "mqtt.h"
#include "ds18b20.h"

void gpio_init()
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_Pin_5;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_Pin_14;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
    gpio_set_level(GPIO_NUM_14, 1);
}

void hello_task(void *args)
{
    while (true) {
        esp_power_consumption_info(true);
        float current_temp = ds18b20_get_current_temp();
        printf("Temp: %.2f\n", current_temp);
        vTaskDelay(5000 / portTICK_RATE_MS);
    }
}

void app_main(void)
{
    gpio_init();
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_sta();
    mqtt_app_start();
    xTaskCreate(ds18b20_task, "ds18b20_task", 2048, NULL, 5, NULL);
    xTaskCreate(hello_task, "hello_task", 2048, NULL, 5, NULL);
}
