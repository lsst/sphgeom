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
/// \brief This file contains tests for the Box class.

#include <limits>
#include <memory>

#include "lsst/sphgeom/Box3d.h"

#include "test.h"
#include "relationshipTestUtils.h"


using namespace lsst::sphgeom;

void checkProperties(Box3d const & b) {
    checkBasicProperties(b);
    // A non-empty box should contain, be within, and intersect itself.
    checkRelationship(b, b, CONTAINS | WITHIN);
    // A non-empty box should contain and intersect its center.
    if (!b.isFull()) {
        if (b == b.getCenter()) {
            checkRelationship(b, b.getCenter(), CONTAINS | WITHIN);
        } else {
            checkRelationship(b, b.getCenter(), CONTAINS);
        }
    }
    // Taking the union of any box with an empty box should be a no-op.
    CHECK(b.expandedTo(b) == b);
    CHECK(b.expandedTo(Box3d()) == b);
    CHECK(Box3d().expandedTo(b) == b);
    // Intersecting a box with itself should be a no-op.
    CHECK(b.clippedTo(b) == b);
    // Intersecting any box with an empty box should give the empty box.
    CHECK(b.clippedTo(Box3d()).isEmpty());
    CHECK(Box3d().clippedTo(b).isEmpty());
    // The union of any box with a full box should result in a full box.
    CHECK(b.expandedTo(Box3d::full()).isFull());
    CHECK(Box3d::full().expandedTo(b).isFull());
    // The intersection of any box with a full box should have no effect.
    CHECK(b.clippedTo(Box3d::full()) == b);
    CHECK(Box3d::full().clippedTo(b) == b);
    if (!b.isFull()) {
        // A non-full box should intersect and be within a full box.
        checkContains(Box3d::full(), b);
    }
}

TEST_CASE(Stream) {
    Box3d b = Box3d(Interval1d(0, 1), Interval1d(2, 3), Interval1d(4, 5));
    std::stringstream ss;
    ss << b;
    CHECK(ss.str() == "{\"Box3d\": [[0, 1], [2, 3], [4, 5]]}");
}

TEST_CASE(Construction) {
    Box3d b0 = Box3d::aroundUnitSphere();
    checkProperties(b0);
    CHECK(b0.getWidth() == 2 && b0.getHeight() == 2 && b0.getDepth() == 2);
    CHECK(b0.getCenter() == Vector3d(0, 0, 0));
    Box3d b1 = Box3d(Vector3d(1, 2, 3));
    checkProperties(b1);
    CHECK(b1.getWidth() == 0 && b1.getHeight() == 0 && b1.getDepth() == 0);
    CHECK(b1.getCenter() == Vector3d(1, 2, 3));
    Box3d b2 = Box3d(Vector3d(1, 2, 3), Vector3d(4, 6, 8));
    checkProperties(b2);
    CHECK(b2.getWidth() == 3 && b2.getHeight() == 4 && b2.getDepth() == 5);
    CHECK(b2.getCenter() == Vector3d(2.5, 4, 5.5));
    Box3d b3 = Box3d(Interval1d(1, 4), Interval1d(2, 6), Interval1d(3, 8));
    Box3d b4 = Box3d(b2.getCenter(), 1.5, 2, 2.5);
    CHECK(b0 != b1 && b1 != b2);
    CHECK(b2 == b3 && b3 == b4);
    CHECK(!b0.isEmpty() && !b1.isEmpty() && !b2.isEmpty());
    CHECK(!b0.isFull() && !b1.isFull() && !b2.isFull());
}

