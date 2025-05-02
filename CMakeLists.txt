cmake_minimum_required(VERSION 3.17..3.30)
project(sphgeom)

# Find Python interpreter and libraries

find_package(Python3 3.8 REQUIRED COMPONENTS Interpreter Development)
execute_process(
  COMMAND "${Python3_EXECUTABLE}" -m pybind11 --cmakedir
  OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE pybind11_ROOT
  ERROR_VARIABLE error_output
)
find_package(pybind11 CONFIG REQUIRED PATHS ${pybind11_ROOT})

enable_testing()

add_subdirectory(python/lsst/sphgeom)
add_subdirectory(src)
add_subdirectory(tests)
