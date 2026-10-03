cmake_minimum_required(VERSION 3.20)
project(mandelbrot_render LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(include)

add_executable(mandelbrot_render src/main.cpp src/mandelbrot.cpp src/image.cpp)