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

#include "lsst/sphgeom/Box.h"
#include "lsst/sphgeom/Circle.h"

#include "Test.h"
#include "RelationTestUtils.h"


using namespace lsst::sphgeom;

void checkProperties(Box const & b) {
    checkBasicProperties(b);
    // A non-empty box should contain, be within, and intersect itself.
    checkRelations(b, b, CONTAINS | INTERSECTS | WITHIN);
    // A non-empty box should contain and intersect its center.
    checkRelations(b, b.getCenter(), CONTAINS | INTERSECTS);
    // Taking the union of any box with an empty box should be a no-op.
    CHECK(b.expandedTo(b) == b);
    CHECK(b.expandedTo(Box()) == b);
    CHECK(Box().expandedTo(b) == b);
    // Intersecting a box with itself should be a no-op.
    CHECK(b.clippedTo(b) == b);
    // Intersecting any box with an empty box should give the empty box.
    CHECK(b.clippedTo(Box()).isEmpty());
    CHECK(Box().clippedTo(b).isEmpty());
    // The union of any box with a full box should result in a full box.
    CHECK(b.expandedTo(Box::full()).isFull());
    CHECK(Box::full().expandedTo(b).isFull());
    // The intersection of any box with a full box should have no effect.
    CHECK(b.clippedTo(Box::full()) == b);
    CHECK(Box::full().clippedTo(b) == b);
    if (!b.isFull()) {
        // A non-full box should intersect and be within a full box.
        checkContains(Box::full(), b);
    }
}

TEST_CASE(Stream) {
    Box b(LonLat::fromRadians(1, 0), Angle(1), Angle(1));
    std::stringstream ss;
    ss << b;
    CHECK(ss.str() == "Box([0 rad, 2 rad], [-1 rad, 1 rad])");
}

TEST_CASE(HalfWidthForCircle) {
    CHECK(Box::halfWidthForCircle(Angle(-1), Angle(0)) == Angle(0));
    CHECK(Box::halfWidthForCircle(Angle(1), Angle(1)) == Angle(PI));
    CHECK_CLOSE(
        Box::halfWidthForCircle(Angle(1), Angle(0)).asRadians(), 1.0, 4);
    CHECK_CLOSE(
        Box::halfWidthForCircle(Angle(PI/6), Angle(PI/4)).asRadians(), PI/4, 4);
}

TEST_CASE(Construction) {
    LonLat p = LonLat::fromRadians(0.1, 0.1);
    Box b1(p, Angle(1), Angle(1));
    CHECK_CLOSE(b1.getWidth().asRadians(), 2, 3);
    CHECK_CLOSE(b1.getHeight().asRadians(), 2, 2);
    Box b2(LonLat::fromRadians(0, 0), Angle(1), Angle(10));
    CHECK(b2.getLat() == Box::allLatitudes());
    CHECK(Box(LonLat::fromRadians(0, 0), Angle(10), Angle(10)).isFull());
    CHECK(Box(LonLat::fromRadians(0, 0), Angle(-1), Angle(-1)).isEmpty());
    CHECK(Box(LonLat::fromRadians(0, 0), Angle(1), Angle(-1)).isEmpty());
    CHECK(Box(LonLat::fromRadians(0, 0), Angle(-1), Angle(1)).isEmpty());
    CHECK(Box(p) == Box(p,p));
    CHECK(!Box(p).isEmpty());
    CHECK(Box(p).getArea() == 0.0);
    Box b3(NormalizedAngleInterval::fromDegrees(270, 90),
           AngleInterval(Angle(-90), Angle(90)));
    CHECK(b3 == Box(LonLat::fromDegrees(270, -90),
                    LonLat::fromDegrees(90, 90)));
}

TEST_CASE(Clone) {
    Box b(LonLat::fromDegrees(45, 45),
          Angle::fromDegrees(15),
          Angle::fromDegrees(15));
    std::auto_ptr<Region> r(b.clone());
    REQUIRE(dynamic_cast<Box *>(r.get()) != 0);
    CHECK(*dynamic_cast<Box *>(r.get()) == b);
    CHECK(dynamic_cast<Box *>(r.get()) != &b);
}

