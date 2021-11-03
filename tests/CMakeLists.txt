FUNCTION(sphgeom_tests)
    FOREACH(TEST IN ITEMS ${ARGV})
        add_executable(${TEST} ${TEST}.cc)
        target_link_libraries(${TEST} PUBLIC sphgeom)
        add_test(NAME ${TEST} COMMAND ${TEST})
    ENDFOREACH()
ENDFUNCTION()

sphgeom_tests(
    testAngle
    testAngleInterval
    testBigInteger
    testBox
    testChunker
    testCircle
    testConvexPolygon
    testCurve
    testEllipse
    testHtmPixelization
    testInterval1d
    testLonLat
    testMatrix3d
    testMq3cPixelization
    testNormalizedAngle
    testNormalizedAngleInterval
    testOrientation
    testQ3cPixelization
    testRangeSet
    testUnitVector3d
    testVector3d
)
