#pragma once

#include "memory.h"
#include <cstdint>

class Timer {
public:
  Timer(Memory &memory);

  void tick(uint8_t cycles);

private:
  Memory &memory;

  uint16_t divider = 0;
  uint8_t tima = 0;
  uint8_t tma = 0;
  uint8_t tac = 0;
};
