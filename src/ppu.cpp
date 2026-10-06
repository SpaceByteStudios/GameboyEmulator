#include <cstdint>
#include <vector>

#include "memory.h"
#include "ppu.h"

PPU::PPU(Memory &memory)
    : memory(memory), vram(0x2000, 0), oam(0xA0, 0), screen(160 * 144, 0),
      oam_search() {}

void PPU::tick(uint8_t cycles) {
  bool lcd_on = LCDC & 0x80;

  if (!lcd_on) {
    return;
  }

  for (uint8_t i = 0; i < cycles * 4; ++i) {
    updateMode();

    if (ppu_mode == 2 && dots_amount == 0) {
      oamSearch();
    }

    if (ppu_mode == 3) {
      // Mode 3 starts at dot 80.
      uint16_t x = dots_amount - 80;

      if (x < 160 && LY < 144) {
        drawBackgroundPixel(static_cast<uint8_t>(x), LY);
      }
    }

    dots_amount += 1;

    if (dots_amount >= 456) {
      dots_amount = 0;

      LY += 1;

      if (LY >= 154) {
        LY = 0;
        window_line = 0;
        frame_count += 1;
      }

      window_was_drawn = false;
      mode3_penalty = 0;
    }
  }
}

uint8_t PPU::read(uint16_t address) const {
  bool lcd_on = LCDC & 0x80;

  // VRAM
  if ((0x8000 <= address) && (address < 0xA000)) {
    if (ppu_mode == 3 && lcd_on) {
      return 0xFF;
    }

    return vram[address - 0x8000];
  }

  // OAM
  if ((0xFE00 <= address) && (address < 0xFEA0)) {
    if ((ppu_mode == 2 || ppu_mode == 3) && lcd_on) {
      return 0xFF;
    }

    return oam[address - 0xFE00];
  }

  // PPU I/O
  if (address >= 0xFF40 && address <= 0xFF4B) {
    switch (address) {
    case 0xFF40:
      return LCDC;
    case 0xFF41:
      return STAT;
    case 0xFF42:
      return SCY;
    case 0xFF43:
      return SCX;
    case 0xFF44:
      return LY;
    case 0xFF45:
      return LYC;
    case 0xFF46:
      return DMA;
    case 0xFF47:
      return BGP;
    case 0xFF48:
      return OBP0;
    case 0xFF49:
      return OBP1;
    case 0xFF4A:
      return WY;
    case 0xFF4B:
      return WX;
    default:
      return 0xFF;
    }
  }

  return 0xFF;
}

void PPU::write(uint16_t address, uint8_t value) {
  bool lcd_on = LCDC & 0x80;

  // VRAM
  if ((0x8000 <= address) && (address < 0xA000)) {
    if (ppu_mode == 3 && lcd_on) {
      return;
    }

    vram[address - 0x8000] = value;
  }

  // OAM
  if ((0xFE00 <= address) && (address < 0xFEA0)) {
    if ((ppu_mode == 2 || ppu_mode == 3) && lcd_on) {
      return;
    }

    oam[address - 0xFE00] = value;
  }

  // PPU I/O
  if (address >= 0xFF40 && address <= 0xFF4B) {
    switch (address) {
    case 0xFF40:
      LCDC = value;
      break;
    case 0xFF41:
      STAT = value & 0x78;
      break;
    case 0xFF42:
      SCY = value;
      break;
    case 0xFF43:
      SCX = value;
      break;
    case 0xFF44:
      break;
    case 0xFF45:
      LYC = value;
      break;
    case 0xFF46:
      DMA = value;
      break;
    case 0xFF47:
      BGP = value;
      break;
    case 0xFF48:
      OBP0 = value;
      break;
    case 0xFF49:
      OBP1 = value;
      break;
    case 0xFF4A:
      WY = value;
      break;
    case 0xFF4B:
      WX = value;
      break;
    }
  }
}

std::vector<uint8_t> PPU::get_screen() { return screen; }

void PPU::updateMode() {
  if (LY >= 144) {
    if (ppu_mode != 1) {
      uint8_t IF = memory.read(0xFF0F);
      memory.write(0xFF0F, IF | 0x01);
    }

    ppu_mode = 1;
    return;
  }

  if (dots_amount < 80) {
    ppu_mode = 2;
  } else if (dots_amount < 252) {
    ppu_mode = 3;
  } else {
    ppu_mode = 0;
  }
}

