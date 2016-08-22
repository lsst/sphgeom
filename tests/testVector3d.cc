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
/// \brief This file contains tests for the Vector3d class.
#include "lsst/sphgeom/Angle.h"
#include "lsst/sphgeom/Vector3d.h"

#include "test.h"


using namespace lsst::sphgeom;

template <typename T>
void checkClose(T const & a, T const & b, Angle threshold) {
    CHECK(NormalizedAngle(a, b) <= threshold);
}

TEST_CASE(Stream) {
    Vector3d v(1, 2, 3);
    std::stringstream ss;
    ss << v;
    CHECK(ss.str() == "[1, 2, 3]");
}

TEST_CASE(Comparison) {
    CHECK(!(Vector3d(0, 0, 0) == Vector3d(1, 0, 0)));
    CHECK(!(Vector3d(0, 0, 0) == Vector3d(0, 1, 0)));
    CHECK(!(Vector3d(0, 0, 0) == Vector3d(0, 0, 1)));
    CHECK(Vector3d(0, 0, 0) != Vector3d(1, 0, 0));
    CHECK(Vector3d(0, 0, 0) != Vector3d(0, 1, 0));
    CHECK(Vector3d(0, 0, 0) != Vector3d(0, 0, 1));
}

TEST_CASE(ComponentAccess) {
    Vector3d zero;
    CHECK(zero.x() == 0 && zero.y() == 0 && zero.z() == 0);
    Vector3d v(1, 2, 3);
    CHECK(v != zero);
    CHECK(v(0) == 1);
    CHECK(v(1) == 2);
    CHECK(v(2) == 3);
    CHECK(v.getData()[0] == 1);
    CHECK(v.getData()[1] == 2);
    CHECK(v.getData()[2] == 3);
    CHECK(v.x() == 1);
    CHECK(v.y() == 2);
    CHECK(v.z() == 3);
}

TEST_CASE(DotProduct) {
    Vector3d u(1, 2, 3), v(3, 0.5, 2), z(0, 0, 1);
    CHECK(z.dot(v) == v(2));
    CHECK(u.dot(v) == 10);
}

TEST_CASE(CrossProduct) {
    Vector3d u(1, 1, 1), v(-2, -0.5, -0.25);
    CHECK(u.cross(v) == -v.cross(u));
    CHECK(u.cross(u) == Vector3d(0, 0, 0));
    CHECK(u.cross(v) == Vector3d(0.25, -1.75, 1.5));
}

TEST_CASE(Norm) {
    Vector3d nil(0, 0, 0), x(1, 0, 0), v(2, 3, 6);
    CHECK(nil.getSquaredNorm() == 0);
    CHECK(nil.getNorm() == 0);
    CHECK(x.getNorm() == 1);
    CHECK(v.getNorm() == 7);
    CHECK(v.getSquaredNorm() == 49);
}

TEST_CASE(Normalize) {
    Vector3d v(2, 3, 6);
    v.normalize();
    CHECK(v.isNormalized());
    CHECK_CLOSE(v.getNorm(), 1, 5);
    // Check that normalization works on vectors with components so large
    // that squaring them would cause overflow.
    v = Vector3d(std::numeric_limits<double>::max(),
                 std::numeric_limits<double>::max(),
                 std::numeric_limits<double>::max());
    v.normalize();
    CHECK(v.isNormalized());
    CHECK_CLOSE(v.getNorm(), 1, 5);
    // Check that normalization works on vectors with components so small
    // that squaring them would cause underflow.
    v = Vector3d(std::numeric_limits<double>::min(),
                 std::numeric_limits<double>::min(),
                 std::numeric_limits<double>::min());
    v.normalize();
    CHECK(v.isNormalized());
    CHECK_CLOSE(v.getNorm(), 1, 5);
    // Check that normalizing the zero vector fails.
    CHECK_THROW(Vector3d().normalize(), std::runtime_error);
}


TEST_CASE(ScalarProduct) {
    CHECK(Vector3d(1, 2, -3) * 2 == Vector3d(2, 4, -6));
    CHECK(0.5 * Vector3d(-8, 2, 4) == Vector3d(-4, 1, 2));
}

TEST_CASE(Sum) {
    CHECK(Vector3d(1, 2, 3) + Vector3d(-3, -2, -1) == Vector3d(-2, 0, 2));
    CHECK(Vector3d(4, -1, 3) + Vector3d(-4, 1, -3) == Vector3d(0, 0, 0));
}

TEST_CASE(Difference) {
    CHECK(Vector3d(1, 2, 3) - Vector3d(-3, -2, -1) == Vector3d(4, 4, 4));
    CHECK(Vector3d(4, -1, 3) - Vector3d(1, 2, 3) == Vector3d(3, -3, 0));
}

TEST_CASE(CwiseProduct) {
    CHECK(Vector3d(1, 2, 3).cwiseProduct(Vector3d(4, 5, 6)) ==
          Vector3d(4, 10, 18));
}
