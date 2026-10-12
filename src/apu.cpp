#include "apu.h"
#include "memory.h"

#include <cstdint>

APU::APU(Memory &memory) : memory(memory), wave_ram(0x10) {}

void APU::tick(uint8_t cycles) {}

uint8_t APU::read(uint16_t address) { return 0xFF; }

void APU::write(uint16_t address, uint8_t value) { return; }
