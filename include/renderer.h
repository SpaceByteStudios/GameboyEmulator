#pragma once

#include <SDL3/SDL.h>
#include <cstdint>
#include <vector>

#include "gameboy.h"

class Renderer {
public:
  Renderer(Gameboy &gameboy);
  ~Renderer();

  void draw();

private:
  SDL_Window *window = nullptr;
  SDL_Renderer *renderer = nullptr;
  SDL_Texture *screen_texture = nullptr;

  Gameboy &gameboy;

  std::vector<uint32_t> pallette;
};
