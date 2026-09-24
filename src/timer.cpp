#include "timer.h"
#include "memory.h"
#include <cstdint>

Timer::Timer(Memory &memory) : memory(memory) {}

void Timer::tick(uint8_t cycles) {
  for (uint8_t i = 0; i < cycles * 4; ++i) {
    bool timer_enable = (tac & 0x04);

    static constexpr uint8_t timer_bits[] = {9, 3, 5, 7};
    uint8_t bit = timer_bits[tac & 0x03];

    bool old_signal = timer_enable && divider & (1 << bit);
    divider += 1;
    bool new_signal = timer_enable && divider & (1 << bit);

    bool falling_edge = old_signal && !new_signal;

    if (falling_edge) {
      if (++tima == 0) {
        tima = tma;

        uint8_t IF = memory.read(0xFF0F);
        memory.write(0xFF0F, IF | 0x4);
      }
    }
  }
}
