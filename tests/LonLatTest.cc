/*
 * LSST Data Management System
 * Copyright 2014-2015 AURA/LSST.
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
/// \brief This file contains tests for the LonLat class.

#include <limits>

#include "lsst/sphgeom/LonLat.h"
#include "lsst/sphgeom/Vector3d.h"

#include "Test.h"


using namespace lsst::sphgeom;

void checkClose(LonLat const & p1, LonLat const & p2, Angle threshold) {
    CHECK(NormalizedAngle(p1, p2) <= threshold);
}

TEST_CASE(Stream) {
    LonLat p = LonLat::fromRadians(2, 1);
    std::stringstream ss;
    ss << p;
    CHECK(ss.str() == "[2, 1]");
}

TEST_CASE(Construction) {
    Angle threshold(1e-15); // less than one billionth of an arcsecond
    checkClose(LonLat::fromDegrees(90, 45),
               LonLat::fromRadians(PI/2, PI/4), threshold);
    checkClose(LonLat::fromRadians(PI/2, PI/4),
               LonLat(NormalizedAngle(PI/2), Angle(PI/4)), threshold);
    // Note that Vector3dTest.cc also contains tests for the
    // LonLot(Vector3d const &) constructor.
    LonLat p1(Vector3d(1, 0, 0));
    checkClose(p1, LonLat::fromDegrees(0, 0), threshold);
    LonLat p2(Vector3d(0, -1, 0));
    checkClose(p2, LonLat::fromDegrees(270, 0), threshold);
    LonLat p3(Vector3d(0, 0, 1));
    checkClose(p3, LonLat::fromDegrees(0, 90), threshold);
    LonLat p4(Vector3d(0, 0, -1));
    checkClose(p4, LonLat::fromDegrees(0, -90), threshold);
    // Trying to create a LonLat having latitude of magnitude
    // > π/2 should fail.
    CHECK_THROW(LonLat::fromRadians(0, 2), std::invalid_argument);
    CHECK_THROW(LonLat::fromRadians(0, -2), std::invalid_argument);
}

TEST_CASE(ComponentAccess) {
    LonLat p = LonLat::fromRadians(2, 1);
    CHECK(p.getLon() == NormalizedAngle(2));
    CHECK(p.getLat() == Angle(1));
}

TEST_CASE(Comparison) {
    LonLat p1 = LonLat::fromRadians(1, 0.5);
    LonLat p2 = LonLat::fromRadians(0, 0.5);
    LonLat p3 = LonLat::fromRadians(1, 1.5);
    CHECK(p1 == p1);
    CHECK(p1 != p2);
    CHECK(p1 != p3);
    CHECK(!(p1 == p3));
    CHECK(!(p1 == p2));
}

TEST_CASE(NaNComponents) {
    double const NaN = std::numeric_limits<double>::quiet_NaN();
    // Creating a LonLat with NaN components should not throw.
    LonLat p1 = LonLat::fromRadians(NaN, NaN);
    CHECK(p1 != p1);
    // If one component is NaN, the other should be as well.
    LonLat p2 = LonLat::fromRadians(NaN, 0.5);
    LonLat p3 = LonLat::fromRadians(0.5, NaN);
    CHECK(p2.getLon().isNan() && p2.getLat().isNan());
    CHECK(p3.getLon().isNan() && p3.getLat().isNan());
}
