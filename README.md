# Gameboy Emulator

A Game Boy emulator written in C++, recreating the hardware and behavior of Nintendo's original Game Boy system in software.

The application emulates the Game Boy's core hardware components, allowing you to run classic Game Boy games directly on your computer.

<!--
<p>
  <img src="assets/Emulator.png" width="600" />
</p>
-->

## Features

- CPU instruction execution
- Memory management
- Graphics rendering
- Game ROM loading
- Input handling

In progress:
- Audio emulation

## Getting Started

Clone the repository

```bash
git clone --recurse-submodules https://github.com/SpaceByteStudios/GameboyEmulator.git
cd GameboyEmulator
```

Build the project

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Run the emulator

```bash
./build/bin/gb_emulator
```

## Application

### Emulator

Load and play Game Boy games directly through the emulator.

<!--
<p>
  <img src="assets/Gameplay.png" width="600" />
</p>
-->

### CPU Emulation

The emulator reproduces the behavior of the Game Boy's CPU, executing instructions and updating the system state accordingly.

The CPU implementation handles:

- Instruction decoding and execution
- Registers and flags
- Arithmetic and logical operations
- Memory access
- Conditional branching
- Interrupt handling
- Instruction timing

### Memory Management

The emulator implements the Game Boy's memory system, allowing the CPU and other hardware components to interact with the correct memory regions.

This includes:

- Memory reads and writes
- Memory-mapped hardware registers
- ROM and RAM access
- MBC Cartridge Banking

### Graphics

The graphics subsystem emulates the Game Boy's display hardware to render game visuals.

<!--
<p>
  <img src="assets/Graphics.png" width="600" />
</p>
-->

The graphics implementation includes:

- Tile-based graphics rendering
- Background and window layers
- Sprite rendering
- LCD timing and rendering modes

## How It Works

The emulator recreates the behavior of the Game Boy's hardware through software components that work together to execute a game.

1. A Game Boy ROM is loaded into the emulator.
2. The CPU fetches and executes instructions from memory.
3. Memory operations update the system state and hardware registers.
4. The graphics subsystem processes video memory and renders the display.
5. Input events are translated into Game Boy button presses.
6. The emulation loop continuously advances the system to keep the game running.

By reproducing the interactions between the CPU, memory, graphics hardware, and input system, the emulator allows original Game Boy software to run on modern hardware.

## Resources

Interested in learning more about the Game Boy's hardware and emulation?

- [Pan Docs](https://gbdev.io/pandocs/) — Comprehensive documentation of the Game Boy's hardware.
- [Game Boy: Complete Technical Reference](https://gekkio.fi/files/gb-docs/gbctr.pdf) — Detailed technical documentation of the system.
- [The Ultimate Game Boy Talk (33c3)](https://youtu.be/HyzD8pNlpwI) – An in-depth talk about the Game Boy's hardware and architecture.
