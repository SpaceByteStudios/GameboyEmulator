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
  uint64_t getFrameCount() const;

private:
  Memory &memory;

  std::vector<uint8_t> vram;
  std::vector<uint8_t> oam;

  // 0-3, lightest to darkest
  std::vector<uint8_t> screen;

  uint64_t frame_count = 0;

  // Mode 0 HBlank
  // Mode 1 VBlank
  // Mode 2 OAM Scan
  // Mode 3 Drawing
  uint8_t ppu_mode = 2;

  // Drawing Variables
  uint8_t current_x = 0;
  uint16_t dots_amount = 0;
  uint8_t mode3_penalty = 0;
  std::vector<uint8_t> oam_search;

  uint8_t LCDC = 0;
  uint8_t STAT = 0;

  uint8_t SCY = 0;
  uint8_t SCX = 0;

  uint8_t LY = 0;
  uint8_t LYC = 0;

  uint8_t DMA = 0;

  uint8_t BGP = 0;
  uint8_t OBP0 = 0;
  uint8_t OBP1 = 0;

  uint8_t WY = 0;
  uint8_t WX = 0;

  uint8_t window_line = 0;
  bool window_drawn = false;

  void updateMode();
  void oamSearch();

  void drawPixel();
  uint8_t getBackgroundPixel(uint8_t x, uint8_t y);
  uint8_t getWindowPixel(uint8_t x, uint8_t y);
  uint8_t getSpritePixel(uint8_t x, uint8_t y, uint8_t screen_color);
};
