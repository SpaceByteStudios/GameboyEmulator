#include "memory.h"
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

Memory::Memory() = default;

uint8_t Memory::read(uint16_t address) const {
  // hardcoded VBlank
  if (address == 0xFF44) {
    return 0x90;
  }

  // IF
  if (address == 0xFF0F) {
    return memory[address] & 0x1F;
  }

  // IE
  if (address == 0xFFFF) {
    return memory[address] & 0x1F;
  }

  return memory[address];
}

void Memory::write(uint16_t address, uint8_t value) {
  // ROM Area
  if (address < 0x8000) {
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
    return;

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

void Memory::loadROM(const std::string &path) {
  std::ifstream file(path, std::ios::binary);

  if (!file) {
    throw std::runtime_error("Failed to open ROM: " + path);
  }

  file.read(reinterpret_cast<char *>(memory.data()), 0x8000);
}

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
