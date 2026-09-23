#include "gameboy.h"
#include "cpu.h"
#include <memory.h>

Gameboy::Gameboy(const std::string &path) : memory(), cpu(memory) {
  memory.loadROM(path);
}

void Gameboy::reset() {}

void Gameboy::run() {
  cpu.print_state();
  cpu.step();
}

void Gameboy::draw() {}

bool Gameboy::is_halted() { return cpu.is_halted(); }

void Gameboy::hexDump(const std::string &filename) const {
  memory.hexDump(filename);
}