void PPU::oamSearch() {
  oam_search.clear();
  const bool size_is_big = LCDC & 0x4;
  const uint8_t height = size_is_big ? 16 : 8;

  for (int i = 0; i < 40; i++) {
    if (oam_search.size() >= 10) {
      break;
    }

    const uint16_t address = 4 * i;
    const uint8_t obj_y = oam[address];

    if (LY + 16 >= obj_y && LY + 16 < obj_y + height) {
      oam_search.push_back(address);
    }
  }
}

void PPU::drawPixel() {
  drawBackgroundPixel(current_x, LY);
  return;

  const uint8_t x = current_x;
  const uint8_t y = LY;

  bool window_enabled = LCDC & 0x20;

  bool drawing_window = window_enabled && y >= WY && x + 7 >= WX;

  if (drawing_window) {
    drawWindowPixel(x, y);
    window_was_drawn = true;
  } else {
    drawBackgroundPixel(x, y);
  }
}

void PPU::drawBackgroundPixel(uint8_t x, uint8_t y) {
  if (x >= 160 || y >= 144) {
    return;
  }

  // BG disabled.
  if (!(LCDC & 0x01)) {
    screen[y * 160 + x] = 0;
    return;
  }

  // Position in the 256x256 background.
  const uint16_t bg_x = (static_cast<uint16_t>(x) + SCX) & 0xFF;
  const uint16_t bg_y = (static_cast<uint16_t>(y) + SCY) & 0xFF;

  // Tile coordinate.
  const uint8_t tile_x = bg_x >> 3;
  const uint8_t tile_y = bg_y >> 3;

  // Pixel coordinate within tile.
  const uint8_t pixel_x = bg_x & 0x7;
  const uint8_t pixel_y = bg_y & 0x7;

  const uint16_t tile_map = (LCDC & 0x08) ? 0x1C00 : 0x1800;
  const uint16_t tile_map_offset = tile_map + tile_y * 32 + tile_x;
  const uint8_t tile_number = vram[tile_map_offset];

  uint16_t tile_offset;

  if (LCDC & 0x10) {
    // $8000 addressing mode.
    tile_offset = static_cast<uint16_t>(tile_number) * 16;
  } else {
    // $8800 addressing mode.
    const int8_t signed_tile = static_cast<int8_t>(tile_number);

    tile_offset = static_cast<uint16_t>(0x1000 + signed_tile * 16);
  }

  // Two bytes per tile row.
  const uint16_t row_offset = tile_offset + pixel_y * 2;

  const uint8_t low = vram[row_offset];

  const uint8_t high = vram[row_offset + 1];

  // Leftmost pixel is bit 7.
  const uint8_t bit = 7 - pixel_x;

  const uint8_t color = ((low >> bit) & 1) | (((high >> bit) & 1) << 1);

  // Apply BGP.
  const uint8_t shade = (BGP >> (color * 2)) & 0x3;

  screen[y * 160 + x] = shade;
}

void PPU::drawWindowPixel(uint8_t x, uint8_t y) {
  if (x >= 160 || y >= 144) {
    return;
  }

  const uint16_t window_x = static_cast<uint16_t>(x) - (WX - 7);

  const uint16_t window_y = window_line;

  const uint8_t tile_x = window_x >> 3;
  const uint8_t tile_y = window_y >> 3;

  const uint8_t pixel_x = window_x & 7;
  const uint8_t pixel_y = window_y & 7;

  const uint16_t tile_map = (LCDC & 0x40) ? 0x1C00 : 0x1800;

  const uint16_t tile_map_address = tile_map + tile_y * 32 + tile_x;

  const uint8_t tile_number = vram[tile_map_address];

  uint16_t tile_address;

  if (LCDC & 0x10) {
    tile_address = static_cast<uint16_t>(tile_number) * 16;
  } else {
    const int8_t signed_tile = static_cast<int8_t>(tile_number);

    tile_address = static_cast<uint16_t>(0x1000 + signed_tile * 16);
  }

  const uint16_t row_address = tile_address + pixel_y * 2;

  const uint8_t low = vram[row_address];

  const uint8_t high = vram[row_address + 1];

  const uint8_t bit = 7 - pixel_x;

  const uint8_t color = ((low >> bit) & 1) | (((high >> bit) & 1) << 1);

  const uint8_t shade = (BGP >> (color * 2)) & 3;

  screen[y * 160 + x] = shade;
}

uint64_t PPU::getFrameCount() const { return frame_count; }
