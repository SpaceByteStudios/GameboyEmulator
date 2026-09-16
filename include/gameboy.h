#include "cpu.h"

class Gameboy {
public:
  Gameboy();

  void reset();
  void run();
  void draw();

  bool is_running();

private:
  CPU cpu;

  bool running;
};
