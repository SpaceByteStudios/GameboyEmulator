#pragma once

#include "cpu.h"
#include "memory.h"

class Gameboy {
public:
  Gameboy();

  void reset();
  void run();
  void draw();

  bool is_running();

private:
  CPU cpu;
  Memory memory;

  bool running;
};
