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

void Player::submitAudio() {
  constexpr int sample_rate = 48000;
  constexpr double frequency = 880.0;
  constexpr int samples_per_frame = sample_rate / 60;

  static double phase = 0.0;

  std::vector<float> samples;
  // Left and Right
  samples.reserve(samples_per_frame * 2);

  constexpr double phase_increment = frequency / sample_rate;

  for (int i = 0; i < samples_per_frame; ++i) {
    // 50% duty cycle: half high, half low.
    const float sample = (phase < 0.5) ? 0.5f : -0.5f;

    // Interleaved stereo: left, right.
    samples.push_back(sample);
    samples.push_back(sample);

    phase += phase_increment;

    if (phase >= 1.0) {
      phase -= 1.0;
    }
  }

  SDL_PutAudioStreamData(stream, samples.data(),
                         samples.size() * sizeof(float));
}
