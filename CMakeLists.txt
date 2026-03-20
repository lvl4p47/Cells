cmake_minimum_required(VERSION 3.10)

project(templateSDL)

add_compile_options(-O3 -march=native -flto -funroll-loops -fomit-frame-pointer -ffast-math -ftree-vectorize -fopt-info-vec)

set(SOURCES src/main.c src/utility.h src/utility.c src/grid.h src/grid.c src/graphics.h src/graphics.c src/input.h src/input.c src/init.h src/init.c)

include(CheckIPOSupported)
check_ipo_supported(RESULT supported OUTPUT error)

add_executable(templateSDL ${SOURCES})

set(CMAKE_BUILD_TYPE Release)

target_link_libraries(templateSDL SDL2 SDL2_image SDL2_mixer)

if( supported )
    message(STATUS "IPO / LTO enabled")
    set_property(TARGET templateSDL PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(STATUS "IPO / LTO not supported: <${error}>")
endif()
