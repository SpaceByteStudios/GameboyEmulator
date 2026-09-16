#include "gameboy.h"

Gameboy::Gameboy() {
  cpu = CPU();
  running = true;
}

void Gameboy::reset() {}

void Gameboy::run() {}

void Gameboy::draw() {}

bool Gameboy::is_running() { return running; }
