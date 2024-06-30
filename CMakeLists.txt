cmake_minimum_required(VERSION 3.22)
project(Chip-8-Emulator
  DESCRIPTION "A simple Chip-8-Emulator"
  LANGUAGES CXX
  )

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



set(SOURCE_FILES ./src/main.cpp ./src/chip8.cpp ./headers/)
add_executable(chip-8-emu ${SOURCE_FILES})
