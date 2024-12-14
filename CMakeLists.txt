cmake_minimum_required(VERSION 3.18)
project(fractal LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 17)

# Find SDL
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Add the source files for the C++ and CUDA code
add_executable(newton newton_fractals/main_newton.cpp
                       newton_fractals/newton_fractal.cpp
                       newton_fractals/reframe.cpp
                       newton_fractals/render_cuda.cu)

add_executable(lyapunov lyapunov_fractals/lyapunov_fractal.cpp
                        lyapunov_fractals/lyapunov_main.cpp
                        lyapunov_fractals/render_cuda.cu
                        lyapunov_fractals/reframe.cpp)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

# Link Libraries
target_link_libraries(newton ${SDL2_LIBRARIES})
target_link_libraries(lyapunov ${SDL2_LIBRARIES})
