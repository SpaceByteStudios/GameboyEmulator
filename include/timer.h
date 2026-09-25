#pragma once

#include <cstdint>

class Memory;

class Timer {
public:
  Timer(Memory &memory);

  void tick(uint8_t cycles);

  uint8_t read(uint16_t address) const;
  void write(uint16_t address, uint8_t value);

private:
  Memory &memory;

  uint16_t divider = 0;
  uint8_t tima = 0;
  uint8_t tma = 0;
  uint8_t tac = 0;
};
