#include "crc/c_crc.h"

#include <stddef.h>
#include <stdint.h>

uint16_t Crc16Update(const uint8_t *table, uint16_t crc, uint8_t byte) {
  return (crc >> 8) ^ table[(uint8_t)crc ^ byte];
}

uint16_t Crc16Block(const uint8_t *table, const uint8_t *input, size_t len, uint16_t initial_crc) {
  uint16_t crc = initial_crc;
  for (size_t i = 0; i < len; ++i) {
    crc = Crc16Update(table, crc, input[i]);
  }
  return crc;
}
