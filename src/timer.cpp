#include "timer.h"
#include "memory.h"
#include <cstdint>

Timer::Timer(Memory &memory) : memory(memory) {}

void Timer::tick(uint8_t cycles) {
  for (uint16_t i = 0; i < static_cast<uint16_t>(cycles) * 4; ++i) {
    static constexpr uint8_t timer_bits[] = {9, 3, 5, 7};

    uint8_t bit = timer_bits[tac & 0x03];

    bool old_signal = (tac & 0x04) && ((divider & (1 << bit)) != 0);

    divider++;

    bool new_signal = (tac & 0x04) && ((divider & (1 << bit)) != 0);

    if (old_signal && !new_signal) {
      if (tima == 0xFF) {
        tima = tma;

        uint8_t IF = memory.read(0xFF0F);
        memory.write(0xFF0F, IF | 0x04);
      } else {
        tima++;
      }
    }
  }
}

uint8_t Timer::read(uint16_t address) const {
  switch (address) {
  case 0xFF04:
    return static_cast<uint8_t>(divider >> 8);

  case 0xFF05:
    return tima;

  case 0xFF06:
    return tma;

  case 0xFF07:
    return tac | 0xF8;

  default:
    return 0xFF;
  }
}

void Timer::write(uint16_t address, uint8_t value) {
  switch (address) {
  case 0xFF04:
    divider = 0;
    break;

  case 0xFF05:
    tima = value;
    break;

  case 0xFF06:
    tma = value;
    break;

  case 0xFF07:
    tac = value & 0x07;
    break;

  default:
    break;
  }
}
