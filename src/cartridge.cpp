#include <cstdint>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>

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

  int rom_real_size = 32768 * (0x1 << rom_size);
  int ram_real_size = 0;

  switch (ram_size) {
  case 2:
    ram_real_size = 8192;
    break;
  case 3:
    ram_real_size = 32768;
    break;
  case 4:
    ram_real_size = 32768 * 4;
  case 5:
    ram_real_size = 32768 * 2;
  }

  std::cout << "Cartridge type: " << std::hex
            << static_cast<int>(cartridge_type) << std::endl;

  std::cout << "ROM size: " << std::dec << rom_real_size << std::endl;
  std::cout << "RAM size: " << std::dec << ram_real_size << std::endl;
}

uint8_t Cartridge::read(uint16_t address) const {}

void Cartridge::write(uint16_t address, uint8_t value) {}
