#include "gameboy.h"
#include "cpu.h"
#include "ppu.h"
#include "timer.h"
#include <cstdint>
#include <memory.h>
#include <memory>

Gameboy::Gameboy(const std::string &path)
    : memory(path), cpu(memory), timer(std::make_unique<Timer>(memory)),
      ppu(std::make_unique<PPU>(memory)) {
  memory.setTimer(timer.get());
  memory.setPPU(ppu.get());
}

void Gameboy::run() {
  // cpu.print_state();

  uint8_t cycles = cpu.step();

  timer->tick(cycles);
  ppu->tick(cycles);
}

void Gameboy::hexDump(const std::string &filename) const {
  memory.hexDump(filename);
}
