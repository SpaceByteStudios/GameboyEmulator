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
    }};

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

  while (ppu->getFrameCount() == start_frame) {
    step();
  }
}

void Gameboy::test_screen() {
  // LCD on
  memory.write(0xFF40, 0x91);

  // No scrolling
  memory.write(0xFF42, 0x0);
  memory.write(0xFF43, 0x0);

  // Normal palette
  memory.write(0xFF47, 0xE4);

  // Write tiles.
  for (int tile_id = 0; tile_id < 4; tile_id++) {
    for (int i = 0; i < 16; i++) {
      memory.write(0x8000 + tile_id * 16 + i, tiles[tile_id][i]);
    }
  }

  // Write tile map.
  for (int y = 0; y < 32; y++) {
    for (int x = 0; x < 32; x++) {
      uint8_t tile_id = (x + y) & 3;

      memory.write(0x9800 + y * 32 + x, tile_id);
    }
  }

  // Write tile map 2.
  for (int y = 0; y < 32; y++) {
    for (int x = 0; x < 32; x++) {
      uint8_t tile_id = x % 4;

      memory.write(0x9C00 + y * 32 + x, tile_id);
    }
  }
}

std::vector<uint8_t> Gameboy::getScreen() { return ppu->get_screen(); }

void Gameboy::hexDump(const std::string &filename) const {
  memory.hexDump(filename);
}