TEST_CASE(EmptyBox3d) {
    Box3d b;
    CHECK(b.isEmpty());
    CHECK(!b.isFull());
    CHECK(b == b);
    CHECK(b == Box3d::empty());
    CHECK(b.x().isEmpty());
    CHECK(b.y().isEmpty());
    CHECK(b.z().isEmpty());
    CHECK(b.getWidth() < 0 || std::isnan(b.getWidth()));
    CHECK(b.getHeight() < 0 || std::isnan(b.getHeight()));
    CHECK(b.getDepth() < 0 || std::isnan(b.getDepth()));
    // An empty box should contain itself, be within itself,
    // and be disjoint from itself.
    checkRelationship(b, b, CONTAINS | WITHIN | DISJOINT);
    // The union with the empty/full box should result in the
    // empty/full boxes.
    CHECK(b.expandedTo(b) == b);
    CHECK(b.expandedTo(Box3d::full()).isFull());
    // Intersecting with the empty/full box should have no effect.
    CHECK(b.clippedTo(b) == b);
    CHECK(b.clippedTo(Box3d::full()) == b);
    // Morphological operations should have no effect on empty boxes.
    CHECK(b.dilatedBy(1) == b);
    CHECK(b.dilatedBy(1, 2, 3) == b);
    CHECK(b.erodedBy(1, 2, 3) == b);
}

TEST_CASE(FullBox3d) {
    Box3d b = Box3d::full();
    checkProperties(b);
    CHECK(b.isFull());
    CHECK(b.getWidth() == std::numeric_limits<double>::infinity());
    CHECK(b.getHeight() == std::numeric_limits<double>::infinity());
    CHECK(b.getDepth() == std::numeric_limits<double>::infinity());
    // Morphological operations should have no effect on full boxes.
    CHECK(b.dilatedBy(1) == b);
    CHECK(b.dilatedBy(1, 2, 3) == b);
    CHECK(b.erodedBy(1, 2, 3) == b);
}


TEST_CASE(Box3dVector3dRelations1) {
    Box3d b = Box3d(Interval1d(0, 1), Interval1d(2, 3), Interval1d(4, 5));
    checkProperties(b);
    CHECK(b.contains(Vector3d(0, 2, 4)) && b.intersects(Vector3d(0, 2, 4)));
    CHECK(!b.isWithin(Vector3d(0, 2, 4)) && !b.isDisjointFrom(Vector3d(0, 2, 4)));
    CHECK(b.contains(Vector3d(1, 3, 5)) && b.intersects(Vector3d(1, 3, 5)));
    CHECK(!b.isWithin(Vector3d(1, 3, 5)) && !b.isDisjointFrom(Vector3d(1, 3, 5)));
    CHECK(b.contains(b.getCenter()) && b.intersects(b.getCenter()));
    CHECK(!b.isWithin(b.getCenter()) && !b.isDisjointFrom(b.getCenter()));
    CHECK(!b.contains(Vector3d(-1, 1, 3)) && !b.intersects(Vector3d(-1, 1, 3)));
    CHECK(!b.isWithin(Vector3d(-1, 1, 3)) && b.isDisjointFrom(Vector3d(-1, 1, 3)));
    CHECK(!b.contains(Vector3d(2, 4, 6)) && !b.intersects(Vector3d(2, 4, 6)));
    CHECK(!b.isWithin(Vector3d(2, 4, 6)) && b.isDisjointFrom(Vector3d(2, 4, 6)));
}

TEST_CASE(Box3dVector3dRelations2) {
    Box3d b = Box3d(Vector3d(1, 2, 3));
    checkProperties(b);
    CHECK(!b.isDisjointFrom(Vector3d(1, 2, 3)));
    CHECK(b.intersects(Vector3d(1, 2, 3)));
    CHECK(b.contains(Vector3d(1, 2, 3)));
    CHECK(b.isWithin(Vector3d(1, 2, 3)));
    CHECK(b.contains(Box3d()));
}

TEST_CASE(Box3dBox3dRelations) {
    CHECK(Box3d::aroundUnitSphere().relate(
            Box3d(Vector3d(2, 2, 2), Vector3d(3, 3, 3))) ==
          DISJOINT);
    CHECK(Box3d::aroundUnitSphere().relate(
            Box3d(Vector3d(-0.5, -0.5, -0.5), Vector3d(1, 1, 1))) ==
          CONTAINS);
    CHECK(Box3d(Vector3d(-0.5, -0.5, -0.5), Vector3d(1, 1, 1)).relate(
            Box3d::aroundUnitSphere()) ==
          WITHIN);
    CHECK(Box3d(Vector3d(0, 0, 0.5), Vector3d(1, 1, 1.5)).relate(
            Box3d::aroundUnitSphere()) ==
          INTERSECTS);
}

