#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "cartridge.h"
#include "ppu.h"
#include "timer.h"

class Memory {
public:
  Memory(const std::string &path);

  uint8_t read(uint16_t address) const;
  void write(uint16_t address, uint8_t value);

  uint16_t read16(uint16_t address) const;
  void write16(uint16_t address, uint16_t value);

  void setTimer(Timer *timer);
  void setPPU(PPU *ppu);

  void hexDump(const std::string &filename) const;

private:
  std::vector<uint8_t> memory;

  Cartridge cartridge;

  Timer *timer = nullptr;
  PPU *ppu = nullptr;
};
