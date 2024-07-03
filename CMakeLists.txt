cmake_minimum_required(VERSION 3.22)

project(Chip-8-Emulator
  DESCRIPTION "A simple Chip-8-Emulator"
  LANGUAGES CXX
  )

find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)

set(CMAKE_CXX_STANDARD 17)

add_executable(Chip-8-Emulator
  ./src/chip8.cpp
  ./src/main.cpp
  ./src/platform.cpp
  )


target_include_directories(Chip-8-Emulator PUBLIC headers/)

target_link_libraries(Chip-8-Emulator PRIVATE SDL2::SDL2)
