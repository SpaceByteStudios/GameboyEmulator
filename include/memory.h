#pragma once

#include <array>
#include <cstdint>

class Memory {
public:
  Memory();

  uint8_t read(uint16_t address);
  void write(uint16_t address, uint8_t value);

private:
  std::array<uint8_t, 0x10000> memory{};
};