TEST_CASE(EmptyBox) {
    Box b;
    CHECK(b.isEmpty());
    CHECK(!b.isFull());
    CHECK(b == b);
    CHECK(b == Box::empty());
    CHECK(b.getLon().isEmpty());
    CHECK(b.getLat().isEmpty());
    CHECK(b.getArea() == 0);
    CHECK(b.getWidth().isNan());
    CHECK(b.getHeight().asRadians() < 0 || b.getHeight().isNan());
    // An empty box should contain itself, be within itself,
    // and be disjoint from itself.
    checkRelations(b, b, CONTAINS | WITHIN | DISJOINT);
    // The union with the empty/full box should result in the
    // empty/full boxes.
    CHECK(b.expandedTo(b) == b);
    CHECK(b.expandedTo(Box::full()).isFull());
    // Intersecting with the empty/full box should have no effect.
    CHECK(b.clippedTo(b) == b);
    CHECK(b.clippedTo(Box::full()) == b);
    // Morphological operations should have no effect on empty boxes.
    CHECK(b.dilatedBy(Angle(PI)) == b);
    CHECK(b.dilatedBy(Angle(PI), Angle(PI)) == b);
    CHECK(b.erodedBy(Angle(PI), Angle(PI)) == b);
    // The bounding box and circle for an empty box should be empty.
    CHECK(b.getBoundingBox().isEmpty());
    CHECK(b.getBoundingCircle().isEmpty());
}

TEST_CASE(FullBox) {
    Box b = Box::full();
    CHECK(b.isFull());
    CHECK(b.getWidth() == 2*Angle(PI));
    CHECK(b.getHeight() == Angle(PI));
    CHECK(b.getArea() == 4*PI);
    checkProperties(b);
    // Morphological operations should have no effect on full boxes.
    CHECK(b.dilatedBy(Angle(PI)) == b);
    CHECK(b.erodedBy(Angle(PI), Angle(PI)) == b);
    // The bounding box and circle for a full box should be full.
    CHECK(b.getBoundingBox().isFull());
    CHECK(b.getBoundingCircle().isFull());
}

TEST_CASE(Center) {
    LonLat p = LonLat::fromRadians(2, 1);
    CHECK(Box(p).getCenter() == p);
    Box b1(LonLat::fromRadians(1, 0.5), LonLat::fromRadians(3, 1.5));
    CHECK(b1.getCenter() == p);
    CHECK(b1.contains(UnitVector3d(p)));
    Box b2(LonLat::fromRadians(2*PI - 1, -1), LonLat::fromRadians(2, 1));
    CHECK(b2.getCenter().getLat() == Angle(0));
    CHECK_CLOSE(b2.getCenter().getLon().asRadians(), 0.5, 1);
}

TEST_CASE(BoxCircleRelations1) {
    CHECK(Box::empty().relate(Circle::empty()) ==
          (CONTAINS | DISJOINT | WITHIN));
    CHECK(Box::empty().relate(Circle::full()) ==
          (DISJOINT | WITHIN));
    CHECK(Box::full().relate(Circle::empty()) ==
          (CONTAINS | DISJOINT));
    CHECK(Box::full().relate(Circle::full()) ==
          (CONTAINS | INTERSECTS | WITHIN));
    CHECK(Box::full().relate(Circle(UnitVector3d::X(), Angle(1))) ==
          (CONTAINS | INTERSECTS));
    CHECK(Box::fromDegrees(0, 0, 0, 0).relate(Circle::full()) ==
          (INTERSECTS | WITHIN));
}

TEST_CASE(BoxCircleRelations2) {
    UnitVector3d x = UnitVector3d::X();
    CHECK(Box::fromDegrees(-10, -10, 10, 10).relate(Circle(-x, Angle(1))) ==
          DISJOINT);
    CHECK(Box::fromRadians(-3, -1, 3, 1).relate(Circle(-x, Angle(0.1))) ==
          DISJOINT);
}

TEST_CASE(BoxCircleRelations3) {
    UnitVector3d x = UnitVector3d::X();
    CHECK(Box::fromRadians(-1, -1, 1, 1)
            .relate(Circle(-x, Angle(PI - 0.5))) ==
          INTERSECTS);
    CHECK(Box::fromRadians(-1, -1, 1, 1)
            .relate(Circle(UnitVector3d(Angle(1), Angle(0)), Angle(0.5))) ==
          INTERSECTS);
    CHECK(Box::fromRadians(-1, -1, 1, 1)
            .relate(Circle(UnitVector3d(Angle(-1.5), Angle(0)), Angle(1))) ==
          INTERSECTS);
    CHECK(Box::fromRadians(-1, -1, 1, 1)
            .relate(Circle(UnitVector3d(Angle(-1), Angle(1)), Angle(0.1))) ==
          INTERSECTS);
    CHECK(Box::fromRadians(-0.5, -0.5, 0, 0)
            .relate(Circle(UnitVector3d(1, 1, 1), 0.8452994616207485)) ==
          INTERSECTS);
    CHECK(Box::fromRadians(-1, -0.5, 1, 0.5)
            .relate(Circle(-x, Angle(PI - 0.6))) ==
          INTERSECTS);
    CHECK(Box::fromRadians(-0.5, -0.5, 0.5, 0.5)
            .relate(Circle(-x, Angle(PI - 0.6))) ==
          INTERSECTS);
    CHECK(Box::fromRadians(-1, -0.5, 1, 0.5)
            .relate(Circle(x, Angle(0.6))) ==
          INTERSECTS);
}

