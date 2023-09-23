#include "crc/c_crc.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

uint8_t Crc8Update(const Crc8Info *info, uint8_t crc, uint8_t byte) {
  const uint8_t idx = crc ^ byte;
  return info->table[idx];
}

uint8_t Crc8Seq(const Crc8Info *info, const uint8_t *input, size_t len, uint8_t crc) {
  for (size_t i = 0; i < len; ++i) {
    crc = Crc8Update(info, crc, input[i]);
  }
  return crc;
}

uint8_t Crc8Block(const Crc8Info *info, const uint8_t *input, size_t len) {
  return info->final_xor ^ Crc8Seq(info, input, len, info->initial_crc);
}

uint16_t Crc16Update(const Crc16Info *info, uint16_t crc, uint8_t byte) {
  if (info->lsb_first) {
    const uint8_t idx = crc ^ byte;
    return (crc >> 8) ^ info->table[idx];
  }

  const uint8_t idx = (crc >> 8) ^ byte;
  return (crc << 8) ^ info->table[idx];
}

uint16_t Crc16Seq(const Crc16Info *info, const uint8_t *input, size_t len, uint16_t crc) {
  for (size_t i = 0; i < len; ++i) {
    crc = Crc16Update(info, crc, input[i]);
  }
  return crc;
}

uint16_t Crc16Block(const Crc16Info *info, const uint8_t *input, size_t len) {
  return info->final_xor ^ Crc16Seq(info, input, len, info->initial_crc);
}

uint32_t Crc32Update(const Crc32Info *info, uint32_t crc, uint8_t byte) {
  if (info->lsb_first) {
    const uint8_t idx = crc ^ byte;
    return (crc >> 8) ^ info->table[idx];
  }

  const uint8_t idx = (crc >> 24) ^ byte;
  return (crc << 8) ^ info->table[idx];
}

uint32_t Crc32Seq(const Crc32Info *info, const uint8_t *input, size_t len, uint32_t crc) {
  for (size_t i = 0; i < len; ++i) {
    crc = Crc32Update(info, crc, input[i]);
  }
  return crc;
}

uint32_t Crc32Block(const Crc32Info *info, const uint8_t *input, size_t len) {
  return info->final_xor ^ Crc32Seq(info, input, len, info->initial_crc);
}
