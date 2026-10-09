#pragma once

#include <SDL3/SDL.h>
#include <cstdint>
#include <vector>

#include "SDL3/SDL_events.h"
#include "gameboy.h"

class Renderer {
public:
  Renderer(Gameboy &gameboy);
  ~Renderer();

  void draw();
  void updateInput(const SDL_KeyboardEvent &event);

private:
  SDL_Window *window = nullptr;
  SDL_Renderer *renderer = nullptr;
  SDL_Texture *screen_texture = nullptr;

  Gameboy &gameboy;

  std::vector<uint32_t> pallette;
};
