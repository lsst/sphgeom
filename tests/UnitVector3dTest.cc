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
/// \brief This file contains tests for the UnitVector3d class.

#include "UnitVector3d.h"

#include "Test.h"


using namespace lsst::sphgeom;


template <typename T>
void checkClose(T const & a, T const & b, Angle threshold) {
    CHECK(NormalizedAngle(a, b) <= threshold);
}

TEST_CASE(Stream) {
    UnitVector3d v(1, 0, 0);
    std::stringstream ss;
    ss << v;
    CHECK(ss.str() == "UnitVector3d(1, 0, 0)");
}

TEST_CASE(Construction) {
    Angle threshold(1e-15); // less than one billionth of an arcsecond
    LonLat points[6] = {
        LonLat::fromDegrees( 90,  0),
        LonLat::fromDegrees(180,  0),
        LonLat::fromDegrees( 55, 90),
        LonLat::fromDegrees(999,-90),
        LonLat::fromDegrees( 45,  0),
        LonLat::fromDegrees( 45, 45)
    };
    UnitVector3d vectors[6] = {
        UnitVector3d( 0, 1, 0),
        UnitVector3d(-1, 0, 0),
        UnitVector3d( 0, 0, 1),
        UnitVector3d( 0, 0,-1),
        UnitVector3d(1, 1, 0),
        UnitVector3d::fromNormalized(Vector3d(0.5, 0.5, 0.5 * std::sqrt(2.0)))
    };
    for (int i = 0; i < 6; ++i) {
        checkClose(UnitVector3d(points[i]), vectors[i], threshold);
        checkClose(vectors[i], UnitVector3d(points[i]), threshold);
        checkClose(LonLat(vectors[i]), points[i], threshold);
        checkClose(points[i], LonLat(vectors[i]), threshold);
    }
}

TEST_CASE(Comparison) {
    CHECK(!(UnitVector3d::X() == UnitVector3d::Y()));
    CHECK(!(UnitVector3d::Y() == UnitVector3d::Z()));
    CHECK(!(UnitVector3d::X() == UnitVector3d::Z()));
    CHECK(UnitVector3d::X() != UnitVector3d::Y());
    CHECK(UnitVector3d::Y() != UnitVector3d::Z());
    CHECK(UnitVector3d::X() != UnitVector3d::Z());
}

TEST_CASE(ComponentAccess) {
    UnitVector3d x = UnitVector3d::X();
    CHECK(x.x() == 1 && x.y() == 0 && x.z() == 0);
    UnitVector3d v =
        UnitVector3d::fromNormalized(0.5, -0.5, 0.5 * std::sqrt(2.0));
    CHECK(v != x);
    CHECK(v(0) == 0.5);
    CHECK(v(1) == -0.5);
    CHECK(v(2) == 0.5 * std::sqrt(2.0));
    CHECK(v.getData()[0] == v(0));
    CHECK(v.getData()[1] == v(1));
    CHECK(v.getData()[2] == v(2));
    CHECK(v.x() == v(0));
    CHECK(v.y() == v(1));
    CHECK(v.z() == v(2));
}

TEST_CASE(DotProduct) {
    UnitVector3d x(1, 0, 0), y(0, 1, 0), z(0, 0, 1);
    CHECK(UnitVector3d::X().dot(UnitVector3d::Y()) == 0);
    CHECK(UnitVector3d::X().dot(UnitVector3d::Z()) == 0);
    CHECK(UnitVector3d::Y().dot(UnitVector3d::Z()) == 0);
}

TEST_CASE(CrossProduct) {
    UnitVector3d x = UnitVector3d::X();
    UnitVector3d y = UnitVector3d::Y();
    UnitVector3d z = UnitVector3d::Z();
    CHECK(x.cross(y) == z);
    CHECK(y.cross(z) == x);
    CHECK(z.cross(x) == y);
    CHECK(x.robustCross(y) == 2.0*z);
    CHECK(y.robustCross(z) == 2.0*x);
    CHECK(z.robustCross(x) == 2.0*y);
}

TEST_CASE(ScalarProduct) {
    CHECK(UnitVector3d::X() * 2 == Vector3d(2, 0, 0));
}

TEST_CASE(Sum) {
    CHECK(UnitVector3d::X() + UnitVector3d::Y() + UnitVector3d::Z() ==
          Vector3d(1, 1, 1));
}

TEST_CASE(Difference) {
    CHECK(UnitVector3d::X() - UnitVector3d::Y() == Vector3d(1, -1, 0));
}

TEST_CASE(CwiseProduct) {
    CHECK(UnitVector3d::X().cwiseProduct(UnitVector3d::Y()) ==
          Vector3d(0, 0, 0));
}

TEST_CASE(OrthogonalTo) {
    UnitVector3d x = UnitVector3d::X();
    UnitVector3d y = UnitVector3d::Y();
    UnitVector3d z = UnitVector3d::Z();
    CHECK(UnitVector3d::orthogonalTo(x).dot(x) == 0.0);
    CHECK(UnitVector3d::orthogonalTo(x, x).dot(x) == 0.0);
    CHECK(UnitVector3d::orthogonalTo(y).dot(y) == 0.0);
    CHECK(UnitVector3d::orthogonalTo(y, y).dot(y) == 0.0);
    CHECK(UnitVector3d::orthogonalTo(x, y).dot(z) == 1.0);
}

TEST_CASE(NorthFrom) {
    UnitVector3d x = UnitVector3d::X();
    UnitVector3d z = UnitVector3d::Z();
    CHECK(UnitVector3d::northFrom(x) == z);
    CHECK(UnitVector3d::northFrom(z) == -x);
    CHECK(UnitVector3d::northFrom(-z) == x);
}

TEST_CASE(RotatedAround) {
    UnitVector3d x = UnitVector3d::X();
    UnitVector3d y = UnitVector3d::Y();
    UnitVector3d z = UnitVector3d::Z();
    Angle threshold(1e-15); // less than one billionth of an arcsecond
    checkClose(x.rotatedAround(z, Angle(PI/2)), y, threshold);
    checkClose(y.rotatedAround(x, Angle(PI/2)), z, threshold);
    checkClose(z.rotatedAround(y, Angle(PI/2)), x, threshold);
}
