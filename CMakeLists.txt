cmake_minimum_required(VERSION 3.22)
project(Chip-8-Emulator
  DESCRIPTION "A simple Chip-8-Emulator"
  LANGUAGES CXX
  )

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL3 REQUIRED)
file(GLOB SRC_FILES
  "./src/*.cpp"
  "./src/*.h"
  )
add_executable(${PROJECT_NAME} ${SRC_FILES})
