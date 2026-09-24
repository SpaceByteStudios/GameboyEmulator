#pragma once

#include "cpu.h"
#include "memory.h"
#include "timer.h"

class Gameboy {
public:
  Gameboy(const std::string &path);

  void run();

  bool is_halted();

  void hexDump(const std::string &filename) const;

private:
  Memory memory;
  CPU cpu;
  Timer timer;
};
