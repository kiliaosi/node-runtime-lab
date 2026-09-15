cmake_minimum_required(VERSION 3.20)
project(f01_c_minimum LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_executable(f01-pointers src/main.c)

if(MSVC)
  target_compile_options(f01-pointers PRIVATE /W4 /WX)
else()
  target_compile_options(f01-pointers PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
  )
endif()
