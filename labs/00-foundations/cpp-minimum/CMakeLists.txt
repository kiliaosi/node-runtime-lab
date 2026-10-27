cmake_minimum_required(VERSION 3.20)
project(f02_cpp_minimum LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(f02-lifetime src/main.cc)
add_executable(f02-file-raii src/file_raii.cc)

foreach(lab_target IN ITEMS f02-lifetime f02-file-raii)
  if(MSVC)
    target_compile_options(${lab_target} PRIVATE /W4 /WX /permissive-)
  else()
    target_compile_options(${lab_target} PRIVATE
      -Wall
      -Wextra
      -Wpedantic
      -Werror
    )
  endif()
endforeach()
