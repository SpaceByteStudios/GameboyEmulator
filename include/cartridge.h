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

private:
  const std::vector<uint8_t> rom;
  std::vector<uint8_t> ram;
};
