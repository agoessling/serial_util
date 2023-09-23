#pragma once

#include <stdint.h>
#include <stddef.h>

uint16_t Crc16Update(const uint8_t *table, uint16_t crc, uint8_t byte);
uint16_t Crc16Block(const uint8_t *table, const uint8_t *input, size_t len, uint16_t initial_crc);
