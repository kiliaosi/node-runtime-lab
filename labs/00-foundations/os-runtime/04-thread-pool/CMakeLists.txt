cmake_minimum_required(VERSION 3.20)
project(f03_os04_thread_pool LANGUAGES C)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
  message(FATAL_ERROR "This thread pool exercise runs on Linux.")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(f03-os04-thread-pool src/thread_pool.c)
target_link_libraries(f03-os04-thread-pool PRIVATE Threads::Threads)
target_compile_options(f03-os04-thread-pool PRIVATE
  -O0 -g -Wall -Wextra -Wpedantic -Werror
)
