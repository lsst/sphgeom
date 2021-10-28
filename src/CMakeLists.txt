add_library(sphgeom SHARED)

target_compile_features(sphgeom PRIVATE 
    cxx_std_14
)

set_target_properties(sphgeom PROPERTIES 
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
)

target_include_directories(sphgeom PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

target_sources(sphgeom PRIVATE
    Angle.cc
    AngleInterval.cc
    BigInteger.cc
    Box3d.cc
    Box.cc
    Chunker.cc
    Circle.cc
    ConvexPolygon.cc
    ConvexPolygonImpl.h
    Ellipse.cc
    HtmPixelization.cc
    Interval1d.cc
    LonLat.cc
    Matrix3d.cc
    Mq3cPixelization.cc
    NormalizedAngle.cc
    NormalizedAngleInterval.cc
    orientation.cc
    PixelFinder.h
    Q3cPixelization.cc
    Q3cPixelizationImpl.h
    RangeSet.cc
    Region.cc
    UnitVector3d.cc
    utils.cc
    Vector3d.cc
)

install(TARGETS sphgeom)
