cmake_minimum_required(VERSION 3.20)

project(
    GameboyEmulator
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL3 REQUIRED)

add_executable(
    gb_emulator
    src/main.cpp
)

target_link_libraries(gb_emulator PRIVATE SDL3::SDL3)

target_include_directories(
    gb_emulator 
    PRIVATE
    include
)
