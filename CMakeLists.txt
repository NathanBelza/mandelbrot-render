cmake_minimum_required(VERSION 3.20)
project(mandelbrotRender)

include_directories(include)
file(GLOB SOURCES "src/*.cpp")
add_executable(mandelbrotRender ${SOURCES})