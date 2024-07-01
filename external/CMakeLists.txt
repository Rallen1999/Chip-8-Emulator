set(OPENGL_GL_PREFERENCE "LEGACY")
find_package(OpenGL 2 REQUIRED)
find_package(SDL3 REQUIRED)
include(FetchContent)
set(SDL_VERSION 2.30.4)
FetchContent_Declare(
  SDL
  URL https://github.com/libsdl-org/SDL/releases/tag/${SDL_VERSION}.zip
  FIND_PACKAGE_ARGS ${SDL_VERSION}
  SOURCE_DIR ./sdl/
  )

FetchContent_MakeAvailable(SDL)

file(GLOB imgui
  "./imgui/*.cpp"
  "./imgui/*.h"
  )
add_library(imgui STATIC
  imgui
  )

target_link_libraries(imgui PUBLIC)
target_link_libraries(SDL PUBLIC)

add_library(app_external_deps)



