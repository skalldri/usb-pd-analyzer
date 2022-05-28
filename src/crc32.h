#pragma once

#include <cstdint>

uint32_t crc32(uint32_t crc, const uint8_t *buf, uint32_t len, uint32_t polynomial);

uint32_t reverse(uint32_t input);

uint32_t little_to_big_endian(uint32_t input);