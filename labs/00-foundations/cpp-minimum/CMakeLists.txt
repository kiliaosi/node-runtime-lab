cmake_minimum_required(VERSION 3.20)
project(f02_cpp_minimum LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(f02-lifetime src/main.cc)
add_executable(f02-file-raii src/file_raii.cc)
add_executable(f02-memory-raii src/memory_raii.cc)


# Maybe<T> is available in the headers bundled with Node.js.
# Keep the standalone C++ exercises available when those headers are absent.
set(f02_targets f02-lifetime f02-file-raii f02-memory-raii)
find_program(F02_NODE_EXECUTABLE NAMES node)
if(F02_NODE_EXECUTABLE)
  get_filename_component(f02_node_bin_dir "${F02_NODE_EXECUTABLE}" DIRECTORY)
  find_path(F02_V8_INCLUDE_DIR NAMES v8-maybe.h
    HINTS "${f02_node_bin_dir}/../include/node")
endif()

if(F02_V8_INCLUDE_DIR)
  add_executable(f02-v8-maybe src/v8_maybe.cc)
  target_include_directories(f02-v8-maybe SYSTEM PRIVATE "${F02_V8_INCLUDE_DIR}")
  list(APPEND f02_targets f02-v8-maybe)
else()
  message(STATUS "Node/V8 headers not found; set F02_V8_INCLUDE_DIR to enable f02-v8-maybe")
endif()

foreach(lab_target IN LISTS f02_targets)
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
