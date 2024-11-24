cmake_minimum_required(VERSION 3.18)
project(fractal LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 17)

# Add the source files for the C++ and CUDA code
add_executable(fractal main.cu)

# If you have external libraries, link them here
# target_link_libraries(cuda_project some_library)