TEST_CASE(BoxCircleRelations4) {
    UnitVector3d x = UnitVector3d::X();
    CHECK(Box::fromRadians(-1, -1, 1, 1).relate(Circle(x, Angle(0.5))) ==
          (CONTAINS | INTERSECTS));
    CHECK(Box::fromRadians(-0.5, -0.5, 0.5, 0.5).relate(Circle(x, Angle(1))) ==
          (INTERSECTS | WITHIN));
}

TEST_CASE(BoxPointRelations1) {
    Box b = Box::fromDegrees(170, -10, 190, 10);
    CHECK(b.contains(LonLat::fromDegrees(180, 0)));
    CHECK(b.isDisjointFrom(LonLat::fromDegrees(200, 0)));
    CHECK(b.isDisjointFrom(LonLat::fromDegrees(0, 0)));
    CHECK(b.isDisjointFrom(LonLat::fromDegrees(180, 45)));
    CHECK(b.isDisjointFrom(LonLat::fromDegrees(180, -45)));
    CHECK(b.contains(LonLat(NormalizedAngle::nan(), Angle(1))));
}

TEST_CASE(BoxPointRelations2) {
    Box b = Box::fromDegrees(270, 40, 90, 50);
    CHECK(b.isDisjointFrom(LonLat::fromDegrees(180, 45)));
    CHECK(b.intersects(LonLat::fromDegrees(315, 45)));
    CHECK(b.intersects(LonLat::fromDegrees(45, 45)));
    CHECK(b.isDisjointFrom(LonLat::fromDegrees(315, 0)));
    CHECK(b.isDisjointFrom(LonLat::fromDegrees(45, 80)));
    CHECK(b.contains(LonLat(NormalizedAngle(1), Angle::nan())));
}

TEST_CASE(PointExpansion1) {
    Box b = Box::fromDegrees(90, -30, 180, 30);
    CHECK(b.expandedTo(LonLat::fromDegrees(100, -45)) ==
          Box::fromDegrees(90, -45, 180, 30));
    CHECK(b.expandedTo(LonLat::fromDegrees(100, 45)) ==
          Box::fromDegrees(90, -30, 180, 45));
    CHECK(b.expandedTo(LonLat::fromDegrees(45, 0)) ==
          Box::fromDegrees(45, -30, 180, 30));
    CHECK(b.expandedTo(LonLat::fromDegrees(200, 0)) ==
          Box::fromDegrees(90, -30, 200, 30));
    CHECK(b.expandedTo(LonLat::fromDegrees(315, 0)) ==
          Box::fromDegrees(315, -30, 180, 30));
}

TEST_CASE(PointExpansion2) {
    Box b = Box::fromDegrees(270, -30, 45, 30);
    CHECK(b.expandedTo(LonLat::fromDegrees(0, -45)) ==
          Box::fromDegrees(270, -45, 45, 30));
    CHECK(b.expandedTo(LonLat::fromDegrees(0, 45)) ==
          Box::fromDegrees(270, -30, 45, 45));
    CHECK(b.expandedTo(LonLat::fromDegrees(90, 0)) ==
          Box::fromDegrees(270, -30, 90, 30));
    CHECK(b.expandedTo(LonLat::fromDegrees(180, 0)) ==
          Box::fromDegrees(180, -30, 45, 30));
}

TEST_CASE(BoxExpansion) {
    CHECK(Box::fromRadians(6, -1, 1, 1)
            .expandedTo(Box::fromRadians(1, 0, 6, 0.5)) ==
          Box::fromRadians(0, -1, 2 * PI, 1));
}

TEST_CASE(PointContraction) {
    LonLat p = LonLat::fromDegrees(45, 45);
    CHECK(Box::fromDegrees(0, 0, 90, 90).clippedTo(p) == p);
    CHECK(Box::fromDegrees(0, 0, 10, 10).clippedTo(p).isEmpty());
    CHECK(Box::fromDegrees(0, 40, 10, 50).clippedTo(p).isEmpty());

}

