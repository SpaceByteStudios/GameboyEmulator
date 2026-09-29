#pragma once

#include <cstdint>
#include <vector>

class Memory;

class PPU {
public:
  PPU(Memory &memory);

  void tick(uint8_t cycles);

  uint8_t read(uint16_t address) const;
  void write(uint16_t address, uint8_t value);

  std::vector<uint8_t> get_screen();

private:
  Memory &memory;

  std::vector<uint8_t> vram;
  std::vector<uint8_t> oam;
  std::vector<uint8_t> screen;

  // Mode 0 HBlank
  // Mode 1 VBlank
  // Mode 2 OAM Scan
  // Mode 3 Drawing
  uint8_t ppu_mode;

  uint8_t LCDC;
  uint8_t STAT;

  uint8_t SCY;
  uint8_t SCX;

  uint8_t LY;
  uint8_t LYC;

  uint8_t DMA;

  uint8_t BGP;
  uint8_t OBP0;
  uint8_t OBP1;

  uint8_t WY;
  uint8_t WX;
};
