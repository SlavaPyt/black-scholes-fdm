cmake_minimum_required(VERSION 3.16)

project(black_scholes_fdm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(black_scholes src/main.cpp src/bs.cpp)

target_include_directories(black_scholes PRIVATE include)

find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

execute_process(COMMAND python3 -m pybind11 --cmakedir OUTPUT_VARIABLE PYBIND11_CMAKE_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)

list(APPEND CMAKE_PREFIX_PATH ${PYBIND11_CMAKE_DIR})
find_package(pybind11 REQUIRED)

pybind11_add_module(bs_engine python_bindings/bindings.cpp src/bs.cpp)

target_include_directories(bs_engine PRIVATE include)
