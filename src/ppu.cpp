#include <vector>

#include "memory.h"
#include "ppu.h"

PPU::PPU(Memory &memory)
    : memory(memory), vram(0x2000), oam(0x9F), screen(0x5A00), oam_search(10) {}

void PPU::tick(uint8_t cycles) {
  for (uint8_t i = 0; i < cycles * 4; i++) {
    // Do OAM Search
    if ((ppu_mode == 2) && (dots_amount == 0)) {
      oamSearch();
    }

    // Drawing Mode
    if ((ppu_mode == 3) && (current_x >= 160)) {
      // Draw Pixels
    }

    dots_amount++;

    if (dots_amount >= 456) {
      dots_amount = 0;
      LY += 1;

      if (LY >= 154) {
        LY = 0;
      }
    }

    updateMode();
  }
}

uint8_t PPU::read(uint16_t address) const {
  // VRAM
  if ((0x8000 <= address) && (address < 0xA000)) {
    if (ppu_mode == 3) {
      return 0xFF;
    }

    return vram[address - 0x8000];
  }

  // OAM
  if ((0xFE00 <= address) && (address < 0xFEA0)) {
    if (ppu_mode == 2 || ppu_mode == 3) {
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
  // VRAM
  if ((0x8000 <= address) && (address < 0xA000)) {
    if (ppu_mode == 3) {
      return;
    }

    vram[address - 0x8000] = value;
  }

  // OAM
  if ((0xFE00 <= address) && (address < 0xFEA0)) {
    if (ppu_mode == 2 || ppu_mode == 3) {
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

std::vector<uint8_t> PPU::get_screen() {
  std::vector<uint8_t> new_screen(160 * 144);

  new_screen[0] = 0;
  new_screen[1] = 1;
  new_screen[2] = 2;
  new_screen[3] = 3;

  new_screen[159] = 3;
  new_screen[160] = 3;

  return new_screen;
}

void PPU::updateMode() {
  if ((144 <= LY) && (LY < 154)) {
    // Request VBlank Interrupt
    if (ppu_mode != 1) {
      uint8_t IF = memory.read(0xFF0F);
      memory.write(0xFF0F, IF | 0x01);
    }

    ppu_mode = 1;
  } else {
    if (dots_amount < 80) {
      ppu_mode = 2;
    } else if (dots_amount - 80 < 172 + mode3_penalty) {
      ppu_mode = 3;
    } else {
      ppu_mode = 0;
    }
  }
}

void PPU::oamSearch() {
  // Do OAM Search
}
