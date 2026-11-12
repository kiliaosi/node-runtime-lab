cmake_minimum_required(VERSION 3.20)
project(f03_os00_syscalls LANGUAGES C)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
  message(FATAL_ERROR "This OS00 syscall experiment runs on Linux.")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(f03-os00-syscalls src/main.c)
target_compile_options(f03-os00-syscalls PRIVATE
  -O0 -g -Wall -Wextra -Wpedantic -Werror
)

add_executable(f03-os00-file-write src/file_write.c)
target_compile_options(f03-os00-file-write PRIVATE
  -O0 -g -Wall -Wextra -Wpedantic -Werror
)

add_executable(f03-os00-fd-reuse src/fd_reuse.c)
target_compile_options(f03-os00-fd-reuse PRIVATE
  -O0 -g -Wall -Wextra -Wpedantic -Werror
)

add_executable(f03-os00-fd-offset src/fd_offset.c)
target_compile_options(f03-os00-fd-offset PRIVATE
  -O0 -g -Wall -Wextra -Wpedantic -Werror
)
