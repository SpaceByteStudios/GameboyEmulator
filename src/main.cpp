#include <iostream>

#include <SDL3/SDL.h>

#include "gameboy.h"
#include "renderer.h"

int main() {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    std::cerr << "SDL_Init failed: " << SDL_GetError() << '\n';
    return 1;
  }

  Renderer renderer = Renderer();
  Gameboy gameboy = Gameboy();

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
