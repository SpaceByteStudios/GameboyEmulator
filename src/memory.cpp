#include "memory.h"
#include "cartridge.h"
#include "joypad.h"

#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory.h>

Memory::Memory(const std::string &path)
    : wram(0x2000), hram(0x7E), cartridge(path) {
  oam_dma_running = false;
}

uint8_t Memory::read(uint16_t address) const {
  if (address >= 0xFF00) {
    // HRAM
    if ((0xFF80 <= address) && (address < 0xFFFF)) {
      return hram[address - 0xFF80];
    }

    // Joypad I/O
    if (address == 0xFF00) {
      uint8_t res = 0xC0 | joypad_select | 0x0F;

      // Direction buttons: Down, Up, Left, Right
      if (!(joypad_select & 0x10)) {
        uint8_t directions = (joypad_input >> 4) & 0x0F;
        res &= ~(directions & 0x0F);
      }

      // Action buttons: Start, Select, B, A
      if (!(joypad_select & 0x20)) {
        uint8_t buttons = joypad_input & 0x0F;
        res &= ~(buttons & 0x0F);
      }

      return res;
    }

    // Timer I/O
    if (address >= 0xFF04 && address <= 0xFF07) {
      return timer->read(address);
    }

    // PPU I/O
    if (address >= 0xFF40 && address <= 0xFF4B) {
      return ppu->read(address);
    }

    // IF
    if (address == 0xFF0F) {
      return IF | 0xE0;
    }

    // IE
    if (address == 0xFFFF) {
      return IE;
    }

    return 0xFF;
  }

  if (oam_dma_running) {
    return 0xFF;
  }

  // ROM
  if (address < 0x8000) {
    return cartridge.read(address);
  }

  // VRAM
  if ((0x8000 <= address) && (address < 0xA000)) {
    return ppu->read(address);
  }

  // External RAM
  if ((0xA000 <= address) && (address < 0xC000)) {
    return cartridge.read(address);
  }

  // WRAM
  if ((0xC000 <= address) && (address < 0xE000)) {
    return wram[address - 0xC000];
  }

  // Echo RAM
  if ((0xE000 <= address) && (address < 0xFE00)) {
    return wram[address - 0xE000];
  }

  // OAM
  if ((0xFE00 <= address) && (address < 0xFEA0)) {
    return ppu->read(address);
  }

  // Not Usable
  if ((0xFEA0 <= address) && (address < 0xFF00)) {
    return 0xFF;
  }

  return 0xFF;
}

void Memory::write(uint16_t address, uint8_t value) {
  if (address >= 0xFF00) {
    // HRAM
    if ((0xFF80 <= address) && (address < 0xFFFF)) {
      hram[address - 0xFF80] = value;
      return;
    }

    // Joypad I/O
    if (address == 0xFF00) {
      joypad_select = (value & 0x30);
      return;
    }

    // Timer I/O
    if (address >= 0xFF04 && address <= 0xFF07) {
      timer->write(address, value);
      return;
    }

    // PPU I/O
    if (address >= 0xFF40 && address <= 0xFF4B) {
      ppu->write(address, value);
      return;
    }

    // IF
    if (address == 0xFF0F) {
      IF = value & 0x1F;
      return;
    }

    // IE
    if (address == 0xFFFF) {
      IE = value;
      return;
    }

    return;
  }

  if (oam_dma_running) {
    return;
  }

  // ROM
  if (address < 0x8000) {
    cartridge.write(address, value);
    return;
  }

  // VRAM
  if ((0x8000 <= address) && (address < 0xA000)) {
    ppu->write(address, value);
    return;
  }

  // External RAM
  if ((0xA000 <= address) && (address < 0xC000)) {
    cartridge.write(address, value);
    return;
  }

  // WRAM
  if ((0xC000 <= address) && (address < 0xE000)) {
    wram[address - 0xC000] = value;
    return;
  }

  // Echo RAM
  if ((0xE000 <= address) && (address < 0xFE00)) {
    wram[address - 0xE000] = value;
    return;
  }

  // OAM
  if ((0xFE00 <= address) && (address < 0xFEA0)) {
    ppu->write(address, value);
    return;
  }

  // Not Usable
  if ((0xFEA0 <= address) && (address < 0xFF00)) {
    return;
  }
}

uint16_t Memory::read16(uint16_t address) const {
  uint16_t low_byte = static_cast<uint16_t>(read(address));
  uint16_t high_byte = static_cast<uint16_t>(read(address + 1));

  return (high_byte << 8) | low_byte;
}

void Memory::write16(uint16_t address, uint16_t value) {
  write(address, static_cast<uint8_t>(value & 0xFF));
  write(address + 1, static_cast<uint8_t>(value >> 8));
}

void Memory::setTimer(Timer *timer) { this->timer = timer; }

void Memory::setPPU(PPU *ppu) { this->ppu = ppu; }

void Memory::updateJoypadInput(Joypad &joypad) {
  joypad_input = 0;

  if (joypad.down) {
    joypad_input |= (1 << 7);
  }

  if (joypad.up) {
    joypad_input |= (1 << 6);
  }

  if (joypad.left) {
    joypad_input |= (1 << 5);
  }

  if (joypad.right) {
    joypad_input |= (1 << 4);
  }

  if (joypad.start) {
    joypad_input |= (1 << 3);
  }

  if (joypad.select) {
    joypad_input |= (1 << 2);
  }

  if (joypad.b) {
    joypad_input |= (1 << 1);
  }

  if (joypad.a) {
    joypad_input |= 1;
  }
}

void Memory::hexDump(const std::string &filename) {
  std::ofstream file(filename);

  if (!file) {
    std::cerr << "Failed to open hex dump file: " << filename << '\n';
    return;
  }

  constexpr uint32_t address_space_size = 0x10000;

  uint8_t lcdc = ppu->read(0xFF40);
  ppu->write(0xFF40, 0x00);

  bool transfer_running = oam_dma_running;
  oam_dma_running = false;

  for (uint32_t address = 0; address < address_space_size; address += 16) {

    // Address
    file << std::uppercase << std::hex << std::setw(4) << std::setfill('0')
         << address << "  ";

    // Hex bytes
    for (uint32_t i = 0; i < 16; ++i) {
      uint8_t value = read(static_cast<uint16_t>(address + i));

      file << std::setw(2) << static_cast<int>(value) << ' ';
    }

    // ASCII representation
    file << " |";

    for (uint32_t i = 0; i < 16; ++i) {
      uint8_t value = read(static_cast<uint16_t>(address + i));

      if (value >= 0x20 && value <= 0x7E) {
        file << static_cast<char>(value);
      } else {
        file << '.';
      }
    }

    file << "|\n";
  }

  ppu->write(0xFF40, lcdc);
  oam_dma_running = transfer_running;
}
