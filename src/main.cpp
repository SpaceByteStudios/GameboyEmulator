#include <chrono>
#include <iostream>

#include <SDL3/SDL.h>

#include "SDL3/SDL_events.h"
#include "gameboy.h"
#include "player.h"
#include "renderer.h"

constexpr std::chrono::nanoseconds frame_duration =
    std::chrono::nanoseconds(1'000'000'000 / 60);

int main() {
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
    std::cerr << "SDL_Init failed: " << SDL_GetError() << '\n';
    return 1;
  }

  {
    Gameboy gameboy = Gameboy("./roms/games/Tetris.gb");
    Renderer renderer = Renderer(gameboy);
    Player player = Player(gameboy);

    bool running = true;

    while (running) {
      auto frame_start = std::chrono::steady_clock::now();

      SDL_Event event;

      while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
          running = false;
        }

        renderer.updateInput(event.key);
      }

      gameboy.runFrame();

      renderer.draw();
      player.submitAudio();

      const auto elapsed = std::chrono::steady_clock::now() - frame_start;

      if (elapsed < frame_duration) {
        const auto remaining = frame_duration - elapsed;

        SDL_DelayNS(static_cast<Uint64>(remaining.count()));
      }
    }

    gameboy.hexDump("memory_dump.txt");
  }

  SDL_Quit();
  return 0;
}
