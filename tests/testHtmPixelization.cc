/*
 * LSST Data Management System
 * Copyright 2016 AURA/LSST.
 *
 * This product includes software developed by the
 * LSST Project (http://www.lsst.org/).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the LSST License Statement and
 * the GNU General Public License along with this program.  If not,
 * see <https://www.lsstcorp.org/LegalNotices/>.
 */

/// \file
/// \brief This file contains tests for HTM indexing.

#include "lsst/sphgeom/Circle.h"
#include "lsst/sphgeom/LonLat.h"
#include "lsst/sphgeom/HtmPixelization.h"
#include "lsst/sphgeom/UnitVector3d.h"

#include "test.h"

using namespace lsst::sphgeom;

enum class Tri : unsigned int {
    S00 = 32, S01, S02, S03,
    S10, S11, S12, S13,
    S20, S21, S22, S23,
    S30, S31, S32, S33,
    N00, N01, N02, N03,
    N10, N11, N12, N13,
    N20, N21, N22, N23,
    N30, N31, N32, N33
};

TEST_CASE(Level) {
    int level = 0;
    for (uint64_t index = 8; index != 0; index *= 4, level += 1) {
        CHECK(HtmPixelization::level(index) == level);
    }
    for (uint64_t index = 4; index != 0; index <<= 2) {
        CHECK(HtmPixelization::level(index) < 0);
    }
    for (uint64_t index = 0; index < 8; ++index) {
        CHECK(HtmPixelization::level(index) < 0);
    }
}

TEST_CASE(InvalidTrixel) {
    for (uint64_t index = 4; index != 0; index *= 4) {
        CHECK_THROW(HtmPixelization::triangle(index), std::invalid_argument);
    }
    for (uint64_t index = 0; index < 8; ++index) {
        CHECK_THROW(HtmPixelization::triangle(index), std::invalid_argument);
    }
}

