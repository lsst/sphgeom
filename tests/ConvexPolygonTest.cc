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

#include "Box.h"
#include "Circle.h"
#include "ConvexPolygon.h"

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

TEST_CASE(Stream) {
    std::vector<UnitVector3d> points;
    points.push_back(UnitVector3d::X());
    points.push_back(UnitVector3d::Y());
    points.push_back(UnitVector3d::Z());
    ConvexPolygon p(points);
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
