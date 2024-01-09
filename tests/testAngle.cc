/*
 * This file is part of sphgeom.
 *
 * Developed for the LSST Data Management System.
 * This product includes software developed by the LSST Project
 * (http://www.lsst.org).
 * See the COPYRIGHT file at the top-level directory of this distribution
 * for details of code ownership.
 *
 * This software is dual licensed under the GNU General Public License and also
 * under a 3-clause BSD license. Recipients may choose which of these licenses
 * to use; please see the files gpl-3.0.txt and/or bsd_license.txt,
 * respectively.  If you choose the GPL option then the following text applies
 * (but note that there is still no warranty even if you opt for BSD instead):
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
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/// \file
/// \brief This file contains tests for the Angle class.

#include <sstream>
#include <string>

#include "lsst/sphgeom/NormalizedAngle.h"

#include "test.h"


using namespace lsst::sphgeom;

TEST_CASE(Basic) {
    Angle a;
    CHECK(a.asRadians() == 0);
    CHECK(a.asDegrees() == 0);
    CHECK(a == a);
    CHECK(!(a != a));
    CHECK(Angle::fromDegrees(90.0).asRadians() == PI/2);
    CHECK(Angle::fromDegrees(180.0).asRadians() == PI);
    CHECK(Angle::fromDegrees(270.0).asRadians() == 3*PI/2);
    CHECK(Angle::fromRadians(PI/2).asDegrees() == 90.0);
    CHECK(Angle::fromRadians(PI).asDegrees() == 180.0);
    CHECK(Angle::fromRadians(3*PI/2).asDegrees() == 270.0);
}

TEST_CASE(Stream) {
    Angle a(1);
    std::stringstream ss;
    ss << a;
    CHECK(ss.str() == "1");
    ss.str(std::string());
}

TEST_CASE(Comparison) {
    Angle a1(1), a2(2);
    CHECK(a1 < a2);
    CHECK(a1 <= a2);
    CHECK(a1 == a1);
    CHECK(a1 != a2);
    CHECK(a2 > a1);
    CHECK(a2 >= a1);
}

TEST_CASE(Arithmetic) {
    Angle a1(1), a2(2), a3(3);
    Angle a4 = -a1;
    CHECK(a2 + a4 == a1);
    CHECK(a2 - a1 == a1);
    CHECK(a1 * 2 == a2);
    CHECK(a1 * 2 == 2 * a1);
    CHECK(a2 / 2 == a1);
    CHECK(a1 / a2 == 0.5);
    CHECK_CLOSE(sin(Angle(PI/6)), 0.5, 2);
    CHECK_CLOSE(cos(Angle(2*PI/3)), -0.5, 4);
    CHECK_CLOSE(tan(Angle(PI/4)), 1.0, 2);
    CHECK(abs(Angle(-1)) == Angle(1));
    CHECK(abs(Angle(1)) == Angle(1));
}

TEST_CASE(Normalization) {
    Angle a1(1), a10(10), mp(-PI);
    CHECK(a1.isNormalized());
    CHECK(NormalizedAngle(a1) == a1);
    CHECK(!mp.isNormalized());
    CHECK(!a10.isNormalized());
}

TEST_CASE(NaNValues) {
    CHECK(!Angle::nan().isNormalized());
    CHECK(Angle::nan().isNan());
    CHECK(std::isnan(Angle::nan().asRadians()));
    CHECK(std::isnan(Angle::nan().asDegrees()));
    // Check that arithmetic propagates NaNs
    CHECK((Angle::nan() * 2).isNan());
    CHECK((Angle::nan() / 2).isNan());
    CHECK((2 * Angle::nan()).isNan());
    CHECK((Angle::nan() + Angle(1)).isNan());
    CHECK((Angle(1) + Angle::nan()).isNan());
    CHECK((Angle::nan() - Angle(1)).isNan());
    CHECK((Angle(1) - Angle::nan()).isNan());
}