TEST_CASE(Index) {
    double const c = 0.2928932188134525; // 1/(2 + √2)
    // A collection of test points.
    UnitVector3d const points[] = {
        UnitVector3d::X(),
        UnitVector3d::Y(),
        UnitVector3d::Z(),
        -UnitVector3d::X(),
        -UnitVector3d::Y(),
        -UnitVector3d::Z(),
        UnitVector3d(1.0, 1.0, 0.0),    // midpoint of  x and  y
        UnitVector3d(-1.0, 1.0, 0.0),   // midpoint of  y and -x
        UnitVector3d(-1.0, -1.0, 0.0),  // midpoint of -x and -y
        UnitVector3d(1.0, -1.0, 0.0),   // midpoint of -y and  x
        UnitVector3d(1.0, 0.0, 1.0),    // midpoint of  x and  z
        UnitVector3d(0.0, 1.0, 1.0),    // midpoint of  y and  z
        UnitVector3d(-1.0, 0.0, 1.0),   // midpoint of -x and  z
        UnitVector3d(0.0, -1.0, 1.0),   // midpoint of -y and  z
        UnitVector3d(1.0, 0.0, -1.0),   // midpoint of  x and -z
        UnitVector3d(0.0, 1.0, -1.0),   // midpoint of  y and -z
        UnitVector3d(-1.0, 0.0, -1.0),  // midpoint of -x and -z
        UnitVector3d(0.0, -1.0, -1.0),  // midpoint of -y and -z
        UnitVector3d(1.0, 1.0, 1.0),    // center of N3
        UnitVector3d(-1.0, 1.0, 1.0),   // center of N2
        UnitVector3d(-1.0, -1.0, 1.0),  // center of N1
        UnitVector3d(1.0, -1.0, 1.0),   // center of N0
        UnitVector3d(1.0, 1.0, -1.0),   // center of S0
        UnitVector3d(-1.0, 1.0, -1.0),  // center of S1
        UnitVector3d(-1.0, -1.0, -1.0), // center of S2
        UnitVector3d(1.0, -1.0, -1.0),  // center of S3
        UnitVector3d(c, c, 1.0),        // center of N31
        UnitVector3d(1.0, c, c),        // center of N32
        UnitVector3d(c, 1.0, c),        // center of N30
        UnitVector3d(-c, c, 1.0),       // center of N21
        UnitVector3d(-c, 1.0, c),       // center of N22
        UnitVector3d(-1.0, c, c),       // center of N20
        UnitVector3d(-c, -c, 1.0),      // center of N11
        UnitVector3d(-1.0, -c, c),      // center of N12
        UnitVector3d(-c, -1.0, c),      // center of N10
        UnitVector3d(c, -c, 1.0),       // center of N01
        UnitVector3d(c, -1.0, c),       // center of N02
        UnitVector3d(1.0, -c, c),       // center of N00
        UnitVector3d(c, c, -1.0),       // center of S01
        UnitVector3d(1.0, c, -c),       // center of S00
        UnitVector3d(c, 1.0, -c),       // center of S02
        UnitVector3d(-c, c, -1.0),      // center of S11
        UnitVector3d(-c, 1.0, -c),      // center of S10
        UnitVector3d(-1.0, c, -c),      // center of S12
        UnitVector3d(-c, -c, -1.0),     // center of S21
        UnitVector3d(-1.0, -c, -c),     // center of S20
        UnitVector3d(-c, -1.0, -c),     // center of S22
        UnitVector3d(c, -c, -1.0),      // center of S31
        UnitVector3d(c, -1.0, -c),      // center of S30
        UnitVector3d(1.0, -c, -c)       // center of S32
    };
    // Expected HTM indexes for the above.
    Tri const results[] = {
        Tri::N32, Tri::N22, Tri::N31, Tri::N12, Tri::N02, Tri::S01,
        Tri::N30, Tri::N20, Tri::N10, Tri::N00,
        Tri::N31, Tri::N21, Tri::N11, Tri::N01,
        Tri::S00, Tri::S10, Tri::S20, Tri::S30,
        Tri::N33, Tri::N23, Tri::N13, Tri::N03,
        Tri::S03, Tri::S13, Tri::S23, Tri::S33,
        Tri::N31, Tri::N32, Tri::N30, Tri::N21, Tri::N22, Tri::N20,
        Tri::N11, Tri::N12, Tri::N10, Tri::N01, Tri::N02, Tri::N00,
        Tri::S01, Tri::S00, Tri::S02, Tri::S11, Tri::S10, Tri::S12,
        Tri::S21, Tri::S20, Tri::S22, Tri::S31, Tri::S30, Tri::S32
    };
    for (size_t i = 0; i < sizeof(points) / sizeof(UnitVector3d); ++i) {
        uint64_t index = static_cast<uint64_t>(results[i]);
        CHECK(HtmPixelization(0).index(points[i]) == index >> 2);
        CHECK(HtmPixelization(1).index(points[i]) == index);
        CHECK(HtmPixelization::triangle(index).contains(points[i]));
        if (i >= 26) {
            // Test-point is the center of a level 2 trixel.
            index = index * 4 + 3;
            CHECK(HtmPixelization(2).index(points[i]) == index);
            CHECK(HtmPixelization(2).envelope(Circle(points[i], 1e-8)) ==
                  RangeSet(index));
        }
    }
    // Check for consistency with values obtained from other libraries
    CHECK(HtmPixelization(20).index(UnitVector3d(LonLat::fromDegrees(1, 1))) ==
          UINT64_C(17043491373057));
    HtmPixelization p(13);
    RangeSet s = p.envelope(Circle(-UnitVector3d::Z(), 1.0e-16));
    CHECK(s == RangeSet({553648128, 620756992, 687865856, 754974720}));
    s = p.envelope(Circle(UnitVector3d::Z(), 1.0e-16));
    CHECK(s == RangeSet({822083584, 889192448, 956301312, 1023410176}));
    s = p.envelope(Circle(UnitVector3d::X(), 1.0e-16));
    CHECK(s == RangeSet({536870912, 771751936, 805306368, 1040187392}));
    s = p.envelope(Circle(UnitVector3d::Y(), 1.0e-16));
    CHECK(s == RangeSet({570425344, 603979776, 973078528, 1006632960}));
    s = p.envelope(Circle(-UnitVector3d::X(), 1.0e-16));
    CHECK(s == RangeSet({637534208, 671088640, 905969664, 939524096}));
    s = p.envelope(Circle(-UnitVector3d::Y(), 1.0e-16));
    CHECK(s == RangeSet({704643072, 738197504, 838860800, 872415232}));
}

TEST_CASE(Adaptivity) {
    UnitVector3d center(1.0, 1.0, 1.0);
    for (int level = 0; level <= 13; ++level) {
        HtmPixelization p(level);
        Circle c0(center, Angle::fromDegrees(0.001));
        Circle c1(center, Angle::fromDegrees(0.1));
        Circle c2(center, Angle::fromDegrees(1.0));
        RangeSet a0 = p.envelope(c0, 0);
        RangeSet a1 = p.envelope(c1, 0);
        RangeSet a2 = p.envelope(c2, 0);
        CHECK(a2.contains(a1));
        CHECK(a1.contains(a0));
        for (size_t maxRanges = 64; maxRanges != 0; maxRanges /= 2) {
            RangeSet s = p.envelope(c0, maxRanges);
            CHECK(s.contains(a0));
            a0 = s;
            s = p.envelope(c1, maxRanges);
            CHECK(s.contains(a1));
            a1 = s;
            s = p.envelope(c2, maxRanges);
            CHECK(s.contains(a2));
            a2 = s;
            CHECK(a2.contains(a1));
            CHECK(a1.contains(a0));
        }
    }
}
