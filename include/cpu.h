#pragma once

#include "memory.h"
#include <cstdint>

class CPU {
public:
  CPU(Memory &memory);

  void step();

private:
  Memory &memory;

  // Accumulator and Flags
  uint16_t reg_af = 0;

  // General Purpose Registers
  uint16_t reg_bc = 0;
  uint16_t reg_de = 0;
  uint16_t reg_hl = 0;

  // Stack pointer and Program Counter
  uint16_t reg_sp = 0;
  uint16_t reg_pc = 0;

  uint8_t fetch();
  void execute(uint8_t opcode);
};
