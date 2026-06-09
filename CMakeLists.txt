cmake_minimum_required(VERSION 3.12)
project(sphgeom)

option(SPHGEOM_ENABLE_COVERAGE "Enable coverage instrumentation for supported compilers" OFF)

if(SPHGEOM_ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(
            "$<$<COMPILE_LANGUAGE:CXX>:-O0>"
            "$<$<COMPILE_LANGUAGE:CXX>:-g>"
            "$<$<COMPILE_LANGUAGE:CXX>:--coverage>"
        )
        add_link_options(--coverage)
    else()
        message(FATAL_ERROR "Coverage instrumentation requires GCC or Clang")
    endif()
endif()

enable_testing()

add_subdirectory(python/lsst/sphgeom)
add_subdirectory(src)
add_subdirectory(tests)
