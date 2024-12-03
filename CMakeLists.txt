cmake_minimum_required(VERSION 3.18)
project(fractal LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 17)

# Find SDL
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Add the source files for the C++ and CUDA code
add_executable(fractal main.cu)

# Link Libraries
target_link_libraries(fractal ${SDL2_LIBRARIES})
