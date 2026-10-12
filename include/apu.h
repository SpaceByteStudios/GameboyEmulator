#pragma once

#include <cstdint>
#include <vector>

class Memory;

class APU {
public:
  APU(Memory &memory);

  void tick(uint8_t cycles);

  uint8_t read(uint16_t address);
  void write(uint16_t address, uint8_t value);

private:
  Memory &memory;

  uint8_t NR50 = 0;
  uint8_t NR51 = 0;
  uint8_t NR52 = 0;

  // Channel 1
  uint8_t NR10 = 0;
  uint8_t NR11 = 0;
  uint8_t NR12 = 0;
  uint8_t NR13 = 0;
  uint8_t NR14 = 0;

  // Channel 2
  uint8_t NR21 = 0;
  uint8_t NR22 = 0;
  uint8_t NR23 = 0;
  uint8_t NR24 = 0;

  // Channel 3
  uint8_t NR30 = 0;
  uint8_t NR31 = 0;
  uint8_t NR32 = 0;
  uint8_t NR33 = 0;
  uint8_t NR34 = 0;

  std::vector<uint8_t> wave_ram;

  // Channel 4
  uint8_t NR41 = 0;
  uint8_t NR42 = 0;
  uint8_t NR43 = 0;
  uint8_t NR44 = 0;
};
