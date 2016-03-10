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
/// \brief This file contains tests for the Box class.

#include <memory>
#include <vector>

#include "lsst/sphgeom/Box.h"
#include "lsst/sphgeom/Box3d.h"
#include "lsst/sphgeom/Circle.h"

#include "Test.h"
#include "RelationshipTestUtils.h"


using namespace lsst::sphgeom;

std::vector<UnitVector3d> getPointsOnCircle(Circle const & c,
                                            size_t numPoints)
{
    std::vector<UnitVector3d> points;
    if (c.isEmpty() || c.isFull()) {
        return points;
    }
    UnitVector3d n = UnitVector3d::orthogonalTo(c.getCenter());
    // Construct a point on the circle boundary.
    UnitVector3d v0 = c.getCenter().rotatedAround(n, c.getOpeningAngle());
    // Rotate v0 around the circle center to generate points.
    for (size_t i = 0; i < numPoints; ++i) {
        double f = static_cast<double>(i) / numPoints;
        points.push_back(v0.rotatedAround(c.getCenter(), Angle(2.0 * PI) * f));
    }
    return points;
}

void checkProperties(Circle const & c) {
    checkBasicProperties(c);
    // A non-empty circle should contain and intersect its center.
    checkRelationship(c, c.getCenter(), CONTAINS);
    // Taking the union of any circle with an empty circle should be a no-op.
    CHECK(c.expandedTo(Circle()) == c);
    CHECK(Circle().expandedTo(c) == c);
    // Intersecting a circle with itself should be a no-op.
    CHECK(c.clippedTo(c) == c);
    // Intersecting a circle with an empty circle should give the empty circle.
    CHECK(c.clippedTo(Circle()).isEmpty());
    CHECK(Circle().clippedTo(c).isEmpty());
    // The union of any circle with a full circle should be a full circle.
    CHECK(c.expandedTo(Circle::full()).isFull());
    CHECK(Circle::full().expandedTo(c).isFull());
    // The intersection of any circle with a full circle should have no effect.
    CHECK(c.clippedTo(Circle::full()) == c);
    CHECK(Circle::full().clippedTo(c) == c);
    if (!c.isFull()) {
        // A non-full circle should intersect and be within a full circle.
        checkContains(Circle::full(), c);
    }
}

TEST_CASE(Stream) {
    Circle c(UnitVector3d::X(), 1);
    std::stringstream ss;
    ss << c;
    CHECK(ss.str() == "{\"Circle\": [[1, 0, 0], 1]}");
}

TEST_CASE(Construction) {
    UnitVector3d v(1.0, 1.0, 1.0);
    Circle c1 = Circle(v);
    CHECK(c1.getSquaredChordLength() == 0.0);
    CHECK(c1.getOpeningAngle() == Angle(0));
    CHECK(c1.getArea() == 0.0);
    CHECK(!c1.isEmpty());
    CHECK(!c1.isFull());
    checkProperties(c1);
    Circle c2 = Circle(v, Angle(PI/2));
    CHECK_CLOSE(c2.getSquaredChordLength(), 2.0, 4);
    CHECK_CLOSE(c2.getArea(), 2.0 * PI, 4);
    CHECK(!c2.isEmpty());
    CHECK(!c2.isFull());
    checkProperties(c2);
    Circle c3 = Circle(v, 2.0);
    CHECK_CLOSE(c3.getOpeningAngle().asRadians(), PI/2, 2);
    CHECK_CLOSE(c3.getArea(), 2.0 * PI, 1);
    checkProperties(c3);
}

TEST_CASE(Clone) {
    Circle c(UnitVector3d::X(), 0.5);
    std::unique_ptr<Region> r(c.clone());
    REQUIRE(dynamic_cast<Circle *>(r.get()) != 0);
    CHECK(*dynamic_cast<Circle *>(r.get()) == c);
    CHECK(dynamic_cast<Circle *>(r.get()) != &c);
}

