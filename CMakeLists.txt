cmake_minimum_required(VERSION 3.20)

project(
    GameboyEmulator
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
    "${CMAKE_BINARY_DIR}/bin"
)

add_subdirectory(
    external/SDL
    EXCLUDE_FROM_ALL
)

add_executable(
    gb_emulator
    src/main.cpp
    src/renderer.cpp
    src/gameboy.cpp
    src/cpu.cpp
    src/registers.cpp
    src/timer.cpp
    src/memory.cpp
    src/cartridge.cpp
    src/ppu.cpp
)

target_include_directories(
    gb_emulator 
    PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(gb_emulator PRIVATE SDL3::SDL3)