TEST_CASE(Vector3dExpansion) {
    Box3d b = Box3d::aroundUnitSphere();
    CHECK(b.expandedTo(Vector3d(0, 0, 0)) == b);
    CHECK(b.expandedTo(Vector3d(2, 0, 0)) ==
          Box3d(Interval1d(-1, 2), Interval1d(-1, 1), Interval1d(-1, 1)));
    CHECK(b.expandedTo(Vector3d(-2, 0, 0)) ==
          Box3d(Interval1d(-2, 1), Interval1d(-1, 1), Interval1d(-1, 1)));
    CHECK(b.expandedTo(Vector3d(0, 2, 0)) ==
          Box3d(Interval1d(-1, 1), Interval1d(-1, 2), Interval1d(-1, 1)));
    CHECK(b.expandedTo(Vector3d(0, -2, 0)) ==
          Box3d(Interval1d(-1, 1), Interval1d(-2, 1), Interval1d(-1, 1)));
    CHECK(b.expandedTo(Vector3d(0, 0, 2)) ==
          Box3d(Interval1d(-1, 1), Interval1d(-1, 1), Interval1d(-1, 2)));
    CHECK(b.expandedTo(Vector3d(0, 0, -2)) ==
          Box3d(Interval1d(-1, 1), Interval1d(-1, 1), Interval1d(-2, 1)));
}

TEST_CASE(Box3dExpansion) {
    CHECK(Box3d::aroundUnitSphere()
            .expandedTo(Box3d(Vector3d(1, 2, 3), 1, 2, 3)) ==
          Box3d(Vector3d(-1, -1, -1), Vector3d(2, 4, 6)));
}

TEST_CASE(Vector3dContraction) {
    Vector3d v(0.25, -0.25, 0.5);
    CHECK(Box3d::aroundUnitSphere().clippedTo(v) == v);
    CHECK(Box3d(Vector3d(2, 2, 2), 1, 1, 1).clippedTo(v).isEmpty());
    CHECK(Box3d::aroundUnitSphere().clippedTo(Vector3d(0, 2, 0)).isEmpty());
    CHECK(Box3d::aroundUnitSphere().clippedTo(Vector3d(0, 0, 2)).isEmpty());
}

TEST_CASE(Box3dContraction) {
    CHECK(Box3d(Vector3d(2, 2, 2), 1, 1, 1)
            .clippedTo(Box3d::aroundUnitSphere()) ==
          Box3d(Vector3d(1, 1, 1)));
    CHECK(Box3d::aroundUnitSphere().clippedTo(
            Box3d(Vector3d(0, 2, 0), Vector3d(1, 3, 1))).isEmpty());
    CHECK(Box3d::aroundUnitSphere().clippedTo(
            Box3d(Vector3d(0, 0, 2), Vector3d(1, 1, 3))).isEmpty());
}

TEST_CASE(Dilation) {
    CHECK(Box3d::aroundUnitSphere().dilatedBy(1) ==
          Box3d(Vector3d(-2, -2, -2), Vector3d(2, 2, 2)));
    CHECK(Box3d::aroundUnitSphere().dilatedBy(1, 2, 3) ==
          Box3d(Vector3d(-2, -3, -4), Vector3d(2, 3, 4)));
}

TEST_CASE(Erosion) {
    CHECK(Box3d(Vector3d(-2, -3, -4), Vector3d(2, 3, 4))
            .erodedBy(1, 2, 3) ==
          Box3d::aroundUnitSphere());
    CHECK(Box3d::aroundUnitSphere().erodedBy(1, 1, 1) ==
          Vector3d(0, 0, 0));
    CHECK(Box3d::aroundUnitSphere().erodedBy(2, 0, 0).isEmpty());
    CHECK(Box3d::aroundUnitSphere().erodedBy(0, 2, 0).isEmpty());
    CHECK(Box3d::aroundUnitSphere().erodedBy(0, 0, 2).isEmpty());
}