TEST_CASE(EmptyCircle) {
    Circle c;
    CHECK(c.isEmpty());
    CHECK(!c.isFull());
    CHECK(c == c);
    CHECK(!(c != c));
    CHECK(c == Circle::empty());
    CHECK(c.getArea() == 0);
    CHECK(c.getSquaredChordLength() < 0.0 ||
          std::isnan(c.getSquaredChordLength()));
    CHECK(c.getOpeningAngle() < Angle(0) || c.getOpeningAngle().isNan());
    // An empty circle should contain itself, be within itself,
    // and be disjoint from itself.
    checkRelationship(c, c, CONTAINS | WITHIN | DISJOINT);
    // The union with the empty/full circle should result in the
    // empty/full circles.
    CHECK(c.expandedTo(c) == c);
    CHECK(c.expandedTo(Circle::full()).isFull());
    // Intersecting with the empty/full circle should have no effect.
    CHECK(c.clippedTo(c) == c);
    CHECK(c.clippedTo(Circle::full()) == c);
    // Morphological operations should have no effect on empty circles.
    CHECK(c.dilatedBy(Angle(PI)) == c);
    CHECK(c.erodedBy(Angle(PI)) == c);
    // The bounding box and circle for an empty circle should be empty.
    CHECK(c.getBoundingBox().isEmpty());
    CHECK(c.getBoundingCircle().isEmpty());
    // Circles with NaN as the opening angle or squared chord lengths
    // should be empty.
    UnitVector3d x = UnitVector3d::X();
    CHECK(Circle(x, Angle::nan()).isEmpty());
    CHECK(Circle(x, Angle(-1)).isEmpty());
    CHECK(Circle(x, -1.0).isEmpty());
    CHECK(Circle(x, std::numeric_limits<double>::quiet_NaN()).isEmpty());
}

TEST_CASE(FullCircle) {
    Circle c = Circle::full();
    CHECK(c == c);
    CHECK(!(c != c));
    CHECK(c.isFull());
    CHECK(c.getOpeningAngle() >= Angle(PI));
    CHECK(c.getSquaredChordLength() >= 4.0);
    CHECK(c.getArea() == 4*PI);
    checkProperties(c);
    checkRelationship(c, Circle::full(), CONTAINS | WITHIN);
    checkRelationship(c, Circle(UnitVector3d::X()), CONTAINS);
    checkRelationship(Circle(UnitVector3d::X()), c, WITHIN);
    // Morphological operations should have no effect on full circles.
    CHECK(c.dilatedBy(Angle(PI)) == c);
    CHECK(c.erodedBy(Angle(PI)) == c);
    // The bounding box and circle for a full circle should be full.
    CHECK(c.getBoundingBox().isFull());
    CHECK(c.getBoundingCircle().isFull());
    // Check constructor arguments that should produce full circles.
    CHECK(Circle(UnitVector3d::X(), Angle(PI)).isFull());
    CHECK(Circle(UnitVector3d::X(), Angle(2*PI)).isFull());
    CHECK(Circle(UnitVector3d::X(), 4.0).isFull());
    CHECK(Circle(UnitVector3d::X(), 5.0).isFull());
}

TEST_CASE(Center) {
    Circle c(UnitVector3d::Z());
    CHECK(c.getCenter() == Vector3d(0, 0, 1));
}

TEST_CASE(Contains1) {
    UnitVector3d x = UnitVector3d::X();
    UnitVector3d y = UnitVector3d::Y();
    UnitVector3d z = UnitVector3d::Z();
    CHECK(Circle(x, 2).contains(Circle(x, 1)));
    CHECK(Circle(z, 3.5).contains(Circle(x, 0.5)));
    CHECK(!Circle(y, 1).contains(Circle(x, 1)));
    CHECK(Circle(x, 1).contains(Circle()));
    CHECK(Circle(x, 0).contains(x));
    CHECK(Circle::full().contains(x));
    CHECK(Circle::full().contains(Circle(z, 1)));
}

TEST_CASE(Contains2) {
    typedef std::vector<UnitVector3d>::const_iterator Iter;
    Angle epsilon = Angle::fromDegrees(1.0 / 3600000.0);
    Circle c(UnitVector3d(1, -2, -3), Angle(0.1));
    Circle c1 = Circle(c.getCenter(), c.getOpeningAngle() + epsilon);
    Circle c2 = Circle(c.getCenter(), c.getOpeningAngle() - epsilon);
    std::vector<UnitVector3d> points = getPointsOnCircle(c1, 100);
    for (Iter p = points.begin(), ep = points.end(); p != ep; ++p) {
        CHECK(!c.contains(*p));
    }
    points = getPointsOnCircle(c2, 100);
    for (Iter p = points.begin(), ep = points.end(); p != ep; ++p) {
        CHECK(c.contains(*p));
    }
}

