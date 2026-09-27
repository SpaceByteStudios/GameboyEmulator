#include "memory.h"
#include "cartridge.h"
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory.h>

Memory::Memory(const std::string &path) : memory(0x10000), cartridge(path) {}

uint8_t Memory::read(uint16_t address) const {
  // ROM
  if (address < 0x8000) {
    return cartridge.read(address);
  }

  // VRAM
  if ((0x8000 <= address) && (address < 0xA000)) {
    return ppu->read(address);
  }

  // External RAM
  if ((0xA000 <= address) && (address < 0xC000)) {
    return cartridge.read(address);
  }

  // WRAM
  if ((0xC000 <= address) && (address < 0xE000)) {
    return memory[address];
  }

  // Echo RAM
  if ((0xE000 <= address) && (address < 0xFE00)) {
    return memory[address - 0x1000];
  }

  // OAM
  if ((0xFE00 <= address) && (address < 0xFEA0)) {
    return ppu->read(address);
  }

  // Not Usable
  if ((0xFEA0 <= address) && (address < 0xFF00)) {
    return 0xFF;
  }

  // Timer I/O
  if (address >= 0xFF04 && address <= 0xFF07) {
    return timer->read(address);
  }

  // PPU I/O
  if (address >= 0xFF40 && address <= 0xFF4B) {
    return ppu->read(address);
  }

  // IF
  if (address == 0xFF0F) {
    return memory[address] | 0xE0;
  }

  // IE
  if (address == 0xFFFF) {
    return memory[address] | 0xE0;
  }

  // hardcoded VBlank
  if (address == 0xFF44) {
    return 0x90;
  }

  return memory[address];
}

void Memory::write(uint16_t address, uint8_t value) {
  // ROM
  if (address < 0x8000) {
    cartridge.write(address, value);
    return;
  }

  // VRAM
  if ((0x8000 <= address) && (address < 0xA000)) {
    ppu->write(address, value);
    return;
  }

  // External RAM
  if ((0xA000 <= address) && (address < 0xC000)) {
    cartridge.write(address, value);
    return;
  }

  // WRAM
  if ((0xC000 <= address) && (address < 0xE000)) {
    memory[address] = value;
    return;
  }

  // Echo RAM
  if ((0xE000 <= address) && (address < 0xFE00)) {
    memory[address - 0x1000] = value;
    return;
  }

  // OAM
  if ((0xFE00 <= address) && (address < 0xFEA0)) {
    ppu->write(address, value);
    return;
  }

  // Not Usable
  if ((0xFEA0 <= address) && (address < 0xFF00)) {
    return;
  }

  // Timer I/O
  if (address >= 0xFF04 && address <= 0xFF07) {
    timer->write(address, value);
    return;
  }

  // PPU I/O
  if (address >= 0xFF40 && address <= 0xFF4B) {
    ppu->write(address, value);
    return;
  }

  // IF
  if (address == 0xFF0F) {
    memory[address] = value & 0x1F;
    return;
  }

  // IE
  if (address == 0xFFFF) {
    memory[address] = value & 0x1F;
    return;
  }

  memory[address] = value;

  // hardcoded Serial to console
  if (address == 0xFF02 && value == 0x81) {
    // return;

    std::cout << static_cast<char>(memory[0xFF01]);
    std::cout.flush();
  }
}

uint16_t Memory::read16(uint16_t address) const {
  uint16_t low_byte = static_cast<uint16_t>(read(address));
  uint16_t high_byte = static_cast<uint16_t>(read(address + 1));

  return (high_byte << 8) | low_byte;
}

void Memory::write16(uint16_t address, uint16_t value) {
  write(address, static_cast<uint8_t>(value & 0xFF));
  write(address + 1, static_cast<uint8_t>(value >> 8));
}

void Memory::setTimer(Timer *timer) { this->timer = timer; }

void Memory::setPPU(PPU *ppu) { this->ppu = ppu; }

void Memory::hexDump(const std::string &filename) const {
  std::ofstream file(filename);

  if (!file) {
    std::cerr << "Failed to open hex dump file: " << filename << '\n';
    return;
  }

  for (uint32_t address = 0; address < memory.size(); address += 16) {
    // Address
    file << std::uppercase << std::hex << std::setw(4) << std::setfill('0')
         << address << "  ";

    // Hex bytes
    for (uint32_t i = 0; i < 16; ++i) {
      if (address + i < memory.size()) {
        file << std::setw(2) << static_cast<int>(memory[address + i]) << ' ';
      }
    }

    // ASCII representation
    file << " |";

    for (uint32_t i = 0; i < 16; ++i) {
      if (address + i >= memory.size())
        break;

      uint8_t value = memory[address + i];

      if (value >= 0x20 && value <= 0x7E)
        file << static_cast<char>(value);
      else
        file << '.';
    }

    file << "|\n";
  }
}
