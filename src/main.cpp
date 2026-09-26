#include <chrono>
#include <iostream>
#include <thread>

#include <SDL3/SDL.h>

#include "gameboy.h"
#include "renderer.h"

constexpr double target_frame_time = 1.0 / 60.0;

int main() {
  Gameboy gameboy = Gameboy("./roms/games/Tetris.gb");
  Renderer renderer = Renderer(gameboy);

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    std::cerr << "SDL_Init failed: " << SDL_GetError() << '\n';
    return 1;
  }

  bool running = true;

  while (running) {
    auto frame_start = std::chrono::steady_clock::now();

    SDL_Event event;

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        running = false;
      }
    }

    gameboy.run();

    renderer.draw();

    auto frame_end = std::chrono::steady_clock::now();
    std::chrono::duration<double> frame_time = frame_end - frame_start;

    double remaining_time = target_frame_time - frame_time.count();

    if (remaining_time > 0.0) {
      std::this_thread::sleep_for(
          std::chrono::duration<double>(remaining_time));
    }
  }

  SDL_Quit();
  return 0;
}
