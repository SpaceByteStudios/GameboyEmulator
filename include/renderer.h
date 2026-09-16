#include <SDL3/SDL.h>

class Renderer {
public:
  Renderer();
  ~Renderer();

  void draw();

private:
  SDL_Window *window = nullptr;
  SDL_Renderer *renderer = nullptr;
};
