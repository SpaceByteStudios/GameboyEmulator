#include <stdexcept>

#include "gameboy.h"
#include "renderer.h"

Renderer::Renderer(Gameboy &gameboy) : gameboy(gameboy) {
  if (!SDL_CreateWindowAndRenderer("Game Boy Emulator", 800, 720, 0, &window,
                                   &renderer)) {
    SDL_Quit();
    throw std::runtime_error("SDL_CreateWindowAndRenderer failed");
  }
}

Renderer::~Renderer() {
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
}

void Renderer::draw() {
  // Clear screen
  SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
  SDL_RenderClear(renderer);

  // Present frame
  SDL_RenderPresent(renderer);
}
