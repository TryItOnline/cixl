cmake_minimum_required(VERSION 3.0)

project(cixl)

set(CMAKE_C_COMPILER "gcc")
add_compile_options(-std=gnu1x -Wall -Werror -O2 -g)

file(GLOB_RECURSE sources src/cixl/*.c)

add_library(libcixl STATIC ${sources})
target_include_directories(libcixl PUBLIC src/)
set_target_properties(libcixl PROPERTIES PREFIX "")
target_link_libraries(libcixl dl)

add_executable(cixl ${sources} src/main.c)
target_include_directories(cixl PUBLIC src/)
target_link_libraries(cixl dl)

add_executable(tests EXCLUDE_FROM_ALL ${sources} src/tests.c)
target_include_directories(tests PUBLIC src/)
target_link_libraries(tests dl)

file(GLOB headers src/cixl/*.h)
install(FILES ${headers} DESTINATION include/cixl)

file(GLOB lib_headers src/cixl/lib/*.h)
install(FILES ${lib_headers} DESTINATION include/cixl/lib)

file(GLOB lib_headers src/cixl/posix/*.h)
install(FILES ${lib_headers} DESTINATION include/cixl/posix)

install(FILES build/cixl PERMISSIONS WORLD_READ WORLD_EXECUTE DESTINATION bin)
install(FILES build/libcixl.a DESTINATION lib)
