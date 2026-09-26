#pragma once

#include <cstdint>

#include "memory.h"
#include "registers.h"

class CPU {
public:
  CPU(Memory &memory);

  uint8_t step();

  void print_state();

private:
  Memory &memory;

  // Accumulator and Flags
  Reg16 reg_af;

  // General Purpose Registers
  Reg16 reg_bc;
  Reg16 reg_de;
  Reg16 reg_hl;

  // Stack pointer and Program Counter
  uint16_t reg_sp;
  uint16_t reg_pc;

  bool IME;
  bool next_IME;

  bool halted;
  bool stopped;

  uint8_t fetch();
  uint8_t execute(uint8_t opcode);

  uint8_t readR8(uint8_t r8) const;
  void writeR8(uint8_t r8, uint8_t value);

  uint16_t readR16(uint8_t r16) const;
  void writeR16(uint8_t r16, uint16_t value);

  enum class Flag : uint8_t {
    Z = 7, // Zero
    N = 6, // Subtract
    H = 5, // Half carry
    C = 4, // Carry
  };

  bool getFlag(Flag flag) const;
  void setFlag(Flag, bool value);

  void add(uint8_t b, bool carry);
  void sub(uint8_t b, bool carry);
  void cp(uint8_t b);

  bool condition(uint8_t cond) const;

  void service_pending(uint8_t pending);
};
