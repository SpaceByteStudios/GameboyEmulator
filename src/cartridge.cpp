#include <cstdint>
#include <fstream>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cartridge.h"

Cartridge::Cartridge(const std::string &path) {
  std::ifstream file(path, std::ios::binary);

  if (!file) {
    throw std::runtime_error("Failed to open ROM: " + path);
  }

  uint8_t cartridge_type;
  uint8_t rom_size;
  uint8_t ram_size;

  file.seekg(0x147);
  file.read(reinterpret_cast<char *>(&cartridge_type), 1);
  file.read(reinterpret_cast<char *>(&rom_size), 1);
  file.read(reinterpret_cast<char *>(&ram_size), 1);

  bool is_supported = cartridge_type <= 0x03;

  if (!is_supported) {
    std::cout << "Currently only ROM + RAM or MBC1 supported!" << std::endl;
    throw std::runtime_error("Cartridge Type not supported: " +
                             std::to_string(cartridge_type));
  }

  int rom_real_size = 32768 * (0x1 << rom_size);
  int ram_real_size = 0;

  switch (ram_size) {
  case 2:
    ram_real_size = 8 * 1024;
    break;
  case 3:
    ram_real_size = 32 * 1024;
    break;
  case 4:
    ram_real_size = 128 * 1024;
    break;
  case 5:
    ram_real_size = 64 * 1024;
    break;
  }

  rom = std::vector<uint8_t>(rom_real_size, 0);
  ram = std::vector<uint8_t>(ram_real_size, 0);

  file.seekg(0x0);
  file.read(reinterpret_cast<char *>(rom.data()), rom.size());

  ram_enable = 0x00;
  rom_bank_number = 0x01;
  ram_bank_number = 0x00;
  banking_mode = 0x00;
}

uint8_t Cartridge::read(uint16_t address) const {
  // ROM read
  if (address < 0x8000) {
    uint32_t bank;

    if (address < 0x4000) {
      bank = (banking_mode == 0) ? 0 : (ram_bank_number << 5);
    } else {
      bank = rom_bank_number | (ram_bank_number << 5);

      if ((bank & 0x1F) == 0)
        bank++;
    }

    uint32_t offset = bank * 0x4000;

    if (address < 0x4000)
      offset += address;
    else
      offset += address - 0x4000;

    return rom[offset];
  }

  // RAM read
  if ((0xA000 <= address) && (address < 0xC000)) {
    if (!ram_enable) {
      return 0xFF;
    }

    uint32_t bank = 0;

    if (banking_mode == 1) {
      bank = ram_bank_number;
    }

    uint32_t offset = bank * 0x2000 + (address - 0xA000);

    return ram[offset];
  }

  return 0xFF;
}

void Cartridge::write(uint16_t address, uint8_t value) {
  // ROM write
  if (address < 0x8000) {
    if (address < 0x2000) {
      ram_enable = ((value & 0x0F) == 0x0A);
    }

    else if ((0x2000 <= address) && (address < 0x4000)) {
      rom_bank_number = value & 0x1F;

      if (rom_bank_number == 0x00) {
        rom_bank_number = 0x01;
      }
    }

    else if ((0x4000 <= address) && (address < 0x6000)) {
      ram_bank_number = value & 0x03;
    }

    else if ((0x6000 <= address) && (address < 0x8000)) {
      banking_mode = value & 0x01;
    }
  }

  // RAM write
  if ((0xA000 <= address) && (address < 0xC000)) {
    if (!ram_enable) {
      return;
    }

    uint32_t bank = 0;

    if (banking_mode == 1) {
      bank = ram_bank_number;
    }

    uint32_t offset = bank * 0x2000 + (address - 0xA000);

    ram[offset] = value;
  }
}
