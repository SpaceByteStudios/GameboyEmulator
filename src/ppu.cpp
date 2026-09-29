#include "ppu.h"

PPU::PPU(Memory &memory)
    : memory(memory), vram(0x2000), oam(0x9F), screen(0x5A00) {}

void PPU::tick(uint8_t cycles) {}

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

std::vector<uint8_t> PPU::get_screen() { return screen; }
