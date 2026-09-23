#pragma once

#include "cpu.h"
#include "memory.h"

class Gameboy {
public:
  Gameboy(const std::string &path);

  void reset();
  void run();
  void draw();

  bool is_halted();

  void hexDump(const std::string &filename) const;

private:
  CPU cpu;
  Memory memory;
};
