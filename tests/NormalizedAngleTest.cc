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
/// \brief This file contains tests for the Angle class.

#include <sstream>
#include <string>

#include "LonLat.h"
#include "NormalizedAngle.h"
#include "Vector3d.h"

#include "Test.h"


using namespace lsst::sphgeom;

TEST_CASE(Basic) {
    NormalizedAngle b;
    CHECK(b.asRadians() == 0);
    CHECK(b.asDegrees() == 0);
    CHECK(b == b);
    CHECK(!(b != b));
}

TEST_CASE(Stream) {
    NormalizedAngle b(1);
    std::stringstream ss;
    ss << b;
    CHECK(ss.str() == "1 rad");
}

TEST_CASE(Comparison) {
    Angle a1(1), a2(2);
    NormalizedAngle b1(7), b2(2);
    CHECK(b1 < b2);
    CHECK(b1 <= b2);
    CHECK(b1 == b1);
    CHECK(b1 != b2);
    CHECK(b2 > b1);
    CHECK(b2 >= b1);
    CHECK(a2 == b2);
    CHECK(a2 <= b2);
    CHECK(a2 >= b2);
    CHECK(!(a2 != b2));
    CHECK(!(a2 < b2));
    CHECK(!(a2 > b2));
    CHECK(b2 == a2);
    CHECK(b2 <= a2);
    CHECK(b2 >= a2);
    CHECK(!(b2 != a2));
    CHECK(!(b2 < a2));
    CHECK(!(b2 > a2));
}

TEST_CASE(Arithmetic) {
    NormalizedAngle b1(1), b2(2);
    CHECK(b1 * 2 == Angle(2));
    CHECK(50 * b1 == Angle(50));
    CHECK(b1 * 2 == 2 * b1);
    CHECK(b1 / 0.0625 == Angle(16));
    CHECK(-b1 == Angle(-1));
    CHECK(b1 - b2 == Angle(-1));
    CHECK(b1 + NormalizedAngle(6) == Angle(7));
}

TEST_CASE(Normalization) {
    Angle a10(10), mp(-PI);
    CHECK_CLOSE(NormalizedAngle(a10).asRadians(), 10 - 2 * PI, 2);
    CHECK_CLOSE(NormalizedAngle(mp).asRadians(), PI, 1);
}

TEST_CASE(GetAngleTo) {
    NormalizedAngle a1(1), a2(2), a5(5);
    CHECK(a1.getAngleTo(a2).asRadians() == 1);
    CHECK(a2.getAngleTo(a5).asRadians() == 3);
    CHECK_CLOSE(a5.getAngleTo(a1).asRadians(), 2 * PI - 4, 1);
    CHECK(a1.getAngleTo(NormalizedAngle::nan()).isNan());
    CHECK(NormalizedAngle::nan().getAngleTo(a1).isNan());
    CHECK(NormalizedAngle::nan().getAngleTo(NormalizedAngle::nan()).isNan());
}

TEST_CASE(Between) {
    NormalizedAngle a1(1), a2(2), a5(5);
    CHECK(NormalizedAngle::between(a1, a1).asRadians() == 0);
    CHECK(NormalizedAngle::between(a1, a2).asRadians() == 1);
    CHECK(NormalizedAngle::between(a2, a5).asRadians() == 3);
    CHECK_CLOSE(NormalizedAngle::between(a1, a5).asRadians(), 2*PI - 4, 1);
    CHECK(NormalizedAngle::between(a1, NormalizedAngle::nan()).isNan());
    CHECK(NormalizedAngle::between(NormalizedAngle::nan(), a1).isNan());
    CHECK(NormalizedAngle::between(NormalizedAngle::nan(),
                                   NormalizedAngle::nan()).isNan());
}

TEST_CASE(AngularSeparation) {
    NormalizedAngle a45 = NormalizedAngle::fromDegrees(45);
    NormalizedAngle a90 = NormalizedAngle::fromDegrees(90);
    NormalizedAngle zero;
    NormalizedAngle a1(Vector3d(1, 0, 0), Vector3d(0, 0, 1));
    NormalizedAngle a2(Vector3d(1, -1, 1), Vector3d(-1, 1, -1));
    NormalizedAngle a3(Vector3d(1, 1, 1), Vector3d(1, 1, 1));
    NormalizedAngle a4(LonLat(a45, a45), LonLat(a45, zero));
    NormalizedAngle a5(LonLat(zero, zero), LonLat(a45, zero));
    NormalizedAngle a6(LonLat(a45, zero), LonLat(a90, zero));
    NormalizedAngle a7(LonLat(a45, a45), LonLat(a45, a45));
    CHECK(NormalizedAngle(Vector3d(), Vector3d()) == Angle(0));
    CHECK_CLOSE(a1.asRadians(), 0.5 * PI, 2);
    CHECK_CLOSE(a2.asRadians(), PI, 2);
    CHECK(a3.asRadians() == 0);
    CHECK_CLOSE(a4.asRadians(), a45.asRadians(), 2);
    CHECK_CLOSE(a5.asRadians(), a45.asRadians(), 2);
    CHECK_CLOSE(a6.asRadians(), a45.asRadians(), 2);
    CHECK(a7.asRadians() == 0);
    // Check for accuracy when measuring the angular separation
    // between positions that are extremely close together.
    LonLat p1(zero, Angle(1e-17));
    LonLat p2(NormalizedAngle(1e-17), Angle(1e-17));
    CHECK_CLOSE(NormalizedAngle(p1, p2).asRadians(), 1e-17, 1);
    Vector3d v1(1.0, 0.0, 0.0);
    Vector3d v2(1.0, 1e-17, 0.0);
    CHECK_CLOSE(NormalizedAngle(v1, v2).asRadians(), 1e-17, 1);
}

TEST_CASE(Infinity) {
    CHECK(NormalizedAngle(std::numeric_limits<double>::infinity()).isNan());
    CHECK(NormalizedAngle(-std::numeric_limits<double>::infinity()).isNan());
}
