find_package(Python
    REQUIRED COMPONENTS Interpreter Development.Module
    OPTIONAL_COMPONENTS Development.SABIModule)

execute_process(
    COMMAND "${Python_EXECUTABLE}" -m nanobind --cmake_dir
    OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE NB_DIR)

list(APPEND CMAKE_PREFIX_PATH "${NB_DIR}")

find_package(nanobind CONFIG REQUIRED)

nanobind_add_module(_sphgeom
    STABLE_ABI
    _angle.cc
    _angleInterval.cc
    _box3d.cc
    _box.cc
    _chunker.cc
    _circle.cc
    _compoundRegion.cc
    _convexPolygon.cc
    _curve.cc
    _ellipse.cc
    _htmPixelization.cc
    _interval1d.cc
    _lonLat.cc
    _matrix3d.cc
    _mq3cPixelization.cc
    _normalizedAngle.cc
    _normalizedAngleInterval.cc
    _orientation.cc
    _pixelization.cc
    _q3cPixelization.cc
    _rangeSet.cc
    _region.cc
    _relationship.cc
    _sphgeom.cc
    _unitVector3d.cc
    _utils.cc
    _vector3d.cc
)

target_link_libraries(_sphgeom PUBLIC sphgeom)

install(TARGETS _sphgeom DESTINATION ${CMAKE_INSTALL_PREFIX}/python/lsst/sphgeom)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
    DESTINATION ${CMAKE_INSTALL_PREFIX}/python/lsst/sphgeom
    FILES_MATCHING PATTERN "*.py"
)

add_custom_target(sphgeom_version ALL
    ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/version.cmake
    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/version.py
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/version.py
    DESTINATION ${CMAKE_INSTALL_PREFIX}/python/lsst/sphgeom
)
