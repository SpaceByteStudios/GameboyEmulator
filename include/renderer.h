#pragma once

#include "gameboy.h"
#include <SDL3/SDL.h>

class Renderer {
public:
  Renderer(Gameboy &gameboy);
  ~Renderer();

  void draw();

private:
  SDL_Window *window = nullptr;
  SDL_Renderer *renderer = nullptr;

  Gameboy &gameboy;
};
