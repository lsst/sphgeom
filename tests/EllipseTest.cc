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
/// \brief This file contains tests for the Ellipse class.

#include <memory>
#include <vector>

#include "lsst/sphgeom/Box.h"
#include "lsst/sphgeom/Circle.h"
#include "lsst/sphgeom/Ellipse.h"

#include "Test.h"


using namespace lsst::sphgeom;

std::vector<UnitVector3d> getPointsOnEllipse(Ellipse const & e,
                                             size_t numPoints)
{
    std::vector<UnitVector3d> points;
    if (e.isEmpty() || e.isFull()) {
        return points;
    }
    Matrix3d M = e.getTransformMatrix().transpose();
    double tana = tan(e.getAlpha() - Angle(0.5 * PI));
    double tanb = tan(e.getBeta() - Angle(0.5 * PI));
    for (size_t i = 0; i < numPoints; ++i) {
        // Points on the ellipse boundary satisfy:
        //
        //     x² cot²α + y² cot²β - z² = 0
        //
        // in the appropriate basis. Set (x, y, z) = (t cos θ, t sin θ, 1) and
        // substitute into the equation above to solve for t. Transform back to
        // the original basis and normalize to obtain a point in S² and close to
        // the ellipse boundary.
        Angle theta = Angle(2.0 * PI) * (static_cast<double>(i) / numPoints);
        double s = sin(theta);
        double c = cos(theta);
        double u = c * tana;
        double v = s * tanb;
        double t = std::sqrt(1.0 / (u * u + v * v));
        points.push_back(UnitVector3d(M * Vector3d(t * c, t * s, 1.0)));
    }
    return points;
}


TEST_CASE(Stream) {
    Ellipse e(UnitVector3d::Z(), Angle(1));
    std::stringstream ss;
    ss << e;
    CHECK(ss.str() == "Ellipse(\n"
                      "    Matrix3d( 0, 1, -0,\n"
                      "             -1, 0,  0,\n"
                      "              0, 0,  1),\n"
                      "    1 rad,\n"
                      "    1 rad\n"
                      ")");
}

TEST_CASE(Clone) {
    Ellipse e(UnitVector3d::X(), Angle(0.5));
    std::unique_ptr<Region> r(e.clone());
    REQUIRE(dynamic_cast<Ellipse *>(r.get()) != 0);
    CHECK(*dynamic_cast<Ellipse *>(r.get()) == e);
    CHECK(dynamic_cast<Ellipse *>(r.get()) != &e);
}

TEST_CASE(Point) {
    UnitVector3d f(1, 1, 1);
    Ellipse e(f, Angle(5.0e-12));
    CHECK(e.contains(f));
}

TEST_CASE(SmallCircle) {
    UnitVector3d f(1, -1, 1);
    Ellipse e(f, Angle(1.0));
    CHECK(e.getGamma() == Angle(0.0));
    CHECK_CLOSE(e.getAlpha().asRadians(), 1.0, 2);
    CHECK(e.getAlpha() == e.getBeta());
    CHECK(e.contains(f));
    CHECK(Ellipse(f, f, Angle(1)) == Ellipse(f, Angle(1), Angle(1), Angle(2)));
}

TEST_CASE(GreatCircle) {
    UnitVector3d f(1, -1, 1);
    Ellipse e(f, f, Angle(0.5 * PI));
    CHECK(e.isGreatCircle());
    CHECK(e.getAlpha() == e.getBeta());
}

TEST_CASE(EmptyEllipse) {
    Ellipse e;
    CHECK(e.isEmpty());
    CHECK(!e.isFull());
    CHECK(!e.isGreatCircle());
    CHECK(e == e);
    CHECK(!(e != e));
    CHECK(e == Ellipse::empty());
    CHECK(e.getAlpha() < Angle(0.0) && e.getBeta() < Angle(0.0));
    CHECK(e.complemented().isFull());
    // Technically, an empty ellipse should contain itself, be within itself,
    // and be disjoint from itself. However, the ellipse-ellipse relation
    // test is currently very inexact.
    CHECK(e.relate(e) == DISJOINT);
    // The bounding box and circle for an empty ellipse should be empty.
    CHECK(e.getBoundingBox().isEmpty());
    CHECK(e.getBoundingCircle().isEmpty());
    // Check constructor arguments that should produce empty ellipses.
    CHECK(Ellipse(UnitVector3d::X(), -Angle(PI)).isEmpty());
    CHECK(Ellipse(UnitVector3d::X(), -Angle(PI), Angle(1), Angle(1)).isEmpty());
    CHECK(Ellipse(UnitVector3d::X(), Angle(1), -Angle(PI), Angle(1)).isEmpty());
}

