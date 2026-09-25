#pragma once

#include <cstdint>
#include <string>
#include <vector>

// Currently only MBC1

class Cartridge {
public:
  Cartridge(const std::string &path);

  uint8_t read(uint16_t address) const;
  void write(uint16_t address, uint8_t value);

  uint32_t get_lower_rom_bank() const;
  uint32_t get_upper_rom_bank() const;

private:
  std::vector<uint8_t> rom;
  std::vector<uint8_t> ram;

  // MBC1 Registers
  uint8_t ram_enable;
  uint8_t rom_bank_number;
  uint8_t ram_bank_number;
  uint8_t banking_mode;
};
