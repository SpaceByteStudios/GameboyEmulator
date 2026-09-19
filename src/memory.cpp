#include "memory.h"
#include <cstdint>

Memory::Memory() {}

uint8_t Memory::read(uint16_t address) { return memory[address]; }

void Memory::write(uint16_t address, uint8_t value) { memory[address] = value; }
