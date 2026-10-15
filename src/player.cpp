#include "player.h"
#include "SDL3/SDL_audio.h"
#include "gameboy.h"

Player::Player(Gameboy &gameboy) : gameboy(gameboy) {
  SDL_AudioSpec spec{SDL_AUDIO_F32, 2, 48000};

  stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec,
                                     nullptr, nullptr);

  if (!stream) {
    throw std::runtime_error("SDL_OpenAudioDeviceStream failed");
  }

  SDL_ResumeAudioStreamDevice(stream);
}

Player::~Player() { SDL_DestroyAudioStream(stream); }

void Player::submitAudio() {}
