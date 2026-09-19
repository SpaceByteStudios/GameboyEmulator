#include "memory.h"
#include <cstdint>

Memory::Memory() {}

uint8_t Memory::read(uint16_t address) const { return memory[address]; }

void Memory::write(uint16_t address, uint8_t value) { memory[address] = value; }

uint16_t Memory::read16(uint16_t address) const {
  uint16_t high_byte = static_cast<uint16_t>(memory[address]);
  uint16_t low_byte = static_cast<uint16_t>(memory[address + 1]);

  return (high_byte << 8) + low_byte;
}

void Memory::write16(uint16_t address, uint16_t value) {
  memory[address] = value >> 8;
  memory[address + 1] = value & 0x00FF;
}
