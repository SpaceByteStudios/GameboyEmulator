#include <iostream>

#include <SDL3/SDL.h>

#include "gameboy.h"
#include "renderer.h"

int main() {
  Gameboy gameboy = Gameboy("./roms/test_roms/11-op a,(hl).gb");

  while (!gameboy.is_halted()) {
    gameboy.run();
  }

  gameboy.hexDump("memory_dump.txt");

  return 0;

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    std::cerr << "SDL_Init failed: " << SDL_GetError() << '\n';
    return 1;
  }

  Renderer renderer = Renderer();

  bool running = true;

  while (running) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT)
        running = false;
    }

    renderer.draw();

    SDL_Delay(16);
  }

  SDL_Quit();
  return 0;
}
