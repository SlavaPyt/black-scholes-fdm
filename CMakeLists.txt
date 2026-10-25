cmake_minimum_required(VERSION 3.16)

project(black_scholes_fdm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(BS_BUILD_PYTHON "Build the bs_engine Python module (requires pybind11)" ON)
option(BS_BUILD_TESTS "Build the unit tests" ON)

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

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
if(BS_BUILD_TESTS)
    enable_testing()

    add_executable(bs_tests tests/test_bs.cpp)
    target_link_libraries(bs_tests PRIVATE bs_core)
    add_test(NAME unit_tests COMMAND bs_tests)

    # End-to-end checks of the command-line tool
    add_test(NAME cli_single_call
             COMMAND black_scholes --S 100 --K 100 --r 0.05 --sigma 0.2 --T 1 --style call)
    set_tests_properties(cli_single_call PROPERTIES PASS_REGULAR_EXPRESSION "price=10\\.4506")

    add_test(NAME cli_single_put
             COMMAND black_scholes --S 100 --K 100 --r 0.05 --sigma 0.2 --T 1 --style put)
    set_tests_properties(cli_single_put PROPERTIES PASS_REGULAR_EXPRESSION "price=5\\.57353")

    add_test(NAME cli_grid
             COMMAND black_scholes --grid-S 50 150 11 --out "${CMAKE_CURRENT_BINARY_DIR}/cli_grid.csv")
    set_tests_properties(cli_grid PROPERTIES PASS_REGULAR_EXPRESSION "Wrote 11 rows")

    add_test(NAME cli_help COMMAND black_scholes --help)

    # Bad input must produce a clear error message instead of a crash
    function(add_cli_error_test name expected_error)
        add_test(NAME ${name} COMMAND black_scholes ${ARGN})
        set_tests_properties(${name} PROPERTIES PASS_REGULAR_EXPRESSION "Error: ${expected_error}")
    endfunction()

    add_cli_error_test(cli_missing_value "--K requires a value" --K)
    add_cli_error_test(cli_not_a_number "--sigma: 'abc' is not a number" --sigma abc)
    add_cli_error_test(cli_invalid_sigma "sigma must be a positive" --sigma 0)
    add_cli_error_test(cli_unknown_style "--style must be" --style straddle)
    add_cli_error_test(cli_unknown_method "--method 'crr' is not implemented" --method crr)
    add_cli_error_test(cli_unknown_argument "unknown argument" --bogus)
    add_cli_error_test(cli_grid_too_few_points "--grid-S needs at least 2 points"
                       --grid-S 50 150 1 --out unused.csv)
    add_cli_error_test(cli_grid_without_out "--out is required" --grid-S 50 150 11)
endif()