TEST_CASE(FullEllipse) {
    Ellipse e = Ellipse::full();
    CHECK(!e.isEmpty());
    CHECK(e.isFull());
    CHECK(!e.isGreatCircle());
    CHECK(e == e);
    CHECK(!(e != e));
    CHECK(e.getAlpha() >= Angle(PI) && e.getBeta() >= Angle(PI));
    CHECK(e.complemented().isEmpty());
    // Technically, an empty ellipse should contain itself, be within itself,
    // and intersect itself. However, the ellipse-ellipse relation
    // test is currently very inexact.
    CHECK(e.relate(e) == INTERSECTS);
    CHECK(e.relate(Circle(UnitVector3d::X())) == INTERSECTS);
    // Check constructor arguments that should produce full ellipses.
    CHECK(Ellipse(UnitVector3d::X(), Angle(PI)).isFull());
    CHECK(Ellipse(UnitVector3d::X(), UnitVector3d::Y(), Angle(PI)).isFull());
    CHECK(Ellipse(UnitVector3d::X(), Angle(PI), Angle(PI), Angle(0)).isFull());
}

TEST_CASE(Complement) {
    UnitVector3d f1(1, 2, 3);
    UnitVector3d f2(3, 2, 1);
    Ellipse e0(f1, f2, Angle(1));
    Ellipse e1 = e0.complemented();
    CHECK(e0 != e1);
    CHECK(e1.complemented() == e0);
    CHECK(e0.getCenter() == -e1.getCenter());
    CHECK(e0.getF1() == -e1.getF1());
    CHECK(e0.getF2() == -e1.getF2());
    CHECK_CLOSE(PI - e0.getAlpha().asRadians(), e1.getAlpha().asRadians(), 2);
    CHECK_CLOSE(PI - e0.getBeta().asRadians(), e1.getBeta().asRadians(), 2);
}

TEST_CASE(InvalidArguments) {
    UnitVector3d v = UnitVector3d::X();
    Angle inf(std::numeric_limits<double>::infinity());
    CHECK_THROW(Ellipse(v, Angle::nan()),
                std::invalid_argument);
    CHECK_THROW(Ellipse(v, Angle::nan(), Angle(1), Angle(1)),
                std::invalid_argument);
    CHECK_THROW(Ellipse(v, Angle(1), Angle::nan(), Angle(1)),
                std::invalid_argument);
    CHECK_THROW(Ellipse(v, Angle(1), Angle(0.5), Angle::nan()),
                std::invalid_argument);
    CHECK_THROW(Ellipse(v, Angle(1), Angle(0.5), inf),
                std::invalid_argument);
    CHECK_THROW(Ellipse(v, Angle(1), Angle(0.5), -inf),
                std::invalid_argument);
    Angle angles[3] = { Angle(0.25 * PI), Angle(0.5 * PI), Angle(0.75 * PI) };
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (i != j) {
                CHECK_THROW(Ellipse(v, angles[i], angles[j], Angle(0)),
                            std::invalid_argument);
            }
        }
    }
}

TEST_CASE(Contains1) {
    typedef std::vector<UnitVector3d>::const_iterator Iter;
    Angle epsilon = Angle::fromDegrees(1.0 / 3600000.0);
    UnitVector3d f1(1, 2, 3);
    UnitVector3d f2(3, 2, 1);
    Ellipse e0(f1, f2, Angle(1));
    Ellipse e1 = e0.complemented();
    Ellipse e2(f1, f2, Angle(1) + epsilon);
    Ellipse e3(f1, f2, Angle(1) - epsilon);
    std::vector<UnitVector3d> points = getPointsOnEllipse(e2, 100);
    for (Iter p = points.begin(), ep = points.end(); p != ep; ++p) {
        CHECK(!e0.contains(*p));
        CHECK(e1.contains(*p));
    }
    points = getPointsOnEllipse(e3, 100);
    for (Iter p = points.begin(), ep = points.end(); p != ep; ++p) {
        CHECK(e0.contains(*p));
        CHECK(!e1.contains(*p));
    }
}

TEST_CASE(Contains2) {
    Ellipse e(UnitVector3d::X(), Angle(0.1), Angle(PI/4), Angle(PI/8));
    CHECK(e.contains(UnitVector3d::X()));
    CHECK(e.contains(UnitVector3d(LonLat::fromDegrees(40, -16))));
    CHECK(e.contains(UnitVector3d(LonLat::fromDegrees(-40, 16))));
    CHECK(!e.contains(UnitVector3d(LonLat::fromDegrees(40, 16))));
    CHECK(!e.contains(UnitVector3d(LonLat::fromDegrees(-40, -16))));
    CHECK(!e.contains(UnitVector3d::Y()));
    CHECK(!e.contains(UnitVector3d::Z()));
    CHECK(!e.contains(-UnitVector3d::X()));
    CHECK(!e.contains(-UnitVector3d::Y()));
    CHECK(!e.contains(-UnitVector3d::Z()));
}
