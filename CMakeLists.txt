cmake_minimum_required(VERSION 3.16)

project(balck_scholes_fdm)

set(CMAKE_CXX_STANDARD 17)

add_executable(black_scholes src/main.cpp src/bs.cpp)

target_include_directories(black_scholes PRIVATE include)