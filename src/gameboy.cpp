#include "gameboy.h"
#include "cpu.h"
#include "timer.h"
#include <memory.h>

Gameboy::Gameboy(const std::string &path)
    : memory(path), cpu(memory), timer(memory) {}

void Gameboy::run() {
  // cpu.print_state();

  uint8_t cycles = cpu.step();

  timer.tick(cycles);
}

bool Gameboy::is_halted() { return cpu.is_halted(); }

void Gameboy::hexDump(const std::string &filename) const {
  memory.hexDump(filename);
}
