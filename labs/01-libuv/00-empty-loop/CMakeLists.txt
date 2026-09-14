add_executable(l00-empty-loop main.c)
target_link_libraries(l00-empty-loop PRIVATE libuv::libuv)

if(MSVC)
  target_compile_options(l00-empty-loop PRIVATE /W4 /WX)
else()
  target_compile_options(l00-empty-loop PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
  )
endif()
