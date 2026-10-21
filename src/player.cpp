#include "player.h"
#include "SDL3/SDL_audio.h"
#include "gameboy.h"
#include <algorithm>
#include <cmath>

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
  constexpr double frequency = 220.0;
  constexpr int channels = 2;
  constexpr size_t bytes_per_sample_frame = channels * sizeof(float);

  constexpr int target_bytes = sample_rate * bytes_per_sample_frame * 50 / 1000;
  constexpr int max_queue_bytes =
      sample_rate * bytes_per_sample_frame * 150 / 1000;

  const int queued_bytes = SDL_GetAudioStreamQueued(stream);

  int nominal_samples = sample_rate / 59;

  int error_bytes = queued_bytes - target_bytes;
  int sample_adjustment =
      -error_bytes /
      (static_cast<int>(bytes_per_sample_frame) * 4); // damping factor

  int samples_to_gen = nominal_samples + sample_adjustment;

  samples_to_gen = std::clamp(samples_to_gen, 200, 1200);

  if (queued_bytes >= max_queue_bytes) {
    const double phase_increment = frequency / sample_rate;
    phase += phase_increment * nominal_samples;
    phase -= std::floor(phase);
    return;
  }

  std::vector<float> samples;
  samples.resize(samples_to_gen * channels);

  constexpr double phase_increment = frequency / sample_rate;

  for (int i = 0; i < samples_to_gen; ++i) {
    const float sample = (phase < 0.5) ? 0.005f : -0.005f;

    samples[i * 2] = sample;
    samples[i * 2 + 1] = sample;

    phase += phase_increment;
    if (phase >= 1.0) {
      phase -= 1.0;
    }
  }

  SDL_PutAudioStreamData(stream, samples.data(),
                         static_cast<int>(samples.size() * sizeof(float)));
}

SDL_AudioStream *Player::getStream() { return stream; }
