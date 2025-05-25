cmake_minimum_required(VERSION 3.27)
project(fft C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(CMAKE_C_STANDARD 99)
add_executable(fft main.c dfts.c util.c)

add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
target_link_libraries(fft PRIVATE SDL2main SDL2)
