#include "gameboy.h"
#include "cpu.h"
#include "ppu.h"
#include "timer.h"

#include <cstdint>
#include <memory.h>
#include <memory>

uint8_t tiles[][16] = {
    // Tile 0: Color 0
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    },

    // Tile 1: Color 1
    {
        0xFF,
        0x00,
        0xFF,
        0x00,
        0xFF,
        0x00,
        0xFF,
        0x00,
        0xFF,
        0x00,
        0xFF,
        0x00,
        0xFF,
        0x00,
        0xFF,
        0x00,
    },

    // Tile 2: Color 2
    {
        0x00,
        0xFF,
        0x00,
        0xFF,
        0x00,
        0xFF,
        0x00,
        0xFF,
        0x00,
        0xFF,
        0x00,
        0xFF,
        0x00,
        0xFF,
        0x00,
        0xFF,
    },

    // Tile 3: Color 3
    {
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
        0xFF,
    },

    // Tile 4: Smiley
    {
        0x00,
        0x00,
        0x24,
        0x24,
        0x00,
        0x24,
        0x00,
        0x00,
        0x42,
        0x42,
        0x7E,
        0x3C,
        0x3C,
        0x00,
        0x00,
        0x00,
    },

    // Tile 5: Triangle
    {
        0x18,
        0x18,
        0x24,
        0x24,
        0x24,
        0x24,
        0x42,
        0x42,
        0x02,
        0x40,
        0x01,
        0x80,
        0x01,
        0x80,
        0x0F,
        0xF0,
    },

    // Tile 6: Box
    {
        0xFF,
        0xFF,
        0x81,
        0x81,
        0x81,
        0x81,
        0x81,
        0x81,
        0x81,
        0x81,
        0x81,
        0x81,
        0x81,
        0x81,
        0xFF,
        0xFF,
    },
};

Gameboy::Gameboy(const std::string &path)
    : memory(path), cpu(memory), timer(std::make_unique<Timer>(memory)),
      ppu(std::make_unique<PPU>(memory)) {

  memory.setTimer(timer.get());
  memory.setPPU(ppu.get());
}

void Gameboy::step() {
  // cycles are in M Cycles
  uint8_t cycles = cpu.step();

  timer->tick(cycles);
  ppu->tick(cycles);
}

void Gameboy::runFrame() {
  const uint64_t start_frame = ppu->getFrameCount();
  int steps = 0;

  while (ppu->getFrameCount() == start_frame) {
    if (steps >= 100000) {
      break;
    }

    step();
    steps += 1;
  }
}

void Gameboy::test_screen() {
  // LCD off
  memory.write(0xFF40, 0x00);

  // No scrolling
  memory.write(0xFF42, 0x0);
  memory.write(0xFF43, 0x0);

  // BGP palette
  memory.write(0xFF47, 0xE4);

  // OBP palettes
  memory.write(0xFF48, 0xE4);
  memory.write(0xFF49, 0xE4);

  // Write tiles.
  for (int tile_id = 0; tile_id < 7; tile_id++) {
    for (int i = 0; i < 16; i++) {
      memory.write(0x8000 + tile_id * 16 + i, tiles[tile_id][i]);
    }
  }

  // Write tile map.
  for (int y = 0; y < 32; y++) {
    for (int x = 0; x < 32; x++) {
      uint8_t tile_id = (x + y) & 3;
      tile_id = x % 4;
      tile_id = 0;

      memory.write(0x9800 + y * 32 + x, tile_id);
    }
  }

  // Write OAM Box
  memory.write(0xFE00, 0x18);
  memory.write(0xFE01, 0x10);
  memory.write(0xFE02, 0x06);
  memory.write(0xFE03, 0x00);

  // Write OAM Triangle
  memory.write(0xFE04, 0x1B);
  memory.write(0xFE05, 0x19);
  memory.write(0xFE06, 0x04);
  memory.write(0xFE07, 0x00);

  // LCD on
  memory.write(0xFF40, 0x93);
}

std::vector<uint8_t> Gameboy::getScreen() { return ppu->get_screen(); }

void Gameboy::updateJoypadInput() { memory.updateJoypadInput(); }

void Gameboy::hexDump(const std::string &filename) { memory.hexDump(filename); }
