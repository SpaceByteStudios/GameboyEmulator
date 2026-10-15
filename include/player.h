#pragma once

#include "SDL3/SDL_audio.h"
#include "gameboy.h"

class Player {
public:
  Player(Gameboy &gameboy);
  ~Player();

  void submitAudio();

private:
  SDL_AudioStream *stream;

  Gameboy &gameboy;
};
