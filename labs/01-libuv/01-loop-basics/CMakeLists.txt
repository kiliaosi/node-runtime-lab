add_executable(l01-loop-basics main.c)
target_link_libraries(l01-loop-basics PRIVATE libuv::libuv)

if(MSVC)
  target_compile_options(l01-loop-basics PRIVATE /W4 /WX)
else()
  target_compile_options(l01-loop-basics PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
  )
endif()
