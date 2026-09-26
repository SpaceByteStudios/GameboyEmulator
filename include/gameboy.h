#pragma once

#include "cpu.h"
#include "memory.h"
#include "ppu.h"
#include "timer.h"
#include <memory>

class Gameboy {
public:
  Gameboy(const std::string &path);

  void run();

  void hexDump(const std::string &filename) const;

private:
  Memory memory;

  CPU cpu;

  std::unique_ptr<Timer> timer;
  std::unique_ptr<PPU> ppu;
};
