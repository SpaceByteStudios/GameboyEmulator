#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "cartridge.h"
#include "joypad.h"
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

  void updateJoypadInput(Joypad &joypad);

  void hexDump(const std::string &filename);

  bool oam_dma_running;

private:
  std::vector<uint8_t> wram;
  std::vector<uint8_t> hram;

  Cartridge cartridge;

  Timer *timer = nullptr;
  PPU *ppu = nullptr;

  // Down, Up, Left, Right
  // Start, Select, B, A
  uint8_t joypad_input = 0;
  uint8_t joypad_select = 0;

  uint8_t IF;
  uint8_t IE;
};