TEST_CASE(IsDisjointFrom) {
    UnitVector3d x = UnitVector3d::X();
    UnitVector3d y = UnitVector3d::Y();
    UnitVector3d z = UnitVector3d::Z();
    CHECK(Circle(x, 2).isDisjointFrom(-x));
    CHECK(Circle().isDisjointFrom(x));
    CHECK(Circle(y, 3).isDisjointFrom(Circle(-y, 0.5)));
    CHECK(Circle(x, 0.5).isDisjointFrom(Circle(z, 0.5)));
}

TEST_CASE(Intersects) {
    UnitVector3d x = UnitVector3d::X();
    UnitVector3d y = UnitVector3d::Y();
    UnitVector3d z = UnitVector3d::Z();
    CHECK(Circle(x).intersects(x));
    CHECK(!Circle().intersects(y));
    CHECK(Circle(x, 2).intersects(Circle(z, 2)));
    CHECK(Circle::full().intersects(y));
    CHECK(Circle::full().intersects(Circle(z, 1)));
}

TEST_CASE(IsWithin) {
    UnitVector3d x = UnitVector3d::X();
    UnitVector3d y = UnitVector3d::Y();
    UnitVector3d z = UnitVector3d::Z();
    CHECK(Circle().isWithin(x));
    CHECK(Circle().isWithin(Circle(y)));
    CHECK(Circle(x, 0.5).isWithin(Circle(z, 3.5)));
    CHECK(Circle(x, 0.5).isWithin(Circle::full()));
}

TEST_CASE(PointContraction) {
    UnitVector3d x = UnitVector3d::X();
    UnitVector3d z = UnitVector3d::Z();
    CHECK(Circle(x, 1).clippedTo(z).isEmpty());
    CHECK(Circle(x, 1).clippedTo(x) == Circle(x));
}

TEST_CASE(CircleContraction) {
    UnitVector3d x = UnitVector3d::X();
    UnitVector3d y = UnitVector3d::Y();
    Circle c1 = Circle(x, 1.5);
    Circle c2 = Circle(y, 1);
    Circle c = c1.clippedTo(c2);
    CHECK(!c.isEmpty());
    CHECK(c.getOpeningAngle() <= c1.getOpeningAngle() &&
          c.getOpeningAngle() <= c2.getOpeningAngle());
    CHECK(c.getSquaredChordLength() <= c1.getSquaredChordLength() &&
          c.getSquaredChordLength() <= c2.getSquaredChordLength());
    CHECK(Circle(x, 1).clippedTo(Circle(-x, 1)).isEmpty());
}

TEST_CASE(PointExpansion) {
    Circle c1 = Circle(UnitVector3d::X(), Angle(PI/4));
    UnitVector3d v = UnitVector3d(LonLat::fromRadians(3*PI/4, 0.0));
    Vector3d expectedCenter(std::sqrt(2)/2, std::sqrt(2)/2, 0.0);
    CHECK(c1.expandedTo(c1.getCenter()) == c1);
    Circle c = c1.expandedTo(v);
    CHECK(abs(c.getOpeningAngle() - Angle(PI/2)) < Angle(4 * MAX_ASIN_ERROR));
    CHECK(NormalizedAngle(c.getCenter(), expectedCenter) < Angle(MAX_ASIN_ERROR));
    CHECK(Circle().expandedTo(v) == Circle(v));
}

TEST_CASE(CircleExpansion) {
    UnitVector3d x = UnitVector3d::X();
    UnitVector3d y = UnitVector3d::Y();
    Circle c1 = Circle(x, Angle(PI/4));
    Circle c2 = Circle(y, Angle(PI/4));
    Vector3d expectedCenter(std::sqrt(2)/2, std::sqrt(2)/2, 0.0);
    Circle c = c1.expandedTo(c2);
    CHECK(abs(c.getOpeningAngle() - Angle(PI/2)) < Angle(4 * MAX_ASIN_ERROR));
    CHECK(NormalizedAngle(c.getCenter(), expectedCenter) < Angle(MAX_ASIN_ERROR));
    CHECK(Circle(x, 2).expandedTo(Circle(x, 1)) == Circle(x, 2));
    CHECK(Circle(x, 1).expandedTo(Circle(x, 2)) == Circle(x, 2));
    CHECK(Circle(x, 3).expandedTo(Circle(-x, 3)).isFull());
}

