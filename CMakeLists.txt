cmake_minimum_required(VERSION 3.1)
project(sphgeom)

enable_testing()

add_subdirectory(python/lsst/sphgeom)
add_subdirectory(src)
add_subdirectory(tests)
