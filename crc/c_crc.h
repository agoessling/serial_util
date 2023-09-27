#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
  const uint8_t *table;  // 256 element CRC table.
  uint8_t initial_crc;  // Initial CRC value.
  uint8_t final_xor;  // Value of successful CRC.
  bool lsb_first;  // Input and output reflected?
} Crc8Info;

typedef struct {
  const uint16_t *table;  // 256 element CRC table.
  uint16_t initial_crc;  // Initial CRC value.
  uint16_t final_xor;  // Value of successful CRC.
  bool lsb_first;  // Input and output reflected?
} Crc16Info;

typedef struct {
  const uint32_t *table;  // 256 element CRC table.
  uint32_t initial_crc;  // Initial CRC value.
  uint32_t final_xor;  // Value of successful CRC.
  bool lsb_first;  // Input and output reflected?
} Crc32Info;

uint8_t Crc8Update(const Crc8Info *info, uint8_t crc, uint8_t byte);
uint8_t Crc8Seq(const Crc8Info *info, const uint8_t *input, size_t len, uint8_t crc);
uint8_t Crc8Block(const Crc8Info *info, const uint8_t *input, size_t len);

uint16_t Crc16Update(const Crc16Info *info, uint16_t crc, uint8_t byte);
uint16_t Crc16Seq(const Crc16Info *info, const uint8_t *input, size_t len, uint16_t crc);
uint16_t Crc16Block(const Crc16Info *info, const uint8_t *input, size_t len);

uint32_t Crc32Update(const Crc32Info *info, uint32_t crc, uint8_t byte);
uint32_t Crc32Seq(const Crc32Info *info, const uint8_t *input, size_t len, uint32_t crc);
uint32_t Crc32Block(const Crc32Info *info, const uint8_t *input, size_t len);
