cmake_minimum_required(VERSION 3.20)
project(node_runtime_labs LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

if(NOT LIBUV_SOURCE_DIR AND DEFINED ENV{NODE_RUNTIME_SOURCES})
  set(LIBUV_SOURCE_DIR "$ENV{NODE_RUNTIME_SOURCES}/libuv")
endif()

if(NOT LIBUV_SOURCE_DIR OR NOT EXISTS "${LIBUV_SOURCE_DIR}/CMakeLists.txt")
  message(FATAL_ERROR
    "libuv source not found. Activate the lab environment and fetch libuv, "
    "or configure with -DLIBUV_SOURCE_DIR=/path/to/libuv")
endif()

set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
add_subdirectory("${LIBUV_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/_deps/libuv" EXCLUDE_FROM_ALL)

add_subdirectory(01-libuv/00-empty-loop)
add_subdirectory(01-libuv/01-loop-basics)
