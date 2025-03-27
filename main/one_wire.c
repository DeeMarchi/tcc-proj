#include "one_wire.h"

#include <driver/gpio.h>
#include <rom/ets_sys.h>

#define ONE_WIRE_PIN GPIO_NUM_5

void one_wire_init() {
    gpio_set_level(ONE_WIRE_PIN, 1); // Set pin high initially
}

void one_wire_reset() {
    gpio_set_level(ONE_WIRE_PIN, 0); // Pull the line low
    ets_delay_us(480);
    gpio_set_level(ONE_WIRE_PIN, 1); // Release the line
    ets_delay_us(60);
    
    if (gpio_get_level(ONE_WIRE_PIN) == 0) {
        ets_delay_us(240);
    }
}

uint8_t one_wire_read_bit() {
    uint8_t bit;
    gpio_set_level(ONE_WIRE_PIN, 0);  // Pull line low
    ets_delay_us(2);
    gpio_set_level(ONE_WIRE_PIN, 1); // Release line
    ets_delay_us(10);
    bit = gpio_get_level(ONE_WIRE_PIN);
    ets_delay_us(50);
    return bit;
}

uint8_t one_wire_read_byte() {
    uint8_t i, value = 0;
    for (i = 0; i < 8; i++) {
        value |= (one_wire_read_bit() << i);
    }
    return value;
}

void one_wire_write_bit(uint8_t bit) {
    gpio_set_level(ONE_WIRE_PIN, 0); // Pull line low
    ets_delay_us(2);
    
    gpio_set_level(ONE_WIRE_PIN, bit); // Set to the desired bit
    ets_delay_us(60);
    gpio_set_level(ONE_WIRE_PIN, 1); // Ensure release
}

void one_wire_write_byte(uint8_t byte) {
    uint8_t i;
    for (i = 0; i < 8; i++) {
        one_wire_write_bit((byte >> i) & 1);
    }
}
