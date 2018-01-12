cmake_minimum_required(VERSION 3.0)

project(cixl)

add_compile_options(-std=gnu1x -Wall -Werror -O3)

file(GLOB_RECURSE sources src/cixl/*.c)
file(GLOB_RECURSE headers src/cixl/*.h)

add_library(libcixl EXCLUDE_FROM_ALL STATIC ${sources})
target_include_directories(libcixl PUBLIC src/)
set_target_properties(libcixl PROPERTIES PREFIX "")

add_executable(cixl ${sources} src/main.c)
target_include_directories(cixl PUBLIC src/)

install(FILES ${headers} DESTINATION include/cixl)
install(FILES build/cixl DESTINATION bin)
install(FILES build/libcixl.a DESTINATION lib)
