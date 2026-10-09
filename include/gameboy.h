#pragma once

#include "cpu.h"
#include "memory.h"
#include "ppu.h"
#include "timer.h"
#include <memory>

class Gameboy {
public:
  Gameboy(const std::string &path);

  void step();
  void runFrame();

  void test_screen();

  std::vector<uint8_t> getScreen();

  void updateJoypadInput();

  void hexDump(const std::string &filename);

private:
  Memory memory;

  CPU cpu;

  std::unique_ptr<Timer> timer;
  std::unique_ptr<PPU> ppu;
};
