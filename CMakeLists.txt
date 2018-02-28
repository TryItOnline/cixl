cmake_minimum_required(VERSION 3.0)

project(cixl)

set(CMAKE_C_COMPILER "gcc")
add_compile_options(-std=gnu1x -Wall -Werror -O2 -g)

file(GLOB_RECURSE sources src/cixl/*.c)

add_library(libcixl STATIC ${sources})
target_include_directories(libcixl PUBLIC src/)
set_target_properties(libcixl PROPERTIES PREFIX "")

add_executable(cixl ${sources} src/main.c)
target_include_directories(cixl PUBLIC src/)

add_executable(tests EXCLUDE_FROM_ALL ${sources} src/tests.c)
target_include_directories(tests PUBLIC src/)

file(GLOB headers src/cixl/*.h)
install(FILES ${headers} DESTINATION include/cixl)

file(GLOB type_headers src/cixl/types/*.h)
install(FILES ${type_headers} DESTINATION include/cixl/types)

file(GLOB lib_headers src/cixl/libs/*.h)
install(FILES ${lib_headers} DESTINATION include/cixl/libs)
install(FILES build/cixl DESTINATION bin)
install(FILES build/libcixl.a DESTINATION lib)