TEST_CASE(Dilation) {
    UnitVector3d x = UnitVector3d::X();
    CHECK(Circle(x).dilatedBy(Angle(0.0)) == Circle(x));
    CHECK(Circle(x).dilatedBy(Angle::nan()) == Circle(x));
    CHECK(Circle(x, Angle(PI/2)).dilatedBy(Angle(PI)).isFull());
    CHECK(Circle(x, Angle(PI/4)).dilatedBy(Angle(PI/4)).getOpeningAngle() ==
          Angle(PI/2));
}

TEST_CASE(Erosion) {
    UnitVector3d x = UnitVector3d::X();
    CHECK(Circle(x).erodedBy(Angle(0.0)) == Circle(x));
    CHECK(Circle(x).erodedBy(Angle::nan()) == Circle(x));
    CHECK(Circle(x, Angle(PI/2)).erodedBy(Angle(PI)).isEmpty());
    CHECK(Circle(x, Angle(PI/2)).erodedBy(Angle(PI/4)).getOpeningAngle() ==
          Angle(PI/4));
}

TEST_CASE(PointRelations) {
    UnitVector3d x = UnitVector3d::X();
    UnitVector3d y = UnitVector3d::Y();
    CHECK(Circle().relate(x) == (DISJOINT | WITHIN));
    CHECK(Circle(x).relate(y) == DISJOINT);
    CHECK(Circle(x).relate(x) == CONTAINS);
}

TEST_CASE(CircleRelations) {
    UnitVector3d x = UnitVector3d::X();
    UnitVector3d y = UnitVector3d::Y();
    CHECK(Circle(x, 1).relate(Circle(-x, 1)) == DISJOINT);
    CHECK(Circle(x, 2).relate(Circle(x, 1)) == CONTAINS);
    CHECK(Circle(x, 1).relate(Circle(x, 2)) == WITHIN);
    CHECK(Circle(x, 1).relate(Circle(y, 1)) == INTERSECTS);
}

TEST_CASE(Box3dBounds1) {
    static double const TOLERANCE = 1.0e-15;

    UnitVector3d x = UnitVector3d::X();
    UnitVector3d y = UnitVector3d::Y();
    UnitVector3d z = UnitVector3d::Z();
    Circle c = Circle(x, 2.0);
    Box3d b = c.getBoundingBox3d();
    CHECK(b.x().getB() == 1);
    CHECK(b.x().getA() >= -TOLERANCE && b.x().getA() <= 0.0);
    CHECK(b.y() == Interval1d(-1, 1));
    CHECK(b.z() == Interval1d(-1, 1));
    c = Circle(y, 2.0);
    b = c.getBoundingBox3d();
    CHECK(b.x() == Interval1d(-1, 1));
    CHECK(b.y().getB() == 1);
    CHECK(b.y().getA() >= -TOLERANCE && b.y().getA() <= 0.0);
    CHECK(b.z() == Interval1d(-1, 1));
    c = Circle(z, 2.0);
    b = c.getBoundingBox3d();
    CHECK(b.x() == Interval1d(-1, 1));
    CHECK(b.y() == Interval1d(-1, 1));
    CHECK(b.z().getB() == 1);
    CHECK(b.z().getA() >= -TOLERANCE && b.z().getA() <= 0.0);
}

TEST_CASE(Box3dBounds2) {
    static double const TOLERANCE = 1.0e-15;
    UnitVector3d a = UnitVector3d(1, 1, 0);
    Circle c = Circle(a, Angle(PI * 0.25));
    Box3d b = c.getBoundingBox3d();
    CHECK(b.x().getA() >= -TOLERANCE && b.x().getA() <= 0.0);
    CHECK(b.x().getB() == 1.0);
    CHECK(b.y().getA() >= -TOLERANCE && b.y().getA() <= 0.0);
    CHECK(b.y().getB() == 1.0);
    CHECK(b.z().getA() == -b.z().getB());
    CHECK(b.z().getB() >= 0.5 * std::sqrt(2.0) - TOLERANCE);
    CHECK(b.z().getB() <= 0.5 * std::sqrt(2.0) + TOLERANCE);
    a = UnitVector3d(1, 1, 1);
    c = Circle(a, 0.0);
    CHECK(Box3d(a).dilatedBy(TOLERANCE).contains(c.getBoundingBox3d()));
}
