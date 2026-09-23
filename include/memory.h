#pragma once

#include <array>
#include <cstdint>
#include <string>

class Memory {
public:
  Memory();

  uint8_t read(uint16_t address) const;
  void write(uint16_t address, uint8_t value);

  uint16_t read16(uint16_t address) const;
  void write16(uint16_t address, uint16_t value);

  void loadROM(const std::string &path);

  void hexDump(const std::string &filename) const;

private:
  std::array<uint8_t, 0x10000> memory{};
};
