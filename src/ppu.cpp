#include "ppu.h"

PPU::PPU(Memory &memory) : memory(memory) {}

void PPU::tick(uint8_t cycles) {}

uint8_t PPU::read(uint16_t address) const {}

void PPU::write(uint16_t address, uint8_t value) {}
