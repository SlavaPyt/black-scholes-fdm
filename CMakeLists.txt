cmake_minimum_required(VERSION 3.16)

project(black_scholes_fdm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(BS_BUILD_PYTHON "Build the bs_engine Python module (requires pybind11)" ON)

# ---------------------------------------------------------------------------
# Pricing core, shared by the CLI, the Python module and the tests
# ---------------------------------------------------------------------------
add_library(bs_core STATIC src/bs.cpp)
target_include_directories(bs_core PUBLIC include)
set_target_properties(bs_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

if(MSVC)
    target_compile_options(bs_core PUBLIC /W4)
else()
    target_compile_options(bs_core PUBLIC -Wall -Wextra -Wpedantic)
endif()

# ---------------------------------------------------------------------------
# Command-line tool
# ---------------------------------------------------------------------------
add_executable(black_scholes src/main.cpp)
target_link_libraries(black_scholes PRIVATE bs_core)

# ---------------------------------------------------------------------------
# Python module (optional)
# ---------------------------------------------------------------------------
if(BS_BUILD_PYTHON)
    find_package(Python 3.8 COMPONENTS Interpreter Development.Module)

    # A pip-installed pybind11 reports where its CMake files are.
    if(Python_FOUND AND NOT pybind11_DIR)
        execute_process(
            COMMAND "${Python_EXECUTABLE}" -m pybind11 --cmakedir
            OUTPUT_VARIABLE _pybind11_cmake_dir
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET)
        if(_pybind11_cmake_dir)
            set(pybind11_DIR "${_pybind11_cmake_dir}")
        endif()
    endif()

    find_package(pybind11 CONFIG QUIET)

    if(Python_FOUND AND pybind11_FOUND)
        pybind11_add_module(bs_engine python_bindings/bindings.cpp)
        target_link_libraries(bs_engine PRIVATE bs_core)
    else()
        message(WARNING
            "pybind11 not found, the Python module will not be built. "
            "Install it with 'pip install pybind11' or configure with -DBS_BUILD_PYTHON=OFF.")
    endif()
endif()
