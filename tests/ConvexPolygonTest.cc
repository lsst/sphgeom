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
/// \brief This file contains tests for the ConvexPolygon class.

#include <vector>

#include "lsst/sphgeom/Box.h"
#include "lsst/sphgeom/Box3d.h"
#include "lsst/sphgeom/Circle.h"
#include "lsst/sphgeom/ConvexPolygon.h"

#include "Test.h"


using namespace lsst::sphgeom;

typedef std::vector<UnitVector3d>::const_iterator VertexIterator;

void checkProperties(ConvexPolygon const & p) {
    CHECK(p.getVertices().size() >= 3);
    CHECK(p == p);
    CHECK(!(p != p));
    // A polygon should contain its vertices.
    for (VertexIterator v = p.getVertices().begin(), end = p.getVertices().end();
         v != end; ++v) {
        CHECK(p.contains(*v));
    }
    // A polygon should contain its centroid.
    CHECK(p.contains(p.getCentroid()));
    // The bounding circle and box for a polygon should
    // CONTAIN and INTERSECT the polygon.
    CHECK(p.getBoundingCircle().relate(p) == (CONTAINS | INTERSECTS));
    CHECK(p.getBoundingBox().relate(p) == (CONTAINS | INTERSECTS));
}

ConvexPolygon makeSimpleTriangle() {
    std::vector<UnitVector3d> points;
    points.push_back(UnitVector3d::X());
    points.push_back(UnitVector3d::Y());
    points.push_back(UnitVector3d::Z());
    return ConvexPolygon(points);
}

ConvexPolygon makeNgon(UnitVector3d const & center,
                       UnitVector3d const & v0,
                       size_t n)
{
    REQUIRE(center.dot(v0) > 1.5 * EPSILON);
    REQUIRE(n >= 3);
    std::vector<UnitVector3d> points;
    points.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        double f = static_cast<double>(i) / static_cast<double>(n);
        points.push_back(v0.rotatedAround(center, Angle(2.0 * PI) * f));
    }
    return ConvexPolygon(points);
}

TEST_CASE(Stream) {
    ConvexPolygon p = makeSimpleTriangle();
    std::stringstream ss;
    ss << p;
    CHECK(ss.str() == "ConvexPolygon(\n"
                      "    UnitVector3d(0, 0, 1),\n"
                      "    UnitVector3d(1, 0, 0),\n"
                      "    UnitVector3d(0, 1, 0)\n"
                      ")");
}

TEST_CASE(Construction) {
    std::vector<UnitVector3d> points;
    points.push_back(UnitVector3d(1, 2, 1));
    points.push_back(UnitVector3d(1, 2, 1));
    points.push_back(UnitVector3d(2, 1, 1));
    points.push_back(UnitVector3d(2, 1, 1));
    points.push_back(UnitVector3d(1, 1, 2));
    points.push_back(UnitVector3d(1, 1, 2));
    points.push_back(UnitVector3d(1, 1, 1));
    points.push_back(UnitVector3d::Y());
    points.push_back(UnitVector3d(1, 1, 0));
    points.push_back(UnitVector3d(0, 1, 1));
    points.push_back(UnitVector3d(1, 0, 1));
    points.push_back(UnitVector3d::X());
    points.push_back(UnitVector3d::Z());
    ConvexPolygon p(points);
    CHECK(p.getVertices().size() == 3);
    if (p.getVertices().size() >= 3) {
        CHECK(p.getVertices()[0] == UnitVector3d::Z());
        CHECK(p.getVertices()[1] == UnitVector3d::X());
        CHECK(p.getVertices()[2] == UnitVector3d::Y());
    }
    checkProperties(p);
    for (VertexIterator v = points.begin(), end = points.end(); v != end; ++v) {
        CHECK(p.contains(*v));
    }
    CHECK(p.contains(UnitVector3d(1, 1, 1)));
    std::rotate(points.begin(), points.begin() + 3, points.end());
    CHECK(p == ConvexPolygon(points));
}

