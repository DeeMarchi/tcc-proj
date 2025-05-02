#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include <driver/gpio.h>

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "ds18b20.h"

static const char *TAG = "MQTT_MAIN";
static const char temp_activation_topic[] = "/test/control/enable_temp_celsius";

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            esp_mqtt_client_publish(client, "/test/ds18b20/temp_celsius", ds18b20_get_current_temp_str(), 0, 2, 0);
            esp_mqtt_client_subscribe(client, temp_activation_topic, 2);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            char publish_json_msg[128] = { 0 };
            char json_format[] =
                "{"
                    "\"temperature\": %s,\n\t"
                    "\"activationTemperature\": %s,\n\t"
                    "\"relayActive\": %d\n\t"
                "}";
            int current_gpio_level = gpio_get_level(GPIO_NUM_14);
            snprintf(
                publish_json_msg,
                sizeof(publish_json_msg),
                json_format,
                ds18b20_get_current_temp_str(),
                ds18b20_get_activation_temp_str(),
                current_gpio_level
            );
            esp_mqtt_client_publish(client, "/test/ds18b20/temp_celsius", publish_json_msg, 0, 2, 0);
            float current_temp = ds18b20_get_current_temp();
            float activation_temp = ds18b20_get_activation_temp();
            if (current_temp > activation_temp && current_gpio_level == 1) {
                printf("Releasing the relay (closing the circuit and activating the fan)\n");
                gpio_set_level(GPIO_NUM_14, 0);
            } else if (current_temp <= activation_temp && current_gpio_level == 0) {
                gpio_set_level(GPIO_NUM_14, 1);
            }
            vTaskDelay(1000 / portTICK_RATE_MS);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            char incoming_topic_buf[256] = { 0 };
            snprintf(incoming_topic_buf, sizeof(incoming_topic_buf), "%.*s", event->topic_len, event->topic);
            if (strncmp(temp_activation_topic, incoming_topic_buf, sizeof(temp_activation_topic)) == 0) {
                char float_buf[32] = { 0 };
                snprintf(float_buf, sizeof(float_buf), "%.*s", event->data_len, event->data);
                float new_activation_temp = atof(float_buf);
                ds18b20_set_activation_temp(new_activation_temp);
            }
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            ESP_LOGE(TAG, "Event error: %d", event->error_handle->error_type);
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}

void mqtt_app_start(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_DEBUG);
    esp_log_level_set("MQTT_MAIN", ESP_LOG_DEBUG);
    esp_log_level_set("TRANSPORT_TCP", ESP_LOG_DEBUG);
    esp_log_level_set("TRANSPORT_SSL", ESP_LOG_DEBUG);
    esp_log_level_set("TRANSPORT", ESP_LOG_DEBUG);
    esp_log_level_set("OUTBOX", ESP_LOG_DEBUG);
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = CONFIG_BROKER_URL,
    };
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}
