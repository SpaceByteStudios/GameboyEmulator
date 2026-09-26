#pragma once

#include <cstdint>

class Memory;

class PPU {
public:
  PPU(Memory &memory);

  void tick(uint8_t cycles);

  uint8_t read(uint16_t address) const;
  void write(uint16_t address, uint8_t value);

private:
  Memory &memory;
};
