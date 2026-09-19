#include <cstdint>

#include "cpu.h"
#include "memory.h"

CPU::CPU(Memory &memory) : memory(memory) {}

void CPU::step() {
  uint8_t opcode = fetch();
  execute(opcode);
}

uint8_t CPU::fetch() { return memory.read(reg_pc); }

void CPU::execute(uint8_t opcode) {
  switch (opcode) {
  case 0x00:
    return; // NOP
  }
}
