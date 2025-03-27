#include <stdint.h>
#include <rom/ets_sys.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ds18b20.h"
#include "one_wire.h"

#define DS18B20_CMD_SKIP_ROM      0xCC
#define DS18B20_CMD_CONVERT_T     0x44
#define DS18B20_CMD_READ_SCRATCHPAD 0xBE

static char temp_str_buf[100];
static float temp = 0.0f;
static float activation_temp = 125.0f; // Default value to not activate before configuring

void ds18b20_read_scratchpad()
{
    one_wire_reset();
    one_wire_write_byte(DS18B20_CMD_SKIP_ROM);
    one_wire_write_byte(DS18B20_CMD_READ_SCRATCHPAD);
}

void ds18b20_convert_t()
{
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(760);
    xLastWakeTime = xTaskGetTickCount();
    one_wire_reset();
    one_wire_write_byte(DS18B20_CMD_SKIP_ROM);
    one_wire_write_byte(DS18B20_CMD_CONVERT_T);
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
}

void ds18b20_task(void *args)
{
    uint8_t temperature_data[2];
    one_wire_init();
    for (;;) {
        taskENTER_CRITICAL();
        ds18b20_convert_t();
        ds18b20_read_scratchpad();
        temperature_data[0] = one_wire_read_byte();
        temperature_data[1] = one_wire_read_byte();
        taskEXIT_CRITICAL();
        int16_t raw_temperature = (temperature_data[1] << 8) | temperature_data[0];
        temp = raw_temperature / 16.0f;
    }
}

float ds18b20_get_current_temp()
{
    return temp;
}

char *ds18b20_get_current_temp_str()
{
    sprintf(temp_str_buf, "%.2f", temp);
    return temp_str_buf;
}

void ds18b20_set_activation_temp(float temp)
{
    activation_temp = temp;
}

float ds18b20_get_activation_temp()
{
    return activation_temp;
}
