#include <algorithm>
#include <cstdint>
#include <vector>

#include "memory.h"
#include "ppu.h"

PPU::PPU(Memory &memory)
    : memory(memory), vram(0x2000, 0), oam(0xA0, 0), screen(160 * 144, 0),
      oam_search() {}

void PPU::tick(uint8_t cycles) {
  const bool lcd_on = LCDC & 0x80;

  if (memory.oam_dma_running) {
    oam_dma_transfer_timer -= cycles;

    if (oam_dma_transfer_timer <= 0) {
      oam_dma_transfer_timer = 0;
      memory.oam_dma_running = false;
    }
  }

  if (!lcd_on) {
    return;
  }

  for (uint8_t i = 0; i < cycles * 4; ++i) {

    updateMode();
    updateInterrupts();

    if (ppu_mode == 2 && dots_amount == 0) {
      oamSearch();
    }

    if (ppu_mode == 3) {
      // Mode 3 starts at dot 80.
      current_x = dots_amount - 80;

      if (current_x < 160 && LY < 144) {
        drawPixel();
      }
    }

    dots_amount += 1;

    if (dots_amount >= 456) {
      dots_amount = 0;

      LY += 1;

      if (window_drawn) {
        window_line += 1;
        window_drawn = false;
      }

      if (LY >= 154) {
        LY = 0;
        window_line = 0;
        frame_count += 1;
      }
    }
  }
}

uint8_t PPU::read(uint16_t address) const {
  const bool lcd_on = LCDC & 0x80;

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
  const bool lcd_on = LCDC & 0x80;

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
      runOamDmaTransfer();
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
    ppu_mode = 1;
  } else if (dots_amount < 80) {
    ppu_mode = 2;
  } else if (dots_amount < 252) {
    ppu_mode = 3;
  } else {
    ppu_mode = 0;
  }

  // Update ppu_mode
  STAT = (STAT & 0xFC) + ppu_mode;
}

void PPU::updateInterrupts() {
  if (LY == 144) {
    uint8_t IF = memory.read(0xFF0F);
    memory.write(0xFF0F, IF | 0x01);
  }

  // Update LYC == LY
  bool lyc_equals_ly = LYC == LY;
  STAT = (STAT & 0xFB) + (lyc_equals_ly << 2);

  bool stat_interrupt =
      (lyc_equals_ly && (STAT & 0x40)) | (ppu_mode == 2 && (STAT & 0x20)) |
      (ppu_mode == 1 && (STAT & 0x10)) | (ppu_mode == 0 && (STAT & 0x08));

  if (stat_interrupt && !previous_stat_interrupt) {
    uint8_t IF = memory.read(0xFF0F);
    memory.write(0xFF0F, IF | 0x02);
  }

  previous_stat_interrupt = stat_interrupt;
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

void PPU::runOamDmaTransfer() {
  uint16_t source = std::min<uint16_t>(DMA, 0xDF) << 8;

  for (int i = 0; i < 160; i++) {
    oam[i] = memory.read(source + i);
  }

  memory.oam_dma_running = true;
  oam_dma_transfer_timer = 160;
}

void PPU::drawPixel() {
  const uint8_t x = current_x;
  const uint8_t y = LY;

  if (x >= 160 || y >= 144) {
    return;
  }

  bool window_enabled = LCDC & 0x20;
  bool drawing_window = window_enabled && y >= WY && x + 7 >= WX;

  uint8_t screen_color = 0;

  if (drawing_window) {
    screen_color = getWindowPixel(x, y);
  } else {
    screen_color = getBackgroundPixel(x, y);
  }

  bool sprites_enabled = LCDC & 0x02;
  if (sprites_enabled) {
    screen_color = getSpritePixel(x, y, screen_color);
  }

  screen[y * 160 + x] = screen_color;
}

uint8_t PPU::getBackgroundPixel(uint8_t x, uint8_t y) {
  // BG disabled.
  if (!(LCDC & 0x01)) {
    return 0;
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
  const uint8_t shade = (BGP >> (color * 2)) & 0x3;

  return shade;
}

uint8_t PPU::getWindowPixel(uint8_t x, uint8_t y) {
  window_drawn = true;

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

  return shade;
}

// TODO
// 8*16 Pixel Sprites working

uint8_t PPU::getSpritePixel(uint8_t x, uint8_t y, uint8_t screen_color) {
  std::vector<uint8_t> sprites;

  // Get relevant Sprites
  for (int i = 0; i < oam_search.size(); i++) {
    uint8_t obj_adress = oam_search[i];

    uint8_t sprite_x = oam[obj_adress + 1];
    uint8_t sprite_y = oam[obj_adress];

    const bool size_is_big = LCDC & 0x4;
    const uint8_t height = size_is_big ? 16 : 8;

    if (y + 16 >= sprite_y && y + 16 < sprite_y + height) {
      if (x + 8 >= sprite_x && x + 8 < sprite_x + 8) {
        sprites.push_back(oam_search[i]);
      }
    }
  }

  // Filter by Priority
  if (screen_color > 0) {
    for (int i = sprites.size() - 1; i >= 0; i--) {
      bool priority = oam[sprites[i] + 3] & 0x80;

      if (priority) {
        sprites.pop_back();
      }
    }
  }

  if (sprites.size() == 0) {
    return screen_color;
  }

  std::sort(sprites.begin(), sprites.end(),
            [this](uint8_t a, uint8_t b) { return oam[a + 1] < oam[b + 1]; });

  for (int i = 0; i < sprites.size(); i++) {
    const uint8_t sprite = sprites[i];

    uint8_t palette = OBP0;
    bool flip_x = oam[sprite + 3] & 0x20;
    bool flip_y = oam[sprite + 3] & 0x40;

    if (oam[sprite + 3] & 0x10) {
      palette = OBP1;
    }

    const uint8_t sprite_x = oam[sprite + 1];
    const uint8_t sprite_y = oam[sprite];

    const uint8_t pixel_x = x + 8 - sprite_x;
    const uint8_t pixel_y = y + 16 - sprite_y;

    const bool size_is_big = LCDC & 0x4;

    const uint8_t tile_number = oam[sprite + 2];
    uint16_t tile_address = static_cast<uint16_t>(tile_number) * 16;

    if (size_is_big) {
      tile_address = (static_cast<uint16_t>(tile_number) * 16) & 0xFE;
    }

    uint16_t row_address = tile_address + pixel_y * 2;
    if (flip_y && !size_is_big) {
      row_address = tile_address + (14 - pixel_y * 2);
    } else if (flip_y && size_is_big) {
      row_address = tile_address + (30 - pixel_y * 2);
    }

    const uint8_t low = vram[row_address];
    const uint8_t high = vram[row_address + 1];

    uint8_t bit = 7 - pixel_x;
    if (flip_x) {
      bit = pixel_x;
    }

    const uint8_t color = ((low >> bit) & 1) | (((high >> bit) & 1) << 1);
    const uint8_t shade = (palette >> (color * 2)) & 3;

    if (color == 0) {
      continue;
    } else {
      return shade;
    }
  }

  return screen_color;
}

uint64_t PPU::getFrameCount() const { return frame_count; }
