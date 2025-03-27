#pragma once

#include <stdint.h>

void one_wire_init();
void one_wire_reset();
uint8_t one_wire_read_bit();
uint8_t one_wire_read_byte();
void one_wire_write_bit(uint8_t bit);
void one_wire_write_byte(uint8_t byte);
