cmake_minimum_required(VERSION 3.16)
project (emulator C)

set(CMAKE_C_STANDARD 11)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)

include_directories(headers)

add_executable(
    emulator
    emu.c
    chip8.c
    display.c
)

target_link_libraries(emulator PRIVATE PkgConfig::SDL2)

set_target_properties(emulator PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)