#include "gameboy.h"
#include "cpu.h"
#include <memory.h>

Gameboy::Gameboy() : memory(), cpu(memory) { running = true; }

void Gameboy::reset() {}

void Gameboy::run() {}

void Gameboy::draw() {}

bool Gameboy::is_running() { return running; }