TEST_CASE(BoxContraction) {
    CHECK(Box::fromDegrees(20, 0, 340, 10)
            .clippedTo(Box::fromDegrees(-20, 0, 20, 10)) ==
          Box::fromDegrees(-20, 0, 20, 10));
    CHECK(Box::fromDegrees(20, 0, 30, 10)
            .clippedTo(Box::fromDegrees(25, 20, 35, 30)).isEmpty());
}

TEST_CASE(Dilation1) {
    Angle radius = Angle::fromDegrees(10);
    Angle epsilon = Angle::fromDegrees(2.78e-6); // about 10 milliarcsec
    Box b = Box::fromDegrees(170, -10, 190, 10).dilatedBy(radius + epsilon);
    UnitVector3d v = UnitVector3d(LonLat::fromDegrees(170, -10));
    CHECK(b.relate(Circle(v, radius)) == (CONTAINS | INTERSECTS));
    v = UnitVector3d(LonLat::fromDegrees(170, 10));
    CHECK(b.relate(Circle(v, radius)) == (CONTAINS | INTERSECTS));
    v = UnitVector3d(LonLat::fromDegrees(190, -10));
    CHECK(b.relate(Circle(v, radius)) == (CONTAINS | INTERSECTS));
    v = UnitVector3d(LonLat::fromDegrees(190, 10));
    CHECK(b.contains(Circle(v, radius).getBoundingBox()));
}

TEST_CASE(Dilation2) {
    CHECK(Box::fromRadians(1, 0.5, 2, 1).dilatedBy(Angle(1), Angle(0.5)) ==
          Box::fromRadians(0, 0, 3, 1.5));
}

TEST_CASE(Erosion) {
    CHECK(Box::fromRadians(1, 0.5, 2, 1).erodedBy(Angle(0.5), Angle(0.25)) ==
          Box(LonLat::fromRadians(1.5, 0.75)));
    CHECK(Box::fromRadians(1, 0.5, 2, 1)
            .erodedBy(Angle(0.6), Angle(0))
            .isEmpty());
    CHECK(Box::fromRadians(1, 0.5, 2, 0.5)
            .erodedBy(Angle(0), Angle(0.5))
            .isEmpty());
}

TEST_CASE(BoxBounds1) {
    // Check that a box is its own bounding box.
    Box b = Box::fromRadians(-1, -1, 1, 1);
    CHECK(b == b.getBoundingBox());
    CHECK(b.getBoundingBox().contains(b));
    // Check that a box is contained by its bounding circle, starting
    // with boxes spanning at most 180 degrees in longitude.
    CHECK(b.relate(static_cast<Region const &>(b.getBoundingCircle())) ==
          (INTERSECTS | WITHIN));
    CHECK(b.getBoundingCircle().relate(static_cast<Region const &>(b)) ==
          (CONTAINS | INTERSECTS));
}

TEST_CASE(BoxBounds2) {
    Box b = Box::fromDegrees(100, 84, 278, 85);
    Circle c = b.getBoundingCircle();
    CHECK(b.relate(c) == (INTERSECTS | WITHIN));
}

TEST_CASE(BoxBounds3) {
    Box b = Box::fromDegrees(100, -86, 270, -85);
    Circle c = b.getBoundingCircle();
    CHECK(b.relate(c) == (INTERSECTS | WITHIN));
}

// Check bounding circle properties for boxes that span more
// than 180 degrees in longitude.

TEST_CASE(BoxBounds4) {
    Box b = Box::fromDegrees(45, 45, 315, 60);
    Circle c = b.getBoundingCircle();
    CHECK(c.getCenter() == UnitVector3d::Z());
    CHECK(c.getOpeningAngle().asDegrees() < 45.001);
    CHECK(c.relate(b) == (CONTAINS | INTERSECTS));
}

TEST_CASE(BoxBounds5) {
    Box b = Box::fromDegrees(45, -60, 315, -45);
    Circle c = b.getBoundingCircle();
    CHECK(c.getCenter() == -UnitVector3d::Z());
    CHECK(c.getOpeningAngle().asDegrees() < 45.001);
    CHECK(c.relate(b) == (CONTAINS | INTERSECTS));
}

TEST_CASE(BoxBounds6) {
    Box b = Box::fromDegrees(45, -60, 315, 60);
    Circle c = b.getBoundingCircle();
    CHECK(c.getCenter().z() == 0);
    CHECK(c.getOpeningAngle().asDegrees() < 135.001);
    CHECK(c.relate(b) == (CONTAINS | INTERSECTS));
}
