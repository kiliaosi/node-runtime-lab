cmake_minimum_required(VERSION 3.20)
project(runtime_smoke LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(runtime-smoke src/main.cc)

if(MSVC)
  target_compile_options(runtime-smoke PRIVATE /W4 /WX /permissive-)
else()
  target_compile_options(runtime-smoke PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
  )
endif()
