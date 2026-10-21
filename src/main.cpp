#include <chrono>
#include <filesystem>
#include <iostream>

#include <SDL3/SDL.h>

#include "SDL3/SDL_events.h"
#include "gameboy.h"
#include "player.h"
#include "renderer.h"

constexpr std::chrono::nanoseconds frame_duration =
    std::chrono::nanoseconds(1'000'000'000 / 60);

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <rom-path>\n";
    return 1;
  }

  const std::filesystem::path rom_path = argv[1];

  if (!std::filesystem::exists(rom_path) ||
      !std::filesystem::is_regular_file(rom_path)) {
    std::cerr << "Error: ROM file not found: " << rom_path << '\n';
    return 1;
  }

  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
    std::cerr << "SDL_Init failed: " << SDL_GetError() << '\n';
    return 1;
  }

  {
    Gameboy gameboy = Gameboy(rom_path.string());
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

      std::cout << "Frame: "
                << std::chrono::duration<double, std::milli>(elapsed).count()
                << " ms, queued: "
                << SDL_GetAudioStreamQueued(player.getStream()) << " bytes\n";

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
