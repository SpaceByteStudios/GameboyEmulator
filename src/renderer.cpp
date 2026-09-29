#include <cstdint>
#include <stdexcept>
#include <vector>

#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"

#include "gameboy.h"
#include "renderer.h"

Renderer::Renderer(Gameboy &gameboy) : gameboy(gameboy) {

  pallette = {0xFFE0F8D0, 0xFF88C070, 0xFF346856, 0xFF081820};

  if (!SDL_CreateWindowAndRenderer("Game Boy Emulator", 160 * 4, 144 * 4, 0,
                                   &window, &renderer)) {
    SDL_Quit();
    throw std::runtime_error("SDL_CreateWindowAndRenderer failed");
  }

  screen_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                     SDL_TEXTUREACCESS_STREAMING, 160, 144);
}

Renderer::~Renderer() {
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
}

void Renderer::draw() {
  // Clear screen
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);

  std::vector<uint8_t> screen = gameboy.getScreen();
  std::vector<uint32_t> pixels(160 * 144);

  for (int i = 0; i < screen.size(); i++) {
    pixels[i] = pallette[screen[i] & 0x03];
  }

  SDL_UpdateTexture(screen_texture, nullptr, pixels.data(),
                    160 * sizeof(uint32_t));

  SDL_FRect dst{0, 0, 160 * 4, 144 * 4};

  // Present frame
  SDL_RenderTexture(renderer, screen_texture, nullptr, &dst);
  SDL_RenderPresent(renderer);
}