TEST_CASE(ConstructionFailure) {
    std::vector<UnitVector3d> points;
    points.push_back(UnitVector3d::Y());
    CHECK_THROW(ConvexPolygon::convexHull(points), std::invalid_argument);
    points.push_back(UnitVector3d::X());
    CHECK_THROW(ConvexPolygon::convexHull(points), std::invalid_argument);
    points.push_back(UnitVector3d::Z());
    points.push_back(UnitVector3d(-1, -1, -1));
    CHECK_THROW(ConvexPolygon::convexHull(points), std::invalid_argument);
}

TEST_CASE(Centroid) {
    ConvexPolygon p = makeSimpleTriangle();
    UnitVector3d c = p.getCentroid();
    CHECK(c.dot(UnitVector3d(1, 1, 1)) >= 1.0 - EPSILON);
}

TEST_CASE(CircleRelations) {
    ConvexPolygon p = makeSimpleTriangle();
    CHECK(p.relate(p.getBoundingCircle()) == (INTERSECTS | WITHIN));
    CHECK(p.getBoundingCircle().relate(p) == (INTERSECTS | CONTAINS));
    CHECK(p.relate(Circle::full()) == (INTERSECTS | WITHIN));
    CHECK(p.relate(Circle::empty()) == (CONTAINS | DISJOINT));
    CHECK(p.relate(Circle(UnitVector3d(1, 1, 1), 0.25)) == (INTERSECTS | CONTAINS));
    CHECK(p.relate(Circle(UnitVector3d::X(), 1)) == INTERSECTS);
    CHECK(p.relate(Circle(UnitVector3d::Y(), 1)) == INTERSECTS);
    CHECK(p.relate(Circle(UnitVector3d::Z(), 1)) == INTERSECTS);
    CHECK(p.relate(Circle(-UnitVector3d::X(), 1)) == DISJOINT);
    CHECK(p.relate(Circle(-UnitVector3d::Y(), 1)) == DISJOINT);
    CHECK(p.relate(Circle(-UnitVector3d::Z(), 1)) == DISJOINT);
}

TEST_CASE(PolygonRelations1) {
    ConvexPolygon t = makeSimpleTriangle();
    std::vector<UnitVector3d> points;
    points.push_back(UnitVector3d::X());
    points.push_back(UnitVector3d::Y());
    points.emplace_back(1, 1, 1);
    ConvexPolygon p = ConvexPolygon::convexHull(points);
    CHECK(p.relate(p) == (CONTAINS | INTERSECTS | WITHIN));
    CHECK(t.relate(p) == (CONTAINS | INTERSECTS));
    CHECK(p.relate(t) == (INTERSECTS | WITHIN));
}

TEST_CASE(PolygonRelations2) {
    // These are all degenerate cases where the intersection of
    // two polygons is an edge or an edge segment.
    ConvexPolygon t = makeSimpleTriangle();
    std::vector<UnitVector3d> points;
    points.emplace_back(1, 2, 0);
    points.emplace_back(2, 1, 0);
    points.push_back(-UnitVector3d::Z());
    ConvexPolygon p = ConvexPolygon::convexHull(points);
    CHECK(p.relate(t) == INTERSECTS);
    CHECK(t.relate(p) == INTERSECTS);
    points.clear();
    points.emplace_back(2, -1, 0);
    points.emplace_back(-1, 2, 0);
    points.push_back(-UnitVector3d::Z());
    p = ConvexPolygon::convexHull(points);
    CHECK(p.relate(t) == INTERSECTS);
    CHECK(t.relate(p) == INTERSECTS);
    points.clear();
    points.emplace_back(1, 1, 0);
    points.emplace_back(-1, 2, 0);
    points.push_back(-UnitVector3d::Z());
    p = ConvexPolygon::convexHull(points);
    CHECK(p.relate(t) == INTERSECTS);
    CHECK(t.relate(p) == INTERSECTS);
}

TEST_CASE(PolygonRelations3) {
    ConvexPolygon p1 = makeSimpleTriangle();
    ConvexPolygon p2 = makeNgon(UnitVector3d::X(), UnitVector3d(1, 1, 1), 3);
    std::vector<UnitVector3d> points;
    points.emplace_back(2, -1, 1);
    points.emplace_back(-1, 2, 1);
    points.emplace_back(2, 2, -1);
    ConvexPolygon p3 = ConvexPolygon::convexHull(points);
    CHECK(p1.relate(p2) == INTERSECTS);
    CHECK(p1.relate(p3) == INTERSECTS);
    CHECK(p2.relate(p3) == INTERSECTS);
}

TEST_CASE(BoundingBox) {
    ConvexPolygon p = makeNgon(UnitVector3d::Z(), UnitVector3d(1, 1, 1), 4);
    Box b = p.getBoundingBox();
    Angle a = Angle::fromRadians(0.61547970867038734);
    CHECK(b.getLon().isFull());
    CHECK(b.getLat().getA() >= a - Angle(MAX_ASIN_ERROR));
    CHECK(b.getLat().getA() <= a);
    CHECK(b.getLat().getB() == Angle(0.5 * PI));
    p = makeNgon(-UnitVector3d::Z(), UnitVector3d(-1, -1, -1), 4);
    b = p.getBoundingBox();
    CHECK(b.getLon().isFull());
    CHECK(b.getLat().getA() == -Angle(0.5 * PI));
    CHECK(b.getLat().getB() >= -a);
    CHECK(b.getLat().getB() <= -a + Angle(MAX_ASIN_ERROR));
    p = makeNgon(UnitVector3d::Y(), UnitVector3d(1, 1, 1), 4);
    b = p.getBoundingBox();
    CHECK(b.getLon().getA() >= Angle(0.25 * PI - MAX_ASIN_ERROR));
    CHECK(b.getLon().getA() <= Angle(0.25 * PI));
    CHECK(b.getLon().getB() >= Angle(0.75 * PI));
    CHECK(b.getLon().getB() <= Angle(0.75 * PI + MAX_ASIN_ERROR));
    CHECK(b.getLat().getA() <= Angle(-0.25 * PI));
    CHECK(b.getLat().getA() >= Angle(-0.25 * PI - MAX_ASIN_ERROR));
    CHECK(b.getLat().getB() >= Angle(0.25 * PI));
    CHECK(b.getLat().getB() <= Angle(0.25 * PI + MAX_ASIN_ERROR));
}

TEST_CASE(BoundingBox3d) {
    ConvexPolygon p = makeSimpleTriangle();
    Box3d b = p.getBoundingBox3d();
    CHECK(b.x().getA() >= -1.0e-14 && b.x().getA() <= 0);
    CHECK(b.y().getA() >= -1.0e-14 && b.y().getA() <= 0);
    CHECK(b.z().getA() >= -1.0e-14 && b.z().getA() <= 0);
    CHECK(b.x().getB() == 1);
    CHECK(b.y().getB() == 1);
    CHECK(b.z().getB() == 1);
}

TEST_CASE(BoundingCircle) {
    ConvexPolygon p = makeSimpleTriangle();
    Circle c = p.getBoundingCircle();
    CHECK(c.contains(UnitVector3d::X()));
    CHECK(c.contains(UnitVector3d::Y()));
    CHECK(c.contains(UnitVector3d::Z()));
    CHECK(c.getCenter().dot(UnitVector3d(1, 1, 1)) >= 1.0 - EPSILON);
    double scl = 2.0 * (std::sqrt(3.0) - 1.0) / std::sqrt(3.0);
    CHECK(c.getSquaredChordLength() >= scl);
    CHECK(c.getSquaredChordLength() <= scl + 3.0 * MAX_SCL_ERROR);
}
